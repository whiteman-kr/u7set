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

	case ADS_GET_APP_SIGNAL_STATE:
		onGetAppSignalStateRequest(requestData, requestDataSize);
		break;

	case ADS_GET_DATA_SOURCES_INFO:
		onGetDataSourcesInfoRequest();
		break;

	case ADS_GET_DATA_SOURCES_STATES:
		onGetDataSourcesStatesRequest(requestData, requestDataSize);
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

		Proto::AppSignal* appSignal = m_getAppSignalParamReply.add_appsignalparams();

		signal->serializeToProtoAppSignal(appSignal);
	}

	sendReply(m_getAppSignalParamReply);
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

		result = getConnectionState(hash, appSignalState);

		if (result == false)
		{
			assert(false);			// unknown hash
			continue;
		}

		Proto::AppSignalState* protoAppSignalState = m_getAppSignalStateReply.add_appsignalstates();

		appSignalState.setProtoAppSignalState(hash, protoAppSignalState);
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


const AppDataSources& TcpAppDataServer::appDataSources() const
{
	return m_thread->appDataSources();
}


bool TcpAppDataServer::getConnectionState(Hash hash, AppSignalState& state)
{
	return m_thread->getState(hash, state);
}


void TcpAppDataServer::onGetDataSourcesInfoRequest()
{
	m_getDataSourcesInfoReply.Clear();

	const AppDataSources& dataSources = appDataSources();

	for(const DataSource* source : dataSources)
	{
		Network::DataSourceInfo* protoInfo = m_getDataSourcesInfoReply.add_datasourceinfo();
		source->getDataSourceInfo(protoInfo);
	}

	m_getDataSourcesInfoReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_getDataSourcesInfoReply);
}


void TcpAppDataServer::onGetDataSourcesStatesRequest(const char* requestData, quint32 requestDataSize)
{

}



// -------------------------------------------------------------------------------
//
// TcpAppDataServerThread class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServerThread::TcpAppDataServerThread(const HostAddressPort& listenAddressPort,
												TcpAppDataServer* server,
												const AppDataSources& appDataSources,
												const AppSignals& appSignals,
												const AppSignalStates& appSignalStates) :
	Tcp::ServerThread(listenAddressPort, server),
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


bool TcpAppDataServerThread::getState(Hash hash, AppSignalState& state)
{
	return m_appSignalStates.getState(hash, state);
}
