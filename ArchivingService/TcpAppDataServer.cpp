#include "TcpAppDataServer.h"

// -------------------------------------------------------------------------------
//
// TcpAppDataServer class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServer::TcpAppDataServer(AppSignalStatesQueue& saveStatesQueue) :
	m_saveStatesQueue(saveStatesQueue)
{
}

Tcp::Server* TcpAppDataServer::getNewInstance()
{
	return new TcpAppDataServer(m_saveStatesQueue);
}

void TcpAppDataServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	switch(requestID)
	{
	case ARCHS_CONNECTION_ALIVE:
		sendReply();
		break;

	case ARCHS_SAVE_APP_SIGNALS_STATES:
		onSaveAppSignalsStatesToArchive(requestData, requestDataSize);
		break;

	default:
		assert(false);
	}
}

void TcpAppDataServer::onSaveAppSignalsStatesToArchive(const char* requestData, quint32 requestDataSize)
{
	bool result = m_saveStatesRequest.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

	m_saveStatesReply.Clear();

	if (result == false)
	{
		m_saveStatesReply.set_error(TO_INT(NetworkError::ParseRequestError));
		sendReply(m_saveStatesReply);
		return;
	}

	//
	// Check m_saveStatesRequest.clientequipmentid() here!
	//

	int statesCount = m_saveStatesRequest.appsignalstates_size();

	SimpleAppSignalState state;

	for(int i = 0; i < statesCount; i++)
	{
		state.load(m_saveStatesRequest.appsignalstates(i));

		m_saveStatesQueue.push(&state);
	}

	qDebug() << "Receive " << statesCount << " states to save";

	m_saveStatesReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_saveStatesReply);
}

void TcpAppDataServer::onConnection()
{
	qDebug() << C_STR(QString(tr("TcpAppDataServer connected")));
}

void TcpAppDataServer::onDisconnection()
{
	qDebug() << C_STR(QString(tr("TcpAppDataServer disconnected")));
}


// -------------------------------------------------------------------------------
//
// TcpAppDataServer class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServerThread::TcpAppDataServerThread(const HostAddressPort& listenAddressPort,
			 Tcp::Server* server,
			 std::shared_ptr<CircularLogger> logger) :
	Tcp::ServerThread(listenAddressPort, server, logger)
{

}

