#include "TcpAppDataServer.h"
#include "AppDataService.h"
#include "AppDataReceiver.h"

// -------------------------------------------------------------------------------
//
// TcpAppDataServer class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServer::TcpAppDataServer(const SoftwareInfo& softwareInfo,
								   E::SecurityLevel securityLevel,
								   AppDataServiceWorker& appDataService) :
	Tcp::Server(softwareInfo, securityLevel, "AppDataServer"),
	m_appDataService(appDataService)
{
	setObjectName("TcpAppDataServer");
}

TcpAppDataServer::~TcpAppDataServer()
{
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

	default:
		logError(QString("unknown request ID = %1 (ignored)").arg(requestID));
	}
}

Tcp::Server* TcpAppDataServer::getNewInstance()
{
	TcpAppDataServer* newServer =  new TcpAppDataServer(localSoftwareInfo(),
														securityLevel(),
														m_appDataService);
	return newServer;
}

void TcpAppDataServer::onGetState()
{
	quint32 ip = 0;
	quint16 port = 0;
	bool connected = m_appDataService.isConnectedToConfigurationService(ip, port);

	m_getAppDataServiceState.set_cfgserviceisconnected(connected);
	if (connected)
	{
		m_getAppDataServiceState.set_cfgserviceip(ip);
		m_getAppDataServiceState.set_cfgserviceport(port);
	}

	connected = m_appDataService.isConnectedToArchiveService(ip, port);

	m_getAppDataServiceState.set_archiveserviceisconnected(connected);
	if (connected)
	{
		m_getAppDataServiceState.set_archiveserviceip(ip);
		m_getAppDataServiceState.set_archiveserviceport(port);
	}

	Network::AppDataReceiveState* adrs = new Network::AppDataReceiveState();

	m_appDataService.fillAppDataReceiveState(adrs);

	m_getAppDataServiceState.set_allocated_appdatareceivestate(adrs);

	sendReply(m_getAppDataServiceState);
}

void TcpAppDataServer::onGetAppSignalListStartRequest()
{
	m_getSignalListStartReply.set_totalitemcount(m_acquiredSignalCount);

	m_getSignalListStartReply.set_partcount(m_acquiredSignalListPartCount);

	m_getSignalListStartReply.set_itemsperpart(ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART);

	m_getSignalListStartReply.set_error(TO_INT(E::NetworkError::Success));

	sendReply(m_getSignalListStartReply);
}

void TcpAppDataServer::onGetAppSignalListNextRequest(const char* requestData, quint32 requestDataSize)
{
	bool result = m_getSignalListNextRequest.ParseFromArray(requestData, requestDataSize);

	m_getSignalListNextReply.Clear();

	if (result == false)
	{
		m_getSignalListNextReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
		sendReply(m_getSignalListNextReply);
		return;
	}

	int requestPartNo = m_getSignalListNextRequest.part();

	if (requestPartNo < 0 ||  requestPartNo >= m_acquiredSignalListPartCount)
	{
		m_getSignalListNextReply.set_error(TO_INT(E::NetworkError::WrongPartNo));
		sendReply(m_getSignalListNextReply);
		return;
	}

	int itemsInPart = m_acquiredSignalCount - requestPartNo * ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART;

	if (itemsInPart > ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART)
	{
		itemsInPart = ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART;
	}

	m_getSignalListNextReply.set_part(requestPartNo);

	const std::vector<QString>& IDs = m_appDataService.acquiredAppSignalIDs();

	int endIndex = requestPartNo * ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART + itemsInPart;

	for(int i = requestPartNo * ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART; i < endIndex; i++ )
	{
		m_getSignalListNextReply.add_appsignalids(IDs[i].toStdString());
	}

	m_getSignalListNextReply.set_error(TO_INT(E::NetworkError::Success));

	sendReply(m_getSignalListNextReply);
}

void TcpAppDataServer::onGetAppSignalParamRequest(const char* requestData, quint32 requestDataSize)
{
	bool result = m_getAppSignalParamRequest.ParseFromArray(requestData, requestDataSize);

	m_getAppSignalParamReply.Clear();

	if (result == false)
	{
		m_getAppSignalParamReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
		sendReply(m_getAppSignalParamReply);
		return;
	}

	int hashesCount = m_getAppSignalParamRequest.signalhashes_size();

	if (hashesCount > ADS_GET_APP_SIGNAL_PARAM_MAX)
	{
		m_getAppSignalParamReply.set_error(TO_INT(E::NetworkError::RequestParamExceed));
		sendReply(m_getAppSignalParamReply);
		return;
	}

	for(int i = 0; i < hashesCount; i++)
	{
		Hash hash = m_getAppSignalParamRequest.signalhashes(i);

		const AppSignal* signal = m_appDataService.appSignals().getSignalByHash(hash);

		if (signal == nullptr)
		{
			continue;
		}

		Proto::AppSignal* appSignalParam = m_getAppSignalParamReply.add_appsignals();

		signal->saveToProto(appSignalParam);
	}

	sendReply(m_getAppSignalParamReply);
}

void TcpAppDataServer::onGetAppSignalRequest(const char* requestData, quint32 requestDataSize)
{
	bool result = m_getAppSignalRequest.ParseFromArray(requestData, requestDataSize);

	m_getAppSignalReply.Clear();

	if (result == false)
	{
		m_getAppSignalReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
		sendReply(m_getAppSignalReply);
		return;
	}

	int hashesCount = m_getAppSignalRequest.signalhashes_size();

	if (hashesCount > ADS_GET_APP_SIGNAL_PARAM_MAX)
	{
		m_getAppSignalReply.set_error(TO_INT(E::NetworkError::RequestParamExceed));
		sendReply(m_getAppSignalReply);
		return;
	}

	for(int i = 0; i < hashesCount; i++)
	{
		Hash hash = m_getAppSignalRequest.signalhashes(i);

		const AppSignal* signal = m_appDataService.appSignals().getSignalByHash(hash);

		if (signal == nullptr)
		{
			continue;
		}

		Proto::AppSignal* appSignal = m_getAppSignalReply.add_appsignals();

		signal->saveToProto(appSignal);
	}

	sendReply(m_getAppSignalReply);
}

void TcpAppDataServer::onGetAppSignalStateRequest(const char* requestData, quint32 requestDataSize, bool constSize)
{
	bool result = m_getAppSignalStateRequest.ParseFromArray(requestData, requestDataSize);

	m_getAppSignalStateReply.Clear();

	if (result == false)
	{
		m_getAppSignalStateReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
		sendReply(m_getAppSignalStateReply);
		return;
	}

	int hashesCount = m_getAppSignalStateRequest.signalhashes_size();

	if (hashesCount > ADS_GET_APP_SIGNAL_STATE_MAX)
	{
		m_getAppSignalStateReply.set_error(TO_INT(E::NetworkError::RequestParamExceed));
		sendReply(m_getAppSignalStateReply);
		return;
	}

	if (hashesCount > 0)
	{
		m_getAppSignalStateReply.mutable_appsignalstates()->Reserve(hashesCount);
	}

	const DynamicAppSignalStates& appSignalStates = m_appDataService.appSignalStates();

	for(int i = 0; i < hashesCount; i++)
	{
		Hash hash = m_getAppSignalStateRequest.signalhashes(i);

		AppSignalState appSignalState;

		result = appSignalStates.getCurrentState(hash, appSignalState);

		if (constSize == false && result == false)
		{
			continue;	// unknown hash
		}

		Proto::AppSignalState* protoAppSignalState = m_getAppSignalStateReply.add_appsignalstates();

		if (result == true)
		{
			appSignalState.save(protoAppSignalState);
		}
	}

	qint64 utc = 0;
	qint64 local = 0;

	getServerTimes(&utc, &local);

	m_getAppSignalStateReply.set_servertimeutc(utc);
	m_getAppSignalStateReply.set_servertimelocal(local);

	m_getAppSignalStateReply.set_statechangesqueuesize(m_signalStatesQueue != nullptr ?
											m_signalStatesQueue->size(QThread::currentThread()) : 0);

	m_getAppSignalStateReply.set_gatewaystatechangesqueuesize(m_gatewaySignalStatesQueue != nullptr ?
											m_gatewaySignalStatesQueue->size(QThread::currentThread()) : 0);

	sendReply(m_getAppSignalStateReply);

	m_sentGetAppSignalStateReplyCount++;

	if ((m_sentGetAppSignalStateReplyCount % 100) == 0)
	{
		qDebug() << C_STR(QString("Send %1 get states replies to %2").
						  arg(m_sentGetAppSignalStateReplyCount).
						  arg(connectedSoftwareInfo().equipmentID()));
	}
}

void TcpAppDataServer::onGetAppSignalStateChangesRequest(const char* requestData, quint32 requestDataSize)
{
	if (m_signalStatesQueue == nullptr)
	{
		m_signalStatesQueue = std::make_shared<SimpleAppSignalStatesQueue>(10000);
		m_appDataService.registerDestSignalStatesQueue(m_signalStatesQueue, false,
			QString("TcpAppDataServer for %1 (%2)").
					arg(connectedSoftwareInfo().equipmentID()).
					arg(peerAddr().addressStr()));
	}

	Network::GetAppSignalStateChangesRequest& request =  m_getAppSignalStateChangesRequest;

	bool result = request.ParseFromArray(requestData, requestDataSize);

	m_getAppSignalStateChangesReply.Clear();

	if (result == false)
	{
		m_getAppSignalStateChangesReply.set_error(TO_INT(E::NetworkError::ParseRequestError));
		sendReply(m_getAppSignalStateChangesReply);
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

		::Proto::AppSignalState* protoState = m_getAppSignalStateChangesReply.add_appsignalstates();

		state.save(protoState);

		if (i + 1 == ADS_GET_APP_SIGNAL_STATE_MAX)
		{
			// on last iteration set pendingStatesCount to actual value
			//
			pendingStatesCount = m_signalStatesQueue->size(thisThread);
		}
	}

	m_getAppSignalStateChangesReply.set_pendingstatescount(pendingStatesCount);

	qint64 utc = 0;
	qint64 local = 0;

	getServerTimes(&utc, &local);

	m_getAppSignalStateChangesReply.set_servertimeutc(utc);
	m_getAppSignalStateChangesReply.set_servertimelocal(local);

	sendReply(m_getAppSignalStateChangesReply);

	m_sentGetAppSignalStateChangesReplyCount++;

	if ((m_sentGetAppSignalStateChangesReplyCount % 100) == 0)
	{
		qDebug() << C_STR(QString("Send %1 states changes replies to %2").
						  arg(m_sentGetAppSignalStateChangesReplyCount).
						  arg(connectedSoftwareInfo().equipmentID()));
	}
}

void TcpAppDataServer::onGatewayGetAppSignalStateChangesRequest(const char* requestData, quint32 requestDataSize)
{
	auto& request = m_gwGetAppSignalStateChangesRequest;

	auto& reply = m_gwGetAppSignalStateChangesReply;

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

	m_sentGatewayGetAppSignalStateChangesReplyCount++;

	if ((m_sentGatewayGetAppSignalStateChangesReplyCount % 100) == 0)
	{
		qDebug() << C_STR(QString("Send %1 gateway states changes replies to %2").
						  arg(m_sentGatewayGetAppSignalStateChangesReplyCount).
						  arg(connectedSoftwareInfo().equipmentID()));
	}
}

void TcpAppDataServer::onGetAppDataSourcesInfoRequest()
{
	m_getDataSourcesInfoReply.Clear();

	const AppDataSources& dataSources = m_appDataService.appDataSources();

	for(AppDataSource* source : dataSources)
	{
		TEST_PTR_CONTINUE(source);

		Network::DataSourceInfo* protoInfo = m_getDataSourcesInfoReply.add_datasourceinfo();
		source->saveToProto(protoInfo);
	}

	m_getDataSourcesInfoReply.set_error(TO_INT(E::NetworkError::Success));

	sendReply(m_getDataSourcesInfoReply);
}

void TcpAppDataServer::onGetAppDataSourcesStatesRequest()
{
	m_getAppDataSourcesStatesReply.Clear();

	const AppDataSources& dataSources = m_appDataService.appDataSources();

	for(const AppDataSource* source : dataSources)
	{
		TEST_PTR_CONTINUE(source);

		Network::AppDataSourceState* state = m_getAppDataSourcesStatesReply.add_appdatasourcesstates();
		source->getState(state);
	}

	m_getAppDataSourcesStatesReply.set_error(TO_INT(E::NetworkError::Success));

	sendReply(m_getAppDataSourcesStatesReply);
}

void TcpAppDataServer::onGetSettings()
{
	m_getServiceSettings.set_equipmentid(m_appDataService.equipmentID().toStdString());
	m_getServiceSettings.set_configip1(m_appDataService.cfgServiceIP1().addressPortStr().toStdString());
	m_getServiceSettings.set_configip2(m_appDataService.cfgServiceIP1().addressPortStr().toStdString());

	sendReply(m_getServiceSettings);
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

	currentTimeLocal.setTimeSpec(Qt::UTC);

	*local = currentTimeLocal.toMSecsSinceEpoch();
}

// -------------------------------------------------------------------------------
//
// TcpAppDataServerThread class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServerThread::TcpAppDataServerThread(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& listenAddressPort,
											   E::SecurityLevel securityLevel,
											   AppDataServiceWorker& appDataServiceWorker) :
	Tcp::ServerThread(listenAddressPort,
					  new TcpAppDataServer(softwareInfo, securityLevel, appDataServiceWorker),
					  appDataServiceWorker.logger())
{
}

