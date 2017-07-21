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
	m_timer(this),
	m_connectionKeepAliveTimer(this)
{
}

void TcpArchiveClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	switch(requestID)
	{
	case ARCHS_SAVE_APP_SIGNALS_STATES:
		onSaveAppSignalsStatesReply(replyData, replyDataSize);
		break;

	case ARCHS_CONNECTION_ALIVE:
		break;

	default:
		assert(false);
	}
}

void TcpArchiveClient::onClientThreadStarted()
{
	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveClient thread started (channel %1, archive server %2)").
								arg(m_channel + 1).arg(serverAddressPort(0).addressPortStr()));

	connect(&m_timer, &QTimer::timeout, this, &TcpArchiveClient::onTimer);
	connect(&m_connectionKeepAliveTimer, &QTimer::timeout, this, &TcpArchiveClient::onConnectionKeepAliveTimer);
	connect(&m_signalStatesQueue, &AppSignalStatesQueue::queueNotEmpty, this, &TcpArchiveClient::onSignalStatesQueueIsNotEmpty);

	m_timer.setInterval(10);
	m_timer.start();

	setWatchdogTimerTimeout(10000);

	m_connectionKeepAliveTimer.setSingleShot(true);
	m_connectionKeepAliveTimer.start(5000);
}

void TcpArchiveClient::onClientThreadFinished()
{
	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveClient thread finished (channel %1, archive server %2)").
								arg(m_channel + 1).arg(serverAddressPort(0).addressPortStr()));
}

void TcpArchiveClient::onConnection()
{
}

void TcpArchiveClient::sendSignalStatesToArchiveRequest()
{
	if (isClearToSendRequest() == false)
	{
		return;
	}

	if (m_signalStatesQueue.size() < 100)
	{
		return;
	}

	Network::SaveAppSignalsStatesToArchiveRequest request;

	int count = 0;

	do
	{
		SimpleAppSignalState state;

		bool res = m_signalStatesQueue.pop(&state);

		if (res == false)
		{
			break;
		}

		Proto::AppSignalState* appSignalState = request.add_appsignalstates();

		if (appSignalState == nullptr)
		{
			assert(false);
			break;
		}

		state.save(appSignalState);

		count++;
	}
	while(count < 1000);

	if (count == 0)
	{
		return;
	}

	request.set_clientequipmentid(equipmentID().toStdString());

	sendRequest(ARCHS_SAVE_APP_SIGNALS_STATES, request);

	m_connectionKeepAliveTimer.start(5000);

	qDebug() << "Send SaveSignalsToArchive count = " << count;
}

void TcpArchiveClient::onSaveAppSignalsStatesReply(const char* replyData, quint32 replyDataSize)
{
	Network::SaveAppSignalsStatesToArchiveReply msg;

	msg.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	NetworkError errorCode = static_cast<NetworkError>(msg.error());

	if (errorCode == NetworkError::Success)
	{
		sendSignalStatesToArchiveRequest();
	}
	else
	{
		m_saveAppSignalsStateErrorReplyCount = 0;

		// in future, may be, depends to error code:
		//
		//  1) Save the perivous request message
		//  2) Try again to send "save" request to prevent signal states loosing
	}
}

void TcpArchiveClient::onTimer()
{
	sendSignalStatesToArchiveRequest();
}

void TcpArchiveClient::onConnectionKeepAliveTimer()
{
	if (isClearToSendRequest() == true)
	{
		sendRequest(ARCHS_CONNECTION_ALIVE);

		m_connectionKeepAliveTimer.start(5000);
	}
}

void TcpArchiveClient::onSignalStatesQueueIsNotEmpty()
{
	sendSignalStatesToArchiveRequest();
}



