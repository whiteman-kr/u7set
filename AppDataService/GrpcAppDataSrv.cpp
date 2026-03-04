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
								const HostAddressPort& listenIP,
								const AppDataSources& appDataSources,
								AppDataReceiver* appDataReceiver,
								const AppSignals& appSignals,
								const DynamicAppSignalStates& signalStates,
								std::shared_ptr<DiscretesLogWriter> dsLogWriter,
								CircularLoggerShared log) :
	GrpcServer(serverSwInfo, allowAllClients, clients, checkHostName, listenIP, log),
	m_appDataSources(appDataSources),
	m_appDataReceiver(appDataReceiver),
	m_appSignals(appSignals),
	m_signalStates(signalStates),
	m_dsLogWriter(dsLogWriter)
{
	TEST_PTR_RETURN(m_appDataReceiver);
	start();
}

GrpcAppDataSrv::~GrpcAppDataSrv()
{
	stop();
}

grpc::Status GrpcAppDataSrv::Handshake(grpc::ServerContext* context,
										const Grpc::HandshakeRequest* request,
										Grpc::HandshakeReply* reply)
{
	return GrpcServer::handshake(context, request, reply);
}

grpc::Status GrpcAppDataSrv::Ping(grpc::ServerContext* context,
								const Grpc::PingRequest* request,
								Grpc::PingReply* reply)
{
	return GrpcServer::ping(context, request, reply);
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

	WriteReplyPtr<Grpc::GetAppSignalListReply> writeReply = &WriteReplyFunc<Grpc::GetAppSignalListReply>;

	int ctr = 0;

	for(const AppSignal& appSignal : m_appSignals)
	{
		*reply.add_appsignalids() = appSignal.appSignalID().toStdString();

		ctr++;

		if ((ctr & 0x3FFF) == 0 && context->IsCancelled())
		{
			logMsg("GetAppSignalList: context CANCELLED");
			return grpc::Status::CANCELLED;
		}

		if (ctr >= IDS_MAX_COUNT)
		{
			if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
			{
				return writeStatus;
			}

			reply.Clear();
			ctr = 0;
		}
	}

	if (ctr > 0)
	{
		if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
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

	WriteReplyPtr<Grpc::GetAppSignalParamReply> writeReply = &WriteReplyFunc<Grpc::GetAppSignalParamReply>;

	int ctr = 0;
	int index = 0;

	if (request->signalhashes_size() == 0)
	{
		logMsg("GetAppSignalParam: signal hashes count 0");

		for(const AppSignal& appSignal : m_appSignals)
		{
			appSignal.saveToProto(reply.add_signalparams());

			ctr++;

			if (ctr >= PARAMS_MAX_COUNT)
			{
				reply.set_totalsize(static_cast<quint32>(m_appSignals.count()));
				reply.set_replysignalindex(index);

				if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
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
			reply.set_totalsize(static_cast<quint32>(m_appSignals.count()));
			reply.set_replysignalindex(index);

			if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
			{
				return writeStatus;
			}
		}

		return grpc::Status::OK;
	}
	else
	{
		logMsg(QString("GetAppSignalParam: signal hashes count %1").arg(request->signalhashes_size()));

		for(auto h : request->signalhashes())
		{
			Hash hash = static_cast<Hash>(h);

			const AppSignal* appSignal = m_appSignals.getByHash(hash);

			appSignal->saveToProto(reply.add_signalparams());

			ctr++;

			if (ctr >= PARAMS_MAX_COUNT)
			{
				reply.set_totalsize(request->signalhashes_size());
				reply.set_replysignalindex(index);

				if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
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
			reply.set_totalsize(request->signalhashes_size());
			reply.set_replysignalindex(index);

			if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
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

	//

	WriteReplyPtr<Grpc::GetAppSignalStateChangesReply> writeReply = &WriteReplyFunc<Grpc::GetAppSignalStateChangesReply>;

	//

	SimpleAppSignalStatesQueueShared statesQueue =
		std::make_shared<SimpleAppSignalStatesQueue>(static_cast<int>(m_appSignals.count()) * 5);

	m_appDataReceiver->registerDestSignalStatesQueue(statesQueue, false,
													 QString("GrpcAppDataSrv[%1]::statesQueue").
													 arg(QString::fromStdString(m_sessionGuard.extractAuthTokenFromMetadata(context))));
	//

	Grpc::GetAppSignalStateChangesReply reply;

	reply.mutable_appsignalstates()->Reserve(ADS_GET_APP_SIGNAL_STATE_MAX);

	grpc::Status writeStatus;
	SimpleAppSignalState state;

	int waitCtr = 0;

	auto lastSendTime = std::chrono::steady_clock::now();
	constexpr int SEND_PACKET_SIZE = 2048;
	constexpr std::chrono::milliseconds SEND_PACKET_INTERVAL{50};

	while(context->IsCancelled() == false)
	{
		int statesCount = reply.appsignalstates_size();

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

		auto now = std::chrono::steady_clock::now();

		if (waitCtr > 2000 ||
			statesCount > SEND_PACKET_SIZE ||
			(statesCount > 0 && now - lastSendTime >= SEND_PACKET_INTERVAL))
		{
			if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
			{
				m_appDataReceiver->unregisterDestSignalStatesQueue(statesQueue);
				return writeStatus;
			}

			waitCtr = 0;
			lastSendTime = now;

			reply.mutable_appsignalstates()->Clear();
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

	if (m_sessionGuard.extractAndValidateAuthToken(context) == false)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, Grpc::INVALID_OR_EXPIRED_SESSION);
	}

	//

	WriteReplyPtr<Grpc::GetDiscretesLogReply> writeReply = &WriteReplyFunc<Grpc::GetDiscretesLogReply>;

	//

	std::shared_ptr<DiscretesLogReader> dlReader = std::make_shared<DiscretesLogReader>(getLog());

	m_dsLogWriter->registerLogReader(dlReader);

	//

	Grpc::GetDiscretesLogReply reply;

	reply.mutable_discreteslogrecord()->Reserve(ADS_GET_DISCRETES_LOG_MAX_RECORD_COUNT);

	grpc::Status writeStatus;
	auto lastSendTime = std::chrono::steady_clock::now();
	constexpr int SEND_PACKET_SIZE = 100;
	constexpr std::chrono::milliseconds SEND_PACKET_INTERVAL{200};

	while(context->IsCancelled() == false)
	{
		dlReader->getDiscretesLog(&reply);

		auto now = std::chrono::steady_clock::now();
		int addedRecordCount = reply.discreteslogrecord_size();

		if (addedRecordCount > SEND_PACKET_SIZE ||
			now - lastSendTime >= SEND_PACKET_INTERVAL)
		{
			if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
			{
				m_dsLogWriter->unregisterLogReader(dlReader);
				return writeStatus;
			}

			lastSendTime = now;

			reply.Clear();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	m_dsLogWriter->unregisterLogReader(dlReader);

	return grpc::Status::CANCELLED;
}

grpc::Status GrpcAppDataSrv::AckDiscretesLog(grpc::ServerContext* context,
											const Grpc::AckDiscretesLogRequest* request,
											Grpc::AckDiscretesLogReply* reply)
{
	if (context == nullptr ||
		request == nullptr ||
		reply == nullptr ||
		m_dsLogWriter == nullptr)
	{
		Q_ASSERT(false);
		return grpc::Status::CANCELLED;
	}

	if (m_sessionGuard.extractAndValidateAuthToken(context) == false)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, Grpc::INVALID_OR_EXPIRED_SESSION);
	}

	m_dsLogWriter->ackDiscretesLog(*request);

	reply->set_ackuser(request->ackuser());
	reply->set_acksource(request->acksource());
	reply->set_ackuptoplanttime(request->ackuptoplanttime());

	return grpc::Status::OK;
}

// grpc::Status GrpcAppDataSrv::GetRtTrendsData(grpc::ServerContext* context,
// 		grpc::ServerReaderWriter<Grpc::GetRtTrendsDataReply, Grpc::GetRtTrendsDataRequest>* stream)
// {
// 	Grpc::GetRtTrendsDataRequest request;
// 	Grpc::GetRtTrendsDataReply reply;

// 	while (stream->Read(&request))
// 	{
// 		// process request

// 		reply.Clear();

// 		// set reply

// 		bool ok = stream->Write(reply);
// 		if (!ok)
// 		{
// 			break;
// 		}
// 	}

// 	return grpc::Status::OK;
// }

grpc::Status GrpcAppDataSrv::GetAppDataSourcesInfo(grpc::ServerContext* context,
												   const Grpc::GetAppDataSourcesInfoRequest* request,
												   Grpc::GetAppDataSourcesInfoReply* reply)
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

	//

	for(AppDataSource* source : m_appDataSources)
	{
		TEST_PTR_CONTINUE(source);

		Network::DataSourceInfo* protoInfo = reply->add_appdatasourceinfo();
		source->saveToProto(protoInfo);
	}

	return grpc::Status::OK;
}

grpc::Status GrpcAppDataSrv::GetAppDataSourcesState(grpc::ServerContext* context,
													const Grpc::GetAppDataSourcesStateRequest* request,
													Grpc::GetAppDataSourcesStateReply* reply)
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

	//

	for(AppDataSource* source : m_appDataSources)
	{
		TEST_PTR_CONTINUE(source);

		Network::AppDataSourceState* protoInfo = reply->add_appdatasourcestate();
		source->getState(protoInfo);
	}

	return grpc::Status::OK;
}

grpc::Status GrpcAppDataSrv::GetServerTime(grpc::ServerContext* context,
											const Grpc::GetServerTimeRequest* request,
											Grpc::GetServerTimeReply* reply)
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

	reply->set_servertimeutc(currentMSecsUTC());
	reply->set_servertimelocal(currentMSecsLocal());

	return grpc::Status::OK;
}

grpc::Service* GrpcAppDataSrv::getGrpcService()
{
	return this;
}

QString GrpcAppDataSrv::serviceName() const
{
	return QStringLiteral("GrpcAppDataSrv");
}


