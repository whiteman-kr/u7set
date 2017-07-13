#include "TcpArchiveClient.h"

TcpArchiveClient::TcpArchiveClient(int channel,
								   const HostAddressPort& serverAddressPort,
								   E::SoftwareType softwareType,
								   const QString equipmentID,
								   int majorVersion,
								   int minorVersion,
								   int commitNo,
								   CircularLoggerShared logger,
								   AppSignalStatesQueue& signalStatesQueue) :
	Tcp::Client(serverAddressPort, softwareType, equipmentID, majorVersion, minorVersion, commitNo),
	m_channel(channel),
	m_logger(logger),
	m_signalStatesQueue(signalStatesQueue),
	m_timer(this)
{
}

void TcpArchiveClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
}

void TcpArchiveClient::onClientThreadStarted()
{
	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveClient thread started (channel %1, archive server %2)").
								arg(m_channel + 1).arg(serverAddressPort(0).addressPortStr()));

	connect(&m_timer, &QTimer::timeout, this, &TcpArchiveClient::onTimer);
	connect(&m_signalStatesQueue, &AppSignalStatesQueue::queueNotEmpty, this, &TcpArchiveClient::onSignalStatesQueueIsNotEmpty);

	m_timer.setInterval(10);
	m_timer.start();
}

void TcpArchiveClient::onClientThreadFinished()
{
	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveClient thread finished (channel %1, archive server %2)").
								arg(m_channel + 1).arg(serverAddressPort(0).addressPortStr()));
}

void TcpArchiveClient::sendSignalStatesToArchive()
{
	if (isClearToSendRequest() == false)
	{
		return;
	}

	int count = 0;

	do
	{
		SimpleAppSignalState state;
		m_signalStatesQueue.pop(&state);
	}
	while(count < 1000);
}

void TcpArchiveClient::onTimer()
{
	sendSignalStatesToArchive();
}

void TcpArchiveClient::onSignalStatesQueueIsNotEmpty()
{
	sendSignalStatesToArchive();
}



