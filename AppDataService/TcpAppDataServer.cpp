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
		onGetSignalListStartRequest();
		break;

	case ADS_GET_APP_SIGNAL_LIST_NEXT:
		onGetSignalListNextRequest(requestData, requestDataSize);
		break;

	case ADS_GET_APP_SIGNAL_PARAM:
		onGetSignalParamRequest(requestData, requestDataSize);
		break;

	default:
		assert(false);
		break;
	}
}


void TcpAppDataServer::onGetSignalListStartRequest()
{
	m_getSignalListStartReply.set_totalitemcount(m_signalCount);

	m_getSignalListStartReply.set_partcount(m_signalListPartCount);

	m_getSignalListStartReply.set_itemsperpart(ADS_GET_APP_SIGNAL_LIST_ITEMS_PER_PART);

	m_getSignalListStartReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_getSignalListStartReply);
}


void TcpAppDataServer::onGetSignalListNextRequest(const char* requestData, quint32 requestDataSize)
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


void TcpAppDataServer::onGetSignalParamRequest(const char* requestData, quint32 requestDataSize)
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
		int hash = m_getAppSignalParamRequest.signalhashes(i);

		const Signal* signal = appSignals().getSignal(hash);

		if (signal == nullptr)
		{
			continue;
		}

		//	m_protoAppSignal.Clear();

		Proto::AppSignal* appSignal = m_getAppSignalParamReply.add_appsignalparams();

		signal->serializeToProtoAppSignal(appSignal);
	}

	sendReply(m_getAppSignalParamReply);
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


// -------------------------------------------------------------------------------
//
// TcpAppDataServerThread class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServerThread::TcpAppDataServerThread(	const HostAddressPort& listenAddressPort,
												TcpAppDataServer* server,
												const AppSignals &appSignals) :
	Tcp::ServerThread(listenAddressPort, server),
	m_appSignals(appSignals)
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
