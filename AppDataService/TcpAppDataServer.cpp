#include "TcpAppDataServer.h"
#include "../Proto/network.pb.h"

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
	case ADS_GET_SIGNAL_LIST_START:
		onGetSignalListStartRequest();
		break;

	case ADS_GET_SIGNAL_LIST_NEXT:
		onGetSignalListNextRequest(requestData, requestDataSize);
		break;

	default:
		assert(false);
		break;
	}
}


void TcpAppDataServer::onGetSignalListStartRequest()
{
	Network::GetSignalListStartReply reply;

	reply.set_totalitemcount(m_signalCount);

	reply.set_partcount(m_signalListPartCount);

	reply.set_itemsperpart(GET_SIGNAL_LIST_ITEMS_PER_PART);

	reply.set_error(ERROR_OK);

	sendReply(reply);
}


void TcpAppDataServer::onGetSignalListNextRequest(const char* requestData, quint32 requestDataSize)
{
	Network::GetSignalListNextRequest request;

	bool result = request.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

	if (result == false)
	{
		qDebug() << "Error parse Network::GetSignalListNextRequest";
		return;
	}

	int requestPartNo = request.part();

	Network::GetSignalListNextReply reply;

	if (requestPartNo < 0 ||  requestPartNo >= m_signalListPartCount)
	{
		reply.set_error(ERROR_BAD_PART_NO);			// bad part number
		sendReply(reply);
		return;
	}

	int itemsInPart = m_signalCount - requestPartNo * GET_SIGNAL_LIST_ITEMS_PER_PART;

	if (itemsInPart > GET_SIGNAL_LIST_ITEMS_PER_PART)
	{
		itemsInPart = GET_SIGNAL_LIST_ITEMS_PER_PART;
	}

	reply.set_part(requestPartNo);

	const QVector<QString>& IDs = appSignalIDs();

	int endIndex = requestPartNo * GET_SIGNAL_LIST_ITEMS_PER_PART + itemsInPart;

	for(int i = requestPartNo * GET_SIGNAL_LIST_ITEMS_PER_PART; i < endIndex; i++ )
	{
		reply.add_appsignalids(IDs[i].toStdString());
	}

	reply.set_error(ERROR_OK);

	sendReply(reply);
}


int TcpAppDataServer::getSignalListPartCount(int signalCount)
{
	return signalCount / GET_SIGNAL_LIST_ITEMS_PER_PART +
			((signalCount % GET_SIGNAL_LIST_ITEMS_PER_PART) == 0 ? 0 : 1);
}


const QVector<QString>& TcpAppDataServer::appSignalIDs()
{
	return m_thread->appSignalIDs();
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
