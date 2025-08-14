#include "TcpAppDataServer.h"
#include "AppDataService.h"
#include "DiscretesLog.h"

// -------------------------------------------------------------------------------
//
// TcpAppDataServer class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServer::TcpAppDataServer(const SoftwareInfo& softwareInfo,
								   AppDataServiceWorker& appDataService) :
	Tcp::Server(softwareInfo, "AppDataServer"),
	m_appDataService(appDataService)
{
	setObjectName("TcpAppDataServer");
}

TcpAppDataServer::~TcpAppDataServer()
{
	deleteDiscretesLogReader();
}

void TcpAppDataServer::onServerThreadStarted()
{
	m_acquiredSignalCount = m_appDataService.acquiredAppSignalIDsCount();
	m_acquiredSignalListPartCount = getSignalListPartCount(m_acquiredSignalCount);

	qDebug() << "TcpAppDataServer::onServerThreadStarted()";
}

void TcpAppDataServer::onServerThreadFinished()
{
	if (m_signalStatesQueue != nullptr)
	{
		m_appDataService.unregisterDestSignalStatesQueue(m_signalStatesQueue);
	}

	if (m_gatewaySignalStatesQueue != nullptr)
	{
		m_appDataService.unregisterGatewaySignalStatesQueue(m_gatewaySignalStatesQueue);
	}

	qDebug() << "TcpAppDataServer::onServerThreadFinished()";
}

void TcpAppDataServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	switch(requestID)
	{
	case ADS_GET_STATE:
		onGetState();
		break;

	case RQID_GET_CLIENT_LIST:
		sendClientList();
		break;

	case ADS_GET_APP_SIGNAL_LIST_START:
		onGetAppSignalListStartRequest();
		break;

	case ADS_GET_APP_SIGNAL_LIST_NEXT:
		onGetAppSignalListNextRequest(requestData, requestDataSize);
		break;

	case ADS_GET_APP_SIGNAL_PARAM:
		onGetAppSignalParamRequest(requestData, requestDataSize);
		break;

	case ADS_GET_APP_SIGNAL:
		onGetAppSignalParamRequest(requestData, requestDataSize);
		break;

	case ADS_GET_APP_SIGNAL_STATE:
		onGetAppSignalStateRequest(requestData, requestDataSize, false);
		break;

	case ADS_GET_APP_SIGNAL_STATE_CONST_SIZE:
		onGetAppSignalStateRequest(requestData, requestDataSize, true);
		break;

	case ADS_GET_APP_SIGNAL_STATE_CHANGES:
		onGetAppSignalStateChangesRequest(requestData, requestDataSize);
		break;

	case ADS_GATEWAY_GET_APP_SIGNAL_STATE_CHANGES:
		onGatewayGetAppSignalStateChangesRequest(requestData, requestDataSize);
		break;

	case ADS_GET_APP_DATA_SOURCES_INFO:
		onGetAppDataSourcesInfoRequest();
		break;

	case ADS_GET_APP_DATA_SOURCES_STATES:
		onGetAppDataSourcesStatesRequest();
		break;

	case ADS_GET_SETTINGS:
		onGetSettings();
		break;

	case ADS_GET_DISCRETES_LOG:
		onGetDiscretesLog();
		break;

	default:
		logError(QString("unknown request ID = %1 (ignored)").arg(requestID));
	}
}

Tcp::Server* TcpAppDataServer::getNewInstance(const Tcp::ListenAddress& listenAddr)
{
	TcpAppDataServer* newServer =  new TcpAppDataServer(localSoftwareInfo(),
														m_appDataService);
	newServer->setListenAddress(listenAddr);
	return newServer;
}

void TcpAppDataServer::onGetState()
{
	thread_local Network::AppDataServiceState tl_getAppDataServiceState;

	quint32 ip = 0;
	quint16 port = 0;
	bool connected = m_appDataService.isConnectedToConfigurationService(ip, port);

	tl_getAppDataServiceState.set_cfgserviceisconnected(connected);
	if (connected)
	{
		tl_getAppDataServiceState.set_cfgserviceip(ip);
		tl_getAppDataServiceState.set_cfgserviceport(port);
	}

	connected = m_appDataService.isConnectedToArchiveService(ip, port);

	tl_getAppDataServiceState.set_archiveserviceisconnected(connected);
	if (connected)
	{
		tl_getAppDataServiceState.set_archiveserviceip(ip);
		tl_getAppDataServiceState.set_archiveserviceport(port);
	}

	Network::AppDataReceiveState* adrs = new Network::AppDataReceiveState();

	m_appDataService.fillAppDataReceiveState(adrs);

	tl_getAppDataServiceState.set_allocated_appdatareceivestate(adrs);

	sendReply(tl_getAppDataServiceState);
}

void TcpAppDataServer::onGetAppSignalListStartRequest()
{
	thread_local Network::GetSignalListStartReply tl_getSignalListStartReply;

	tl_getSignalListStartReply.set_totalitemcount(m_acquiredSignalCount);

	tl_getSignalListStartReply.set_partcount(m_acquiredSignalListPartCount);

	tl_getSignalListStartReply.set_itemsperpart(ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART);

	tl_getSignalListStartReply.set_error(TO_INT(E::NetworkError::Success));

	sendReply(tl_getSignalListStartReply);
}

void TcpAppDataServer::onGetAppSignalListNextRequest(const char* requestData, quint32 requestDataSize)
{
	thread_local Network::GetSignalListNextRequest tl_getSignalListNextRequest;
	thread_local Network::GetSignalListNextReply tl_getSignalListNextReply;

	bool result = tl_getSignalListNextRequest.ParseFromArray(requestData, requestDataSize);

	tl_getSignalListNextReply.Clear();

	if (result == false)
	{
		tl_getSignalListNextReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
		sendReply(tl_getSignalListNextReply);
		return;
	}

	int requestPartNo = tl_getSignalListNextRequest.part();

	if (requestPartNo < 0 ||  requestPartNo >= m_acquiredSignalListPartCount)
	{
		tl_getSignalListNextReply.set_error(TO_INT(E::NetworkError::WrongPartNo));
		sendReply(tl_getSignalListNextReply);
		return;
	}

	int itemsInPart = m_acquiredSignalCount - requestPartNo * ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART;

	if (itemsInPart > ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART)
	{
		itemsInPart = ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART;
	}

	tl_getSignalListNextReply.set_part(requestPartNo);

	const std::vector<QString>& IDs = m_appDataService.acquiredAppSignalIDs();

	int endIndex = requestPartNo * ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART + itemsInPart;

	for(int i = requestPartNo * ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART; i < endIndex; i++ )
	{
		tl_getSignalListNextReply.add_appsignalids(IDs[i].toStdString());
	}

	tl_getSignalListNextReply.set_error(TO_INT(E::NetworkError::Success));

	sendReply(tl_getSignalListNextReply);
}

void TcpAppDataServer::onGetAppSignalParamRequest(const char* requestData, quint32 requestDataSize)
{
	thread_local Network::GetAppSignalParamRequest tl_getAppSignalParamRequest;
	thread_local Network::GetAppSignalParamReply tl_getAppSignalParamReply;

	bool result = tl_getAppSignalParamRequest.ParseFromArray(requestData, requestDataSize);

	tl_getAppSignalParamReply.Clear();

	if (result == false)
	{
		tl_getAppSignalParamReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
		sendReply(tl_getAppSignalParamReply);
		return;
	}

	int hashesCount = tl_getAppSignalParamRequest.signalhashes_size();

	if (hashesCount > ADS_GET_APP_SIGNAL_PARAM_MAX)
	{
		tl_getAppSignalParamReply.set_error(TO_INT(E::NetworkError::RequestParamExceed));
		sendReply(tl_getAppSignalParamReply);
		return;
	}

	for(int i = 0; i < hashesCount; i++)
	{
		Hash hash = tl_getAppSignalParamRequest.signalhashes(i);

		const AppSignal* signal = m_appDataService.appSignals().getSignalByHash(hash);

		if (signal == nullptr)
		{
			continue;
		}

		Proto::AppSignal* appSignalParam = tl_getAppSignalParamReply.add_appsignals();

		signal->saveToProto(appSignalParam);
	}

	sendReply(tl_getAppSignalParamReply);
}

void TcpAppDataServer::onGetAppSignalRequest(const char* requestData, quint32 requestDataSize)
{
	thread_local Network::GetAppSignalRequest tl_getAppSignalRequest;
	thread_local Network::GetAppSignalReply tl_getAppSignalReply;

	bool result = tl_getAppSignalRequest.ParseFromArray(requestData, requestDataSize);

	tl_getAppSignalReply.Clear();

	if (result == false)
	{
		tl_getAppSignalReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
		sendReply(tl_getAppSignalReply);
		return;
	}

	int hashesCount = tl_getAppSignalRequest.signalhashes_size();

	if (hashesCount > ADS_GET_APP_SIGNAL_PARAM_MAX)
	{
		tl_getAppSignalReply.set_error(TO_INT(E::NetworkError::RequestParamExceed));
		sendReply(tl_getAppSignalReply);
		return;
	}

	for(int i = 0; i < hashesCount; i++)
	{
		Hash hash = tl_getAppSignalRequest.signalhashes(i);

		const AppSignal* signal = m_appDataService.appSignals().getSignalByHash(hash);

		if (signal == nullptr)
		{
			continue;
		}

		Proto::AppSignal* appSignal = tl_getAppSignalReply.add_appsignals();

		signal->saveToProto(appSignal);
	}

	sendReply(tl_getAppSignalReply);
}

void TcpAppDataServer::onGetAppSignalStateRequest(const char* requestData, quint32 requestDataSize, bool constSize)
{
	thread_local Network::GetAppSignalStateRequest tl_getAppSignalStateRequest;
	thread_local Network::GetAppSignalStateReply tl_getAppSignalStateReply;
	thread_local int tl_sentGetAppSignalStateReplyCount = 0;

	bool result = tl_getAppSignalStateRequest.ParseFromArray(requestData, requestDataSize);

	tl_getAppSignalStateReply.Clear();

	if (result == false)
	{
		tl_getAppSignalStateReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
		sendReply(tl_getAppSignalStateReply);
		return;
	}

	int hashesCount = tl_getAppSignalStateRequest.signalhashes_size();

	if (hashesCount > ADS_GET_APP_SIGNAL_STATE_MAX)
	{
		tl_getAppSignalStateReply.set_error(TO_INT(E::NetworkError::RequestParamExceed));
		sendReply(tl_getAppSignalStateReply);
		return;
	}

	if (hashesCount > 0)
	{
		tl_getAppSignalStateReply.mutable_appsignalstates()->Reserve(hashesCount);
	}

	const DynamicAppSignalStates& appSignalStates = m_appDataService.appSignalStates();

	for(int i = 0; i < hashesCount; i++)
	{
		Hash hash = tl_getAppSignalStateRequest.signalhashes(i);

		AppSignalState appSignalState;

		result = appSignalStates.getCurrentState(hash, appSignalState);

		if (constSize == false && result == false)
		{
			continue;	// unknown hash
		}

		Proto::AppSignalState* protoAppSignalState = tl_getAppSignalStateReply.add_appsignalstates();

		if (result == true)
		{
			appSignalState.save(protoAppSignalState);
		}
	}

	qint64 utc = 0;
	qint64 local = 0;

	getServerTimes(&utc, &local);

	tl_getAppSignalStateReply.set_servertimeutc(utc);
	tl_getAppSignalStateReply.set_servertimelocal(local);

	tl_getAppSignalStateReply.set_statechangesqueuesize(m_signalStatesQueue != nullptr ?
											m_signalStatesQueue->size(QThread::currentThread()) : 0);

	tl_getAppSignalStateReply.set_gatewaystatechangesqueuesize(m_gatewaySignalStatesQueue != nullptr ?
											m_gatewaySignalStatesQueue->size(QThread::currentThread()) : 0);

	sendReply(tl_getAppSignalStateReply);

	tl_sentGetAppSignalStateReplyCount++;

/*	if ((tl_sentGetAppSignalStateReplyCount % 100) == 0)
	{
		qDebug() << C_STR(QString("Send %1 get states replies to %2").
						  arg(tl_sentGetAppSignalStateReplyCount).
						  arg(connectedSoftwareInfo().equipmentID()));
	}*/
}

void TcpAppDataServer::onGetAppSignalStateChangesRequest(const char* requestData, quint32 requestDataSize)
{
	thread_local Network::GetAppSignalStateChangesRequest tl_getAppSignalStateChangesRequest;
	thread_local Network::GetAppSignalStateChangesReply tl_getAppSignalStateChangesReply;
	thread_local int tl_sentGetAppSignalStateChangesReplyCount = 0;

	if (m_signalStatesQueue == nullptr)
	{
		m_signalStatesQueue = std::make_shared<SimpleAppSignalStatesQueue>(10000);
		m_appDataService.registerDestSignalStatesQueue(m_signalStatesQueue, false,
			QString("TcpAppDataServer for %1 (%2)").
					arg(connectedSoftwareInfo().equipmentID()).
					arg(peerAddr().addressStr()));
	}

	Network::GetAppSignalStateChangesRequest& request =  tl_getAppSignalStateChangesRequest;

	bool result = request.ParseFromArray(requestData, requestDataSize);

	tl_getAppSignalStateChangesReply.Clear();

	if (result == false)
	{
		tl_getAppSignalStateChangesReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
		sendReply(tl_getAppSignalStateChangesReply);
		return;
	}

	QThread* thisThread = QThread::currentThread();

	SimpleAppSignalState state;

	int pendingStatesCount = 0;

	for(int i = 0; i < ADS_GET_APP_SIGNAL_STATE_MAX; i++)
	{
		result = m_signalStatesQueue->pop(&state, thisThread);

		if (result == false)
		{
			break;		// queue is empty - pendingStatesCount == 0
		}

		::Proto::AppSignalState* protoState = tl_getAppSignalStateChangesReply.add_appsignalstates();

		state.save(protoState);

		if (i + 1 == ADS_GET_APP_SIGNAL_STATE_MAX)
		{
			// on last iteration set pendingStatesCount to actual value
			//
			pendingStatesCount = m_signalStatesQueue->size(thisThread);
		}
	}

	tl_getAppSignalStateChangesReply.set_pendingstatescount(pendingStatesCount);

	qint64 utc = 0;
	qint64 local = 0;

	getServerTimes(&utc, &local);

	tl_getAppSignalStateChangesReply.set_servertimeutc(utc);
	tl_getAppSignalStateChangesReply.set_servertimelocal(local);

	sendReply(tl_getAppSignalStateChangesReply);

	tl_sentGetAppSignalStateChangesReplyCount++;

/*	if ((tl_sentGetAppSignalStateChangesReplyCount % 100) == 0)
	{
		qDebug() << C_STR(QString("Send %1 states changes replies to %2").
						  arg(tl_sentGetAppSignalStateChangesReplyCount).
						  arg(connectedSoftwareInfo().equipmentID()));
	} */
}

void TcpAppDataServer::onGatewayGetAppSignalStateChangesRequest(const char* requestData, quint32 requestDataSize)
{
	thread_local Network::GatewayGetAppSignalStateChangesRequest tl_gwGetAppSignalStateChangesRequest;
	thread_local Network::GatewayGetAppSignalStateChangesReply tl_gwGetAppSignalStateChangesReply;
	thread_local int tl_sentGatewayGetAppSignalStateChangesReplyCount = 0;

	auto& request = tl_gwGetAppSignalStateChangesRequest;

	auto& reply = tl_gwGetAppSignalStateChangesReply;

	bool result = request.ParseFromArray(requestData, requestDataSize);

	reply.Clear();

	if (result == false)
	{
		reply.set_error(TO_INT(E::NetworkError::ParseRequestError));
		sendReply(reply);
		return;
	}

	if (request.signalshashes_size() != 0)
	{
		if (m_gatewaySignalStatesQueue != nullptr)
		{
			m_appDataService.unregisterGatewaySignalStatesQueue(m_gatewaySignalStatesQueue);
			m_gatewaySignalStatesQueue.reset();
		}
	}

	if (m_gatewaySignalStatesQueue == nullptr &&
		request.signalshashes_size() != 0)
	{
		m_gatewaySignalStatesQueue = std::make_shared<GatewayAppSignalStatesQueue>(10000);

		std::set<Hash> hashes;

		int hashesCount = request.signalshashes_size();

		for(int i = 0; i < hashesCount; i++)
		{
			hashes.insert(request.signalshashes(i));
		}

		m_appDataService.registerGatewaySignalStatesQueue(m_gatewaySignalStatesQueue, hashes);
	}

	if (m_gatewaySignalStatesQueue == nullptr)
	{
		sendReply(reply);
		return;
	}

	QThread* thisThread = QThread::currentThread();

	GatewayAppSignalStateQueueMask state;

	int pendingStatesCount = 0;

	for(int i = 0; i < ADS_GET_APP_SIGNAL_STATE_MAX; i++)
	{
		result = m_gatewaySignalStatesQueue->pop(&state, thisThread);

		if (result == false)
		{
			break;		// queue is empty - pendingStatesCount == 0
		}

		::Network::GatewayAppSignalState* protoState = reply.add_appsignalstates();

		state.gwState.saveToProto(protoState);

		if (i + 1 == ADS_GET_APP_SIGNAL_STATE_MAX)
		{
			// on last iteration set pendingStatesCount to actual value
			//
			pendingStatesCount = m_gatewaySignalStatesQueue->size(thisThread);
		}
	}

	reply.set_pendingstatescount(pendingStatesCount);

	sendReply(reply);

	tl_sentGatewayGetAppSignalStateChangesReplyCount++;

/*	if ((tl_sentGatewayGetAppSignalStateChangesReplyCount % 100) == 0)
	{
		qDebug() << C_STR(QString("Send %1 gateway states changes replies to %2").
						  arg(tl_sentGatewayGetAppSignalStateChangesReplyCount).
						  arg(connectedSoftwareInfo().equipmentID()));
	} */
}

void TcpAppDataServer::onGetAppDataSourcesInfoRequest()
{
	thread_local Network::GetDataSourcesInfoReply tl_getDataSourcesInfoReply;

	tl_getDataSourcesInfoReply.Clear();

	const AppDataSources& dataSources = m_appDataService.appDataSources();

	for(AppDataSource* source : dataSources)
	{
		TEST_PTR_CONTINUE(source);

		Network::DataSourceInfo* protoInfo = tl_getDataSourcesInfoReply.add_datasourceinfo();
		source->saveToProto(protoInfo);
	}

	tl_getDataSourcesInfoReply.set_error(TO_INT(E::NetworkError::Success));

	sendReply(tl_getDataSourcesInfoReply);
}

void TcpAppDataServer::onGetAppDataSourcesStatesRequest()
{
	thread_local Network::GetAppDataSourcesStatesReply tl_getAppDataSourcesStatesReply;

	tl_getAppDataSourcesStatesReply.Clear();

	const AppDataSources& dataSources = m_appDataService.appDataSources();

	for(const AppDataSource* source : dataSources)
	{
		TEST_PTR_CONTINUE(source);

		Network::AppDataSourceState* state = tl_getAppDataSourcesStatesReply.add_appdatasourcesstates();
		source->getState(state);
	}

	tl_getAppDataSourcesStatesReply.set_error(TO_INT(E::NetworkError::Success));

	sendReply(tl_getAppDataSourcesStatesReply);
}

void TcpAppDataServer::onGetSettings()
{
	thread_local Network::ServiceSettings tl_getServiceSettings;

	tl_getServiceSettings.set_equipmentid(m_appDataService.equipmentID().toStdString());
	tl_getServiceSettings.set_configip1(m_appDataService.cfgServiceIP1().addressPortStr().toStdString());
	tl_getServiceSettings.set_configip2(m_appDataService.cfgServiceIP1().addressPortStr().toStdString());

	sendReply(tl_getServiceSettings);
}

void TcpAppDataServer::onGetDiscretesLog()
{
	createDiscretesLogReader();

	Network::GetDiscretesLogReply r;

	m_dlReader->getDiscretesLog(&r);

	sendReply(r);
}

int TcpAppDataServer::getSignalListPartCount(int signalCount)
{
	return signalCount / ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART +
			((signalCount % ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART) == 0 ? 0 : 1);
}

void TcpAppDataServer::getServerTimes(qint64* utc, qint64* local)
{
	TEST_PTR_RETURN(utc);
	TEST_PTR_RETURN(local);

	QDateTime currentTimeLocal = QDateTime::currentDateTime();

	*utc = currentTimeLocal.toMSecsSinceEpoch();

	currentTimeLocal.setTimeZone(TIME_ZONE_UTC);

	*local = currentTimeLocal.toMSecsSinceEpoch();
}

void TcpAppDataServer::createDiscretesLogReader()
{
	if (m_dlReader == nullptr)
	{
		m_dlReader = new DiscretesLogReader(m_appDataService.logger());

		m_appDataService.registerDiscretesLogReader(m_dlReader);
	}
}

void TcpAppDataServer::deleteDiscretesLogReader()
{
	if (m_dlReader != nullptr)
	{
		m_appDataService.unregisterDiscretesLogReader(m_dlReader);

		delete m_dlReader;

		m_dlReader = nullptr;
	}
}

// -------------------------------------------------------------------------------
//
// TcpAppDataServerThread class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServerThread::TcpAppDataServerThread(const SoftwareInfo& softwareInfo,
											   const std::vector<Tcp::ListenAddress>& listenAddresses,
											   AppDataServiceWorker& appDataServiceWorker) :
	Tcp::ListenerThread(listenAddresses,  new TcpAppDataServer(softwareInfo, appDataServiceWorker),
					  appDataServiceWorker.logger())
{
}

