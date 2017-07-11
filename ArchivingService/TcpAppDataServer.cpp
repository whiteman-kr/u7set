#include "TcpAppDataServer.h"

// -------------------------------------------------------------------------------
//
// TcpAppDataServer class implementation
//
// -------------------------------------------------------------------------------

TcpAppDataServer::TcpAppDataServer()
{
}

Tcp::Server* TcpAppDataServer::getNewInstance()
{
	return new TcpAppDataServer();
}

void TcpAppDataServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
	switch(requestID)
	{
	case ARCHS_SAVE_APP_SIGNALS_STATES_TO_ARCHIVE:
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

	m_saveStatesReply.set_error(TO_INT(NetworkError::Success));

	sendReply(m_saveStatesReply);
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

