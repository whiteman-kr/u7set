#include "TcpArchiveRequestsServer.h"

TcpArchiveRequestsServer::TcpArchiveRequestsServer(CircularLoggerShared logger) :
    m_logger(logger)
{
}

Tcp::Server* TcpArchiveRequestsServer::getNewInstance()
{
    return new TcpArchiveRequestsServer(m_logger);
}

void TcpArchiveRequestsServer::processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize)
{
    switch(requestID)
    {
	case ARCHS_GET_APP_SIGNALS_STATES_START:
		onGetSignalStatesFromArchiveStart(requestData, requestDataSize);
		break;

    default:
		assert(false);
    }
}

void TcpArchiveRequestsServer::onServerThreadStarted()
{
	DEBUG_LOG_MSG(m_logger, "TcpArchiveRequestsServer thread started");
}

void TcpArchiveRequestsServer::onServerThreadFinished()
{
	DEBUG_LOG_MSG(m_logger, "TcpArchiveRequestsServer thread finished");
}

void TcpArchiveRequestsServer::onGetSignalStatesFromArchiveStart(const char* requestData, quint32 requestDataSize)
{
	bool result = m_getSignalStatesRequest.ParseFromArray(reinterpret_cast<const void*>(requestData), requestDataSize);

	m_getSignalStatesReply.Clear();

	if (result == false)
	{
		m_getSignalStatesReply.set_error(static_cast<int>(NetworkError::ParseRequestError));
		sendReply(m_getSignalStatesReply);
		return;
	}

	m_getSignalStatesReply.set_error(static_cast<int>(NetworkError::Success));

/*	m_getSignalStatesReply.set_timetype(m_getSignalStatesRequest.timetype());
	m_getSignalStatesReply.set_starttime(m_getSignalStatesRequest.starttime());
	m_getSignalStatesReply.set_endtime(m_getSignalStatesRequest.endtime());*/

	sendReply(m_getSignalStatesReply);
}

TcpArchiveRequestsServerThread::TcpArchiveRequestsServerThread(const HostAddressPort& listenAddressPort,
			 Tcp::Server* server,
			 std::shared_ptr<CircularLogger> logger) :
	Tcp::ServerThread(listenAddressPort, server, logger)
{
}
