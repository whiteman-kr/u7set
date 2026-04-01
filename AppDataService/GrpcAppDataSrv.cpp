#include <chrono>
#include <QStringList>

#include <CommonLib/Times.h>

#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/SocketIO.h"
#include "../UtilsLib/WUtils.h"

#include "GrpcAppDataSrv.h"

// ------------------------------------------------------------------------------------
//
// SignalStateQueueGuard class implementation
//
// ------------------------------------------------------------------------------------

class SignalStateQueueGuard
{
public:
	SignalStateQueueGuard(AppDataReceiver* receiver, size_t queueSize, const std::string& authToken) :
		m_receiver(receiver)
	{
		if (m_receiver == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		m_queue = std::make_shared<SimpleAppSignalStatesQueue>(TO_INT(queueSize));
		m_receiver->registerDestSignalStatesQueue(m_queue, false,
												  QString("GrpcAppDataSrv[%1]::statesQueue").
												  arg(QString::fromStdString(authToken)));
	}

	~SignalStateQueueGuard()
	{
		if (m_receiver != nullptr && m_queue != nullptr)
		{
			m_receiver->unregisterDestSignalStatesQueue(m_queue);
		}
	}

	SimpleAppSignalStatesQueueShared queue() const { return m_queue; }

private:
	AppDataReceiver* m_receiver = nullptr;
	SimpleAppSignalStatesQueueShared m_queue;
};

// ------------------------------------------------------------------------------------
//
// DiscretesLogReaderGuard class implementation
//
// ------------------------------------------------------------------------------------

class DiscretesLogReaderGuard
{
public:
	DiscretesLogReaderGuard(std::shared_ptr<DiscretesLogWriter> dsLogWriter, CircularLoggerShared log) :
		m_dsLogWriter(dsLogWriter)
	{
		if (m_dsLogWriter == nullptr)
		{
			Q_ASSERT(false);
			return;
		}

		m_dsLogReader = std::make_shared<DiscretesLogReader>(log);
		m_dsLogWriter->registerLogReader(m_dsLogReader);
	}

	~DiscretesLogReaderGuard()
	{
		if (m_dsLogWriter != nullptr && m_dsLogReader != nullptr)
		{
			m_dsLogWriter->unregisterLogReader(m_dsLogReader);
		}
	}

	std::shared_ptr<DiscretesLogReader> reader() const { return m_dsLogReader; }

private:
	std::shared_ptr<DiscretesLogWriter> m_dsLogWriter;
	std::shared_ptr<DiscretesLogReader> m_dsLogReader;
};

// ------------------------------------------------------------------------------------
//
// GrpcAppDataSrv class implementation
//
// ------------------------------------------------------------------------------------

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
}

GrpcAppDataSrv::~GrpcAppDataSrv()
{
	unregisterAllQueues();
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

		if ((ctr & 0x3FFF) == 0)
		{
			if (context->IsCancelled() || isStopRequested())
			{
				logMsg("GetAppSignalList: context CANCELLED");
				return grpc::Status::CANCELLED;
			}
		}

		if (ctr >= IDS_MAX_COUNT)
		{
			if (isStopRequested())
			{
				return grpc::Status::CANCELLED;
			}

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
		if (isStopRequested())
		{
			return grpc::Status::CANCELLED;
		}

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

				if (context->IsCancelled() || isStopRequested())
				{
					return grpc::Status::CANCELLED;
				}

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

			if (context->IsCancelled() || isStopRequested())
			{
				return grpc::Status::CANCELLED;
			}

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

				if (context->IsCancelled() || isStopRequested())
				{
					return grpc::Status::CANCELLED;
				}

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

			if (context->IsCancelled() || isStopRequested())
			{
				return grpc::Status::CANCELLED;
			}

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
	return getAppSignalState(context, request, reply, false);
}

grpc::Status GrpcAppDataSrv::GetAppSignalStateConstSize(grpc::ServerContext* context,
										const Grpc::GetAppSignalStateRequest* request,
										Grpc::GetAppSignalStateReply* reply)
{
	return getAppSignalState(context, request, reply, true);
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

	std::string authToken;

	if (m_sessionGuard.extractAndValidateAuthToken(context, &authToken) == false)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, Grpc::INVALID_OR_EXPIRED_SESSION);
	}

	//

	WriteReplyPtr<Grpc::GetAppSignalStateChangesReply> writeReply = &WriteReplyFunc<Grpc::GetAppSignalStateChangesReply>;

	//

	SignalStateQueueGuard queueGuard(m_appDataReceiver, m_appSignals.count() * 5, authToken);
	SimpleAppSignalStatesQueueShared statesQueue = queueGuard.queue();

	if (statesQueue == nullptr)
	{
		return grpc::Status::CANCELLED;
	}

	//

	Grpc::GetAppSignalStateChangesReply reply;

	reply.mutable_appsignalstates()->Reserve(ADS_GET_APP_SIGNAL_STATE_MAX);

	grpc::Status writeStatus;
	SimpleAppSignalState state;

	int waitCtr = 0;

	auto lastSendTime = std::chrono::steady_clock::now();
	constexpr int SEND_PACKET_SIZE = 2048;
	constexpr std::chrono::milliseconds SEND_PACKET_INTERVAL{50};

	while(context->IsCancelled() == false && isStopRequested() == false)
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

	return grpc::Status::CANCELLED;
}

grpc::Status GrpcAppDataSrv::GetAppSignalStateChangesNoStream(grpc::ServerContext* context,
											  const Grpc::GetAppSignalStateChangesRequest* request,
											  Grpc::GetAppSignalStateChangesReply* reply)
{
	if (context == nullptr ||
		request == nullptr ||
		reply == nullptr ||
		m_appDataReceiver == nullptr)
	{
		Q_ASSERT(false);
		return grpc::Status::CANCELLED;
	}

	std::string authToken;

	if (m_sessionGuard.extractAndValidateAuthToken(context, &authToken) == false)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, Grpc::INVALID_OR_EXPIRED_SESSION);
	}

	reply->Clear();

	SimpleAppSignalStatesQueueShared queue;

	{
		std::lock_guard lg(m_signalStateChangesQueuesMutex);

		auto it = m_signalStateChangesQueues.find(authToken);

		if (it == m_signalStateChangesQueues.end())
		{
			queue = std::make_shared<SimpleAppSignalStatesQueue>(TO_INT(m_appSignals.count() * 5));
			m_signalStateChangesQueues.emplace(authToken, queue);

			m_appDataReceiver->registerDestSignalStatesQueue(queue, false,
															 QString("GrpcAppDataSrv[%1]::statesQueue").
															 arg(QString::fromStdString(authToken)));
		}
		else
		{
			queue = it->second;
		}
	}

	if (queue == nullptr)
	{
		return grpc::Status::CANCELLED;
	}

	SimpleAppSignalState state;

	int pendingStatesCount = 0;

	for(int i = 0; i < ADS_GET_APP_SIGNAL_STATE_MAX; i++)
	{
		if (queue->pop(&state) == false)
		{
			break;		// queue is empty - pendingStatesCount == 0
		}

		Proto::AppSignalState* protoState = reply->add_appsignalstates();

		state.save(protoState);

		if (i + 1 == ADS_GET_APP_SIGNAL_STATE_MAX)
		{
			// on last iteration set pendingStatesCount to actual value
			//
			pendingStatesCount = queue->size();
		}
	}

	reply->set_pendingstatescount(pendingStatesCount);

	return grpc::Status::OK;
}

grpc::Status GrpcAppDataSrv::GetGatewayAppSignalStateChanges(grpc::ServerContext* context,
											 const Grpc::GetGatewayAppSignalStateChangesRequest* request,
											 Grpc::GetGatewayAppSignalStateChangesReply* reply)
{
	if (context == nullptr ||
		request == nullptr ||
		reply == nullptr ||
		m_appDataReceiver == nullptr)
	{
		Q_ASSERT(false);
		return grpc::Status::CANCELLED;
	}

	std::string authToken;

	if (m_sessionGuard.extractAndValidateAuthToken(context, &authToken) == false)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, Grpc::INVALID_OR_EXPIRED_SESSION);
	}

	reply->Clear();

	GatewayAppSignalStatesQueueShared queue;

	{
		std::lock_guard lg(m_gwSignalStateChangesQueuesMutex);

		auto it = m_gwSignalStateChangesQueues.find(authToken);

		if (it != m_gwSignalStateChangesQueues.end())
		{
			queue = it->second;
		}

		if (request->signalhashes_size() != 0 &&
			queue != nullptr)
		{
			m_appDataReceiver->unregisterGatewaySignalStatesQueue(queue);
			m_gwSignalStateChangesQueues.erase(authToken);
			queue.reset();
		}

		if (request->signalhashes_size() != 0 &&
			queue == nullptr)
		{
			queue = std::make_shared<GatewayAppSignalStatesQueue>(10000);
			m_gwSignalStateChangesQueues.emplace(authToken, queue);

			std::set<Hash> hashes;

			for(const uint64_t h : request->signalhashes())
			{
				hashes.insert(static_cast<Hash>(h));
			}

			m_appDataReceiver->registerGatewaySignalStatesQueue(queue, hashes);
		}
	}

	if (queue == nullptr)
	{
		return grpc::Status::CANCELLED;
	}

	GatewayAppSignalStateQueueMask state;

	int pendingStatesCount = 0;

	for(int i = 0; i < ADS_GET_APP_SIGNAL_STATE_MAX; i++)
	{
		if (queue->pop(&state) == false)
		{
			break;		// queue is empty - pendingStatesCount == 0
		}

		Network::GatewayAppSignalState* protoState = reply->add_appsignalstates();

		state.gwState.saveToProto(protoState);

		if (i + 1 == ADS_GET_APP_SIGNAL_STATE_MAX)
		{
			// on last iteration set pendingStatesCount to actual value
			//
			pendingStatesCount = queue->size();
		}
	}

	reply->set_pendingstatescount(pendingStatesCount);

	return grpc::Status::OK;
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

	DiscretesLogReaderGuard dlReaderGuard(m_dsLogWriter, getLog());

	std::shared_ptr<DiscretesLogReader> dlReader = dlReaderGuard.reader();

	if (dlReader == nullptr)
	{
		return grpc::Status::CANCELLED;
	}

	//

	Grpc::GetDiscretesLogReply reply;

	reply.mutable_discreteslogrecord()->Reserve(ADS_GET_DISCRETES_LOG_MAX_RECORD_COUNT);

	grpc::Status writeStatus;
	auto lastSendTime = std::chrono::steady_clock::now();
	constexpr int SEND_PACKET_SIZE = 100;
	constexpr std::chrono::milliseconds SEND_PACKET_INTERVAL{200};

	while(context->IsCancelled() == false && isStopRequested() == false)
	{
		dlReader->getDiscretesLog(&reply);

		auto now = std::chrono::steady_clock::now();
		int addedRecordCount = reply.discreteslogrecord_size();

		if (addedRecordCount > SEND_PACKET_SIZE ||
			now - lastSendTime >= SEND_PACKET_INTERVAL)
		{
			if (writeReply(context, writer, reply, writeStatus, getLog()) == false)
			{
				return writeStatus;
			}

			lastSendTime = now;

			reply.Clear();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

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

grpc::Status GrpcAppDataSrv::getAppSignalState(grpc::ServerContext* context,
											const Grpc::GetAppSignalStateRequest* request,
											Grpc::GetAppSignalStateReply* reply,
											bool constSize)
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
			// unknown hash
			//
			if (constSize == false)
			{
				continue;
			}

			appSignalState.clear();
		}

		appSignalState.save(reply->add_appsignalstates());
	}

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

void GrpcAppDataSrv::eraseAuthToken(const std::string& authToken)
{
	{
		// remove SimpleAppSignalStatesQueueShared
		//
		std::lock_guard lg(m_signalStateChangesQueuesMutex);

		auto it = m_signalStateChangesQueues.find(authToken);

		if (it != m_signalStateChangesQueues.end())
		{
			m_appDataReceiver->unregisterDestSignalStatesQueue(it->second);
			m_signalStateChangesQueues.erase(authToken);
		}
	}

	{
		// remove GatewayAppSignalStatesQueueShared
		//
		std::lock_guard lg(m_gwSignalStateChangesQueuesMutex);

		auto it = m_gwSignalStateChangesQueues.find(authToken);

		if (it != m_gwSignalStateChangesQueues.end())
		{
			m_appDataReceiver->unregisterGatewaySignalStatesQueue(it->second);
			m_gwSignalStateChangesQueues.erase(authToken);
		}
	}
}

void GrpcAppDataSrv::unregisterAllQueues()
{
	{
		std::lock_guard lg(m_signalStateChangesQueuesMutex);

		for(auto [token, queue] : m_signalStateChangesQueues)
		{
			m_appDataReceiver->unregisterDestSignalStatesQueue(queue);
		}

		m_signalStateChangesQueues.clear();
	}

	{
		// remove GatewayAppSignalStatesQueueShared
		//
		std::lock_guard lg(m_gwSignalStateChangesQueuesMutex);

		for(auto [token,queue] : m_gwSignalStateChangesQueues)
		{
			m_appDataReceiver->unregisterGatewaySignalStatesQueue(queue);
		}

		m_gwSignalStateChangesQueues.clear();
	}
}



