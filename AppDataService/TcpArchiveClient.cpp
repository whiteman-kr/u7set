#include "TcpArchiveClient.h"

TcpArchiveClient::TcpArchiveClient(int channel,
								   const HostAddressPort& serverAddressPort,
								   E::SoftwareType softwareType,
								   const QString equipmentID,
								   int majorVersion,
								   int minorVersion,
								   int commitNo,
								   CircularLoggerShared logger) :
	Tcp::Client(serverAddressPort, softwareType, equipmentID, majorVersion, minorVersion, commitNo),
	m_channel(channel),
	m_logger(logger)
{
}

void TcpArchiveClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
}

void TcpArchiveClient::onClientThreadStarted()
{
	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveClient thread started (channel %1, archive server %2)").
								arg(m_channel + 1).arg(serverAddressPort(0).addressPortStr()))
}

void TcpArchiveClient::onClientThreadFinished()
{
	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveClient thread finished (channel %1, archive server %2)").
								arg(m_channel + 1).arg(serverAddressPort(0).addressPortStr()))
}


