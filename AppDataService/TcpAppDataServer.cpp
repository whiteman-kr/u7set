#include "TcpAppDataServer.h"


// -------------------------------------------------------------------------------
//
// TcpAppDataServer class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServer::TcpAppDataServer()
{

}


TcpAppDataServer::~TcpAppDataServer()
{
}


Tcp::Server* TcpAppDataServer::getNewInstance()
{
	TcpAppDataServer* newServer =  new TcpAppDataServer();

	newServer->setThread(m_thread);

	return newServer;
}


void TcpAppDataServer::onServerThreadStarted()
{
	m_signalCount = m_thread->appSignalIDsCount();
	m_signalListPartCount = getSignalListPartCount(m_signalCount);

	qDebug() << "TcpAppDataServer::onServerThreadStarted()";
}


void TcpAppDataServer::onServerThreadFinished()
{
	qDebug() << "TcpAppDataServer::onServerThreadFinished()";
}


void TcpAppDataServer::onConnection()
{
	qDebug() << "TcpAppDataServer::onConnection()";
}


void TcpAppDataServer::onDisconnection()
{
	qDebug() << "TcpAppDataServer::onDisconnection()";
}


void TcpAppDataServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	switch(requestID)
	{
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
		onGetAppSignalStateRequest(requestData, requestDataSize);
		break;

	case ADS_GET_DATA_SOURCES_INFO:
		onGetDataSourcesInfoRequest();
		break;

	case ADS_GET_DATA_SOURCES_STATES:
		onGetDataSourcesStatesRequest();
		break;

	case ADS_GET_UNITS:
		onGetUnitsRequest();
		break;

	default:
		assert(false);
		break;
	}
}


void TcpAppDataServer::onGetAppSignalListStartRequest()
{
	m_getSignalListStartReply.set_totalitemcount(m_signalCount);

	m_getSignalListStartReply.set_partcount(m_signalListPartCount);

	m_getSignalListStartReply.set_itemsperpart(ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART);

	m_getSignalListStartReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_getSignalListStartReply);
}


void TcpAppDataServer::onGetAppSignalListNextRequest(const char* requestData, quint32 requestDataSize)
{
	bool result = m_getSignalListNextRequest.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

	m_getSignalListNextReply.Clear();

	if (result == false)
	{
		m_getSignalListNextReply.set_error(TO_INT(NetworkError::ParseRequestError));
		sendReply(m_getSignalListNextReply);
		return;
	}

	int requestPartNo = m_getSignalListNextRequest.part();

	if (requestPartNo < 0 ||  requestPartNo >= m_signalListPartCount)
	{
		m_getSignalListNextReply.set_error(TO_INT(NetworkError::WrongPartNo));
		sendReply(m_getSignalListNextReply);
		return;
	}

	int itemsInPart = m_signalCount - requestPartNo * ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART;

	if (itemsInPart > ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART)
	{
		itemsInPart = ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART;
	}

	m_getSignalListNextReply.set_part(requestPartNo);

	const QVector<QString>& IDs = appSignalIDs();

	int endIndex = requestPartNo * ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART + itemsInPart;

	for(int i = requestPartNo * ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART; i < endIndex; i++ )
	{
		m_getSignalListNextReply.add_appsignalids(IDs[i].toStdString());
	}

	m_getSignalListNextReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_getSignalListNextReply);
}


void TcpAppDataServer::onGetAppSignalParamRequest(const char* requestData, quint32 requestDataSize)
{
	bool result = m_getAppSignalParamRequest.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

	m_getAppSignalParamReply.Clear();

	if (result == false)
	{
		m_getAppSignalParamReply.set_error(TO_INT(NetworkError::ParseRequestError));
		sendReply(m_getAppSignalParamReply);
		return;
	}

	int hashesCount = m_getAppSignalParamRequest.signalhashes_size();

	if (hashesCount > ADS_GET_APP_SIGNAL_PARAM_MAX)
	{
		m_getAppSignalParamReply.set_error(TO_INT(NetworkError::RequestParamExceed));
		sendReply(m_getAppSignalParamReply);
		return;
	}

	for(int i = 0; i < hashesCount; i++)
	{
		Hash hash = m_getAppSignalParamRequest.signalhashes(i);

		const Signal* signal = appSignals().getSignal(hash);

		if (signal == nullptr)
		{
			continue;
		}

		Proto::AppSignal* appSignalParam = m_getAppSignalParamReply.add_appsignals();

		signal->serializeTo(appSignalParam);
	}

	sendReply(m_getAppSignalParamReply);
}

void TcpAppDataServer::onGetAppSignalRequest(const char* requestData, quint32 requestDataSize)
{
	bool result = m_getAppSignalRequest.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

	m_getAppSignalReply.Clear();

	if (result == false)
	{
		m_getAppSignalReply.set_error(TO_INT(NetworkError::ParseRequestError));
		sendReply(m_getAppSignalReply);
		return;
	}

	int hashesCount = m_getAppSignalRequest.signalhashes_size();

	if (hashesCount > ADS_GET_APP_SIGNAL_PARAM_MAX)
	{
		m_getAppSignalReply.set_error(TO_INT(NetworkError::RequestParamExceed));
		sendReply(m_getAppSignalReply);
		return;
	}

	for(int i = 0; i < hashesCount; i++)
	{
		Hash hash = m_getAppSignalRequest.signalhashes(i);

		const Signal* signal = appSignals().getSignal(hash);

		if (signal == nullptr)
		{
			continue;
		}

		Proto::AppSignal* appSignal = m_getAppSignalReply.add_appsignals();

		signal->serializeTo(appSignal);
	}

	sendReply(m_getAppSignalReply);
}


void TcpAppDataServer::onGetAppSignalStateRequest(const char* requestData, quint32 requestDataSize)
{
	bool result = m_getAppSignalStateRequest.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

	m_getAppSignalStateReply.Clear();

	if (result == false)
	{
		m_getAppSignalStateReply.set_error(TO_INT(NetworkError::ParseRequestError));
		sendReply(m_getAppSignalStateReply);
		return;
	}

	int hashesCount = m_getAppSignalStateRequest.signalhashes_size();

	if (hashesCount > ADS_GET_APP_SIGNAL_STATE_MAX)
	{
		m_getAppSignalStateReply.set_error(TO_INT(NetworkError::RequestParamExceed));
		sendReply(m_getAppSignalStateReply);
		return;
	}

	for(int i = 0; i < hashesCount; i++)
	{
		Hash hash = m_getAppSignalStateRequest.signalhashes(i);

		AppSignalState appSignalState;

		result = getAppSignalStateState(hash, appSignalState);

		if (result == false)
		{
			//assert(false);			// unknown hash
			continue;
		}

		Proto::AppSignalState* protoAppSignalState = m_getAppSignalStateReply.add_appsignalstates();

		appSignalState.save(protoAppSignalState);
	}

	sendReply(m_getAppSignalStateReply);

	static int ctr = 0;

	ctr++;

	if ((ctr % 100) == 0)
	{
		qDebug() << "Send states" << ctr;
	}
}


int TcpAppDataServer::getSignalListPartCount(int signalCount)
{
	return signalCount / ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART +
			((signalCount % ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART) == 0 ? 0 : 1);
}


const QVector<QString>& TcpAppDataServer::appSignalIDs() const
{
	return m_thread->appSignalIDs();
}


const AppSignals& TcpAppDataServer::appSignals() const
{
	return m_thread->appSignals();
}


const AppDataSourcesIP& TcpAppDataServer::appDataSources() const
{
	return m_thread->appDataSources();
}


bool TcpAppDataServer::getAppSignalStateState(Hash hash, AppSignalState& state)
{
	return m_thread->getAppSignalState(hash, state);
}


void TcpAppDataServer::onGetDataSourcesInfoRequest()
{
	m_getDataSourcesInfoReply.Clear();

	const AppDataSourcesIP& dataSources = appDataSources();

	for(const DataSource* source : dataSources)
	{
		Network::DataSourceInfo* protoInfo = m_getDataSourcesInfoReply.add_datasourceinfo();
		source->getInfo(protoInfo);
	}

	m_getDataSourcesInfoReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_getDataSourcesInfoReply);
}


void TcpAppDataServer::onGetDataSourcesStatesRequest()
{
	m_getAppDataSourcesStatesReply.Clear();

	const AppDataSourcesIP& dataSources = appDataSources();

	for (const AppDataSource* source : dataSources)
	{
		Network::AppDataSourceState* state = m_getAppDataSourcesStatesReply.add_appdatasourcesstates();
		source->getState(state);
	}

	m_getAppDataSourcesStatesReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_getAppDataSourcesStatesReply);
}


void TcpAppDataServer::onGetUnitsRequest()
{
	m_getUnitsReply.Clear();

	sendReply(m_getUnitsReply);
}


// -------------------------------------------------------------------------------
//
// TcpAppDataServerThread class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServerThread::TcpAppDataServerThread(const HostAddressPort& listenAddressPort,
												TcpAppDataServer* server,
												const AppDataSourcesIP& appDataSources,
												const AppSignals& appSignals,
												const AppSignalStates& appSignalStates,
												std::shared_ptr<CircularLogger> logger) :
	Tcp::ServerThread(listenAddressPort, server, logger),
	m_appDataSources(appDataSources),
	m_appSignals(appSignals),
	m_appSignalStates(appSignalStates)
{
	server->setThread(this);
	buildAppSignalIDs();
}


void TcpAppDataServerThread::buildAppSignalIDs()
{
	m_appSignalIDs.clear();

	m_appSignalIDs.resize(m_appSignals.count());

	int i = 0;

	for(Signal* signal : m_appSignals)
	{
		m_appSignalIDs[i] = signal->appSignalID();

		i++;
	}
}


bool TcpAppDataServerThread::getAppSignalState(Hash hash, AppSignalState& state)
{
	return m_appSignalStates.getCurrentState(hash, state);
}
