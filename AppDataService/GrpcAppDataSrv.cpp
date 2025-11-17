#include <chrono>
#include <QStringList>

#include <CommonLib/Times.h>

#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/SocketIO.h"
#include "../UtilsLib/WUtils.h"

#include "GrpcAppDataSrv.h"

GrpcAppDataSrv::GrpcAppDataSrv(const SoftwareInfo& serverSwInfo,
								bool allowAllClients,
								const std::vector<ClientInfo>& clients,
								bool checkHostName,
								const std::vector<HostAddressPort>& listenIPs,
								AppDataReceiver* appDataReceiver,
								const AppSignals& appSignals,
								const DynamicAppSignalStates& signalStates,
								std::shared_ptr<DiscretesLogWriter> dsLogWriter,
								CircularLoggerShared log) :
	m_sessionGuard(serverSwInfo, allowAllClients, clients, checkHostName),
	m_appDataReceiver(appDataReceiver),
	m_appSignals(appSignals),
	m_signalStates(signalStates),
	m_dsLogWriter(dsLogWriter),
	m_log(log)
{
	TEST_PTR_RETURN(m_appDataReceiver);
	initService(listenIPs);
}

GrpcAppDataSrv::GrpcAppDataSrv(const SoftwareInfo& serverSwInfo,
							   bool allowAllClients,
							   const std::vector<ClientInfo>& clients,
							   bool checkHostName,
							   const HostAddressPort& listenIP,
							   AppDataReceiver* appDataReceiver,
							   const AppSignals& appSignals,
							   const DynamicAppSignalStates& signalStates,
							   std::shared_ptr<DiscretesLogWriter> dsLogWriter,
							   CircularLoggerShared log) :
	m_sessionGuard(serverSwInfo, allowAllClients, clients, checkHostName),
	m_appDataReceiver(appDataReceiver),
	m_appSignals(appSignals),
	m_signalStates(signalStates),
	m_dsLogWriter(dsLogWriter),
	m_log(log)
{
	TEST_PTR_RETURN(m_appDataReceiver);
	initService(std::vector<HostAddressPort>{listenIP});
}

GrpcAppDataSrv::~GrpcAppDataSrv()
{
	m_sessionGuard.stop();

	if (m_server != nullptr)
	{
		m_thread.request_stop();

		if (m_thread.joinable())
		{
			m_thread.join();
		}

		m_server.reset();

		DEBUG_LOG_MSG(m_log, "GrpcAppDataSrv finished.");
	}

	return;
}

void GrpcAppDataSrv::setSessionTimeout(int seconds)
{
	m_sessionGuard.setSessionTimeout(seconds);
}

grpc::Status GrpcAppDataSrv::Handshake(grpc::ServerContext* context,
										const Grpc::HandshakeRequest* request,
										Grpc::HandshakeReply* reply)
{
	Q_UNUSED(context);

	return m_sessionGuard.handshake(request, reply);
}

grpc::Status GrpcAppDataSrv::GetAppSignalList(grpc::ServerContext* context,
										const Grpc::GetAppSignalListRequest* request,
										grpc::ServerWriter<Grpc::GetAppSignalListReply>* writer)
{
	if (context == nullptr ||
		request == nullptr ||
		writer == nullptr)
	{
		Q_ASSERT(false);
		return grpc::Status::CANCELLED;
	}

	if (m_sessionGuard.extractAndValidateAuthToken(context) == false)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, Grpc::INVALID_OR_EXPIRED_SESSION);
	}

	Grpc::GetAppSignalListReply reply;
	reply.set_totalsize(static_cast<uint32_t>(m_appSignals.count()));

	constexpr int IDS_MAX_COUNT = 50000;

	reply.mutable_appsignalids()->Reserve(IDS_MAX_COUNT);

	grpc::Status writeStatus;

	auto writeReply = [this, context, writer](Grpc::GetAppSignalListReply& reply, grpc::Status& wrStatus) -> bool
	{
		wrStatus = grpc::Status::OK;

		if (context->IsCancelled())
		{
			wrStatus = grpc::Status::CANCELLED;
			DEBUG_LOG_MSG(m_log, "GetAppSignalList: context CANCELLED");
			return false;
		}

		// DEBUG_LOG_MSG(m_log, QString("GetAppSignalList: Write reply count = %1").arg(reply.appsignalids_size()));

		if (writer->Write(reply) == false)
		{
			DEBUG_LOG_MSG(m_log, "GetAppSignalList: writer->Write returns FALSE");
			return false;
		}

		return true;
	};

	int ctr = 0;

	for(const AppSignal& appSignal : m_appSignals)
	{
		*reply.add_appsignalids() = appSignal.appSignalID().toStdString();

		ctr++;

		if ((ctr & 0x3FFF) == 0 && context->IsCancelled())
		{
			DEBUG_LOG_MSG(m_log, "GetAppSignalList: context CANCELLED");
			return grpc::Status::CANCELLED;
		}

		if (ctr >= IDS_MAX_COUNT)
		{
			if (writeReply(reply, writeStatus) == false)
			{
				return writeStatus;
			}

			reply.Clear();
			ctr = 0;
		}
	}

	if (ctr > 0)
	{
		if (writeReply(reply, writeStatus) == false)
		{
			return writeStatus;
		}
	}

	return grpc::Status::OK;
}

grpc::Status GrpcAppDataSrv::GetAppSignalParam(grpc::ServerContext* context,
										 const Grpc::GetAppSignalParamRequest* request,
										 grpc::ServerWriter<Grpc::GetAppSignalParamReply>* writer)
{
	if (context == nullptr ||
		request == nullptr ||
		writer == nullptr)
	{
		Q_ASSERT(false);
		return grpc::Status::CANCELLED;
	}

	if (m_sessionGuard.extractAndValidateAuthToken(context) == false)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, Grpc::INVALID_OR_EXPIRED_SESSION);
	}

	Grpc::GetAppSignalParamReply reply;

	constexpr int PARAMS_MAX_COUNT = 256;

	reply.mutable_signalparams()->Reserve(PARAMS_MAX_COUNT);

	grpc::Status writeStatus;

	auto writeReply = [this, context, writer](Grpc::GetAppSignalParamReply& reply,
										size_t totalCount, int index,
										grpc::Status& wrStatus) -> bool
	{
		wrStatus = grpc::Status::OK;

		reply.set_totalsize(static_cast<::uint32_t>(totalCount));
		reply.set_replysignalindex(static_cast<::uint32_t>(index));

		if (context->IsCancelled())
		{
			wrStatus = grpc::Status::CANCELLED;
			DEBUG_LOG_MSG(m_log, "GetAppSignalParam: context CANCELLED");
			return false;
		}

		// DEBUG_LOG_MSG(m_log, QString("GetAppSignalParam: Write reply count = %1").arg(reply.signalparams_size()));

		if (writer->Write(reply) == false)
		{
			DEBUG_LOG_MSG(m_log, "GetAppSignalParam: writer->Write returns FALSE");
			return false;
		}

		return true;
	};

	int ctr = 0;
	int index = 0;

	if (request->signalhashes_size() == 0)
	{
		DEBUG_LOG_MSG(m_log, "GetAppSignalParam: signal hashes count 0");

		for(const AppSignal& appSignal : m_appSignals)
		{
			appSignal.saveToProto(reply.add_signalparams());

			ctr++;

			if (ctr >= PARAMS_MAX_COUNT)
			{
				if (writeReply(reply, m_appSignals.count(), index, writeStatus) == false)
				{
					return writeStatus;
				}

				index += ctr;

				reply.Clear();
				ctr = 0;
			}
		}

		if (ctr > 0)
		{
			if (writeReply(reply, m_appSignals.count(), index, writeStatus) == false)
			{
				return writeStatus;
			}
		}

		return grpc::Status::OK;
	}
	else
	{
		DEBUG_LOG_MSG(m_log, QString("GetAppSignalParam: signal hashes count %1").arg(request->signalhashes_size()));

		for(auto h : request->signalhashes())
		{
			Hash hash = static_cast<Hash>(h);

			const AppSignal* appSignal = m_appSignals.getByHash(hash);

			appSignal->saveToProto(reply.add_signalparams());

			ctr++;

			if (ctr >= PARAMS_MAX_COUNT)
			{
				if (writeReply(reply, request->signalhashes_size(), index, writeStatus) == false)
				{
					return writeStatus;
				}

				index += ctr;

				reply.Clear();
				ctr = 0;
			}
		}

		if (ctr > 0)
		{
			if (writeReply(reply, request->signalhashes_size(), index, writeStatus) == false)
			{
				return writeStatus;
			}
		}
	}

	return grpc::Status::OK;
}

grpc::Status GrpcAppDataSrv::GetAppSignalState(grpc::ServerContext* context,
											const Grpc::GetAppSignalStateRequest* request,
											Grpc::GetAppSignalStateReply* reply)
{
	if (context == nullptr ||
		request == nullptr ||
		reply == nullptr)
	{
		Q_ASSERT(false);
		return grpc::Status::CANCELLED;
	}

	if (m_sessionGuard.extractAndValidateAuthToken(context) == false)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, Grpc::INVALID_OR_EXPIRED_SESSION);
	}

	if (request->signalhashes_size() > ADS_GET_APP_SIGNAL_STATE_MAX)
	{
		return grpc::Status(grpc::StatusCode::OUT_OF_RANGE,
							Grpc::SIGNAL_HASHES_COUNT_EXEEDS_ADS_GET_APP_SIGNAL_STATE_MAX);
	}

	//

	reply->mutable_appsignalstates()->Reserve(request->signalhashes_size());

	AppSignalState appSignalState;

	for(Hash hash : request->signalhashes())
	{
		bool result = m_signalStates.getCurrentState(hash, appSignalState);

		if (result == false)
		{
			continue;	// unknown hash
		}

		appSignalState.save(reply->add_appsignalstates());
	}

	return grpc::Status::OK;
}

grpc::Status GrpcAppDataSrv::GetAppSignalStateChanges(grpc::ServerContext* context,
													  const Grpc::GetAppSignalStateChangesRequest* request,
													  grpc::ServerWriter<Grpc::GetAppSignalStateChangesReply>* writer)
{
	if (context == nullptr ||
		request == nullptr ||
		writer == nullptr ||
		m_appDataReceiver == nullptr)
	{
		Q_ASSERT(false);
		return grpc::Status::CANCELLED;
	}

	if (m_sessionGuard.extractAndValidateAuthToken(context) == false)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, Grpc::INVALID_OR_EXPIRED_SESSION);
	}

	SimpleAppSignalStatesQueueShared statesQueue =
		std::make_shared<SimpleAppSignalStatesQueue>(static_cast<int>(m_appSignals.count()) * 3);

	m_appDataReceiver->registerDestSignalStatesQueue(statesQueue, false,
					QString("GrpcAppDataSrv[%1]::statesQueue").
						arg(QString::fromStdString(m_sessionGuard.extractAuthTokenFromMetadata(context))));

	auto writeReply = [this, context, writer](Grpc::GetAppSignalStateChangesReply& reply,
											  grpc::Status& wrStatus) -> bool
	{
		wrStatus = grpc::Status::OK;

		if (context->IsCancelled())
		{
			wrStatus = grpc::Status::CANCELLED;
			DEBUG_LOG_MSG(m_log, "GetAppSignalStateChanges: context CANCELLED");
			return false;
		}

		// DEBUG_LOG_MSG(m_log, QString("GetAppSignalStateChanges: Write reply states count = %1").arg(reply.appsignalstates_size()));

		if (writer->Write(reply) == false)
		{
			DEBUG_LOG_MSG(m_log, "GetAppSignalStateChanges: writer->Write returns FALSE");
			return false;
		}

		return true;
	};

	Grpc::GetAppSignalStateChangesReply reply;

	reply.mutable_appsignalstates()->Reserve(ADS_GET_APP_SIGNAL_STATE_MAX);

	grpc::Status writeStatus;
	SimpleAppSignalState state;

	int waitCtr = 0;

	while(context->IsCancelled() == false)
	{
		int statesCount = 0;

		while(statesQueue->isEmpty() == false && statesCount < ADS_GET_APP_SIGNAL_STATE_MAX)
		{
			bool res = statesQueue->pop(&state);

			if (res == false)
			{
				continue;
			}

			Proto::AppSignalState* protoState = reply.add_appsignalstates();

			state.save(protoState);
			statesCount++;
		}

		if (reply.appsignalstates_size() > 0)
		{
			if (writeReply(reply, writeStatus) == false)
			{
				m_appDataReceiver->unregisterDestSignalStatesQueue(statesQueue);
				return writeStatus;
			}

			waitCtr = 0;

			reply.Clear();
		}
		else
		{
			if (waitCtr > 2000)
			{
				waitCtr = 0;

				if (writeReply(reply, writeStatus) == false)
				{
					m_appDataReceiver->unregisterDestSignalStatesQueue(statesQueue);
					return writeStatus;
				}
			}
		}

		if (statesQueue->isEmpty() == true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			waitCtr++;
		}
	}

	m_appDataReceiver->unregisterDestSignalStatesQueue(statesQueue);

	return grpc::Status::CANCELLED;
}

grpc::Status GrpcAppDataSrv::GetDiscretesLog(grpc::ServerContext* context,
											const Grpc::GetDiscretesLogRequest* request,
											grpc::ServerWriter<Grpc::GetDiscretesLogReply>* writer)
{
	if (context == nullptr ||
		request == nullptr ||
		writer == nullptr ||
		m_dsLogWriter == nullptr)
	{
		Q_ASSERT(false);
		return grpc::Status::CANCELLED;
	}

	std::shared_ptr<DiscretesLogReader> dlReader = std::make_shared<DiscretesLogReader>(m_log);

	m_dsLogWriter->registerLogReader(dlReader);

	//

	m_dsLogWriter->unregisterLogReader(dlReader);

	return grpc::Status::CANCELLED;
}


void GrpcAppDataSrv::initService(const std::vector<HostAddressPort>& listenIPs)
{
	QStringList ips;

	grpc::ServerBuilder builder;

	builder.RegisterService(this);

	for(const HostAddressPort& ip : listenIPs)
	{
		QString ipStr = ip.addressPortStr();
		ips.append(ipStr);
		int selectedPort = 0;
		builder.AddListeningPort(ipStr.toStdString(), grpc::InsecureServerCredentials(), &selectedPort);
	}

	m_server = builder.BuildAndStart();

	if (m_server == nullptr)
	{
		DEBUG_LOG_ERR(m_log, "GrpcAppDataSrv NOT started!");
		return;
	}

	DEBUG_LOG_MSG(m_log, QString("GrpcAppDataSrv started. Listening addresses: %1").arg(ips.join(", ")));

	m_thread = std::jthread{[this](std::stop_token stoken, grpc::Server* server)
							{
								std::stop_callback stop_cb{stoken,
														   [server]()
														   {
															   server->Shutdown(std::chrono::system_clock::now() + std::chrono::seconds(5));
														   }};
								try
								{
									DEBUG_LOG_MSG(m_log, QString("GrpcAppDataSrv in Wait state"));
									server->Wait();		// unblocked by stop_token callback
								}
								catch (std::exception& e)
								{
									DEBUG_LOG_ERR(m_log, "GrpcAppDataSrv thread exception: " + QString{e.what()});
								}
								catch (...)
								{
									DEBUG_LOG_ERR(m_log, "GrpcAppDataSrv thread unknown exception");
								}
							},
							m_server.get()};

	m_sessionGuard.start();
}



