#include "TcpArchiveClient.h"

TcpArchiveClient::TcpArchiveClient(const SoftwareInfo& softwareInfo,
								   const HostAddressPort& serverAddressPort,
								   SignalStatesProcessingThread* signalStatesProcessingThread,
								   CircularLoggerShared logger) :
	Tcp::Client(softwareInfo, serverAddressPort, "TcpArchiveClient"),
	m_signalStatesProcessingThread(signalStatesProcessingThread),
	m_logger(logger),
	m_timer(this)
{
	setObjectName("TcpArchiveClient");
}

void TcpArchiveClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
{
	switch(requestID)
	{
	case ARCHS_SAVE_APP_SIGNALS_STATES:
		onSaveAppSignalsStatesReply(replyData, replyDataSize);
		break;

	default:
		assert(false);
	}
}

void TcpArchiveClient::onClientThreadStarted()
{
	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveClient thread started, archive server %1)").
								arg(serverAddressPort(0).addressPortStr()));

	m_signalStatesQueue = std::make_shared<SimpleAppSignalStatesQueue>(10000);

	connect(m_signalStatesQueue.get(), &SimpleAppSignalStatesQueue::queueNotEmpty, this, &TcpArchiveClient::onSignalStatesQueueIsNotEmpty);

	if (m_signalStatesProcessingThread != nullptr)
	{
		m_signalStatesProcessingThread->registerDestSignalStatesQueue(m_signalStatesQueue, true, "TcpArchiveClient");
	}
	else
	{
		assert(false);
	}

	connect(&m_timer, &QTimer::timeout, this, &TcpArchiveClient::onTimer);

	m_timer.setInterval(2000);
	m_timer.start();
}

void TcpArchiveClient::onClientThreadFinished()
{
	if (m_signalStatesProcessingThread != nullptr)
	{
		m_signalStatesProcessingThread->unregisterDestSignalStatesQueue(m_signalStatesQueue, "TcpArchiveClient");
	}

	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveClient thread finished, archive server %1)").
								arg(serverAddressPort(0).addressPortStr()));
}

void TcpArchiveClient::onConnection()
{
}

bool TcpArchiveClient::sendSignalStatesToArchiveRequest(bool sendNow)
{
	if (isClearToSendRequest() == false)
	{
		return false;
	}

	const QThread* thread = QThread::currentThread();

	if (sendNow == false && m_signalStatesQueue->size(thread) < 200)
	{
		return false;
	}

	Network::SaveAppSignalsStatesToArchiveRequest request;

	int count = 0;

	do
	{
		SimpleAppSignalState state;

		bool res = m_signalStatesQueue->pop(&state, thread);

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
	while(count < 3000);

	if (count == 0)
	{
		return false;
	}

	request.set_clientequipmentid(equipmentID().toStdString());

//	qDebug() << "ARCHS_SAVE_APP_SIGNALS_STATES = " << count;

	sendRequest(ARCHS_SAVE_APP_SIGNALS_STATES, request);

	m_timer.start();

	return true;
}

void TcpArchiveClient::onSaveAppSignalsStatesReply(const char* replyData, quint32 replyDataSize)
{
	Network::SaveAppSignalsStatesToArchiveReply msg;

	msg.ParseFromArray(reinterpret_cast<const void*>(replyData), replyDataSize);

	NetworkError errorCode = static_cast<NetworkError>(msg.error());

	if (errorCode == NetworkError::Success)
	{
		sendSignalStatesToArchiveRequest(false);
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
	sendSignalStatesToArchiveRequest(true);		// send any quantity of states every 2 seconds
}

void TcpArchiveClient::onSignalStatesQueueIsNotEmpty()
{
	sendSignalStatesToArchiveRequest(false);
}

TcpArchiveClientThread::TcpArchiveClientThread(TcpArchiveClient* tcpArchiveClient) :
	SimpleThread(tcpArchiveClient),
	m_tcpArchiveClient(tcpArchiveClient)
{
}

Tcp::ConnectionState TcpArchiveClientThread::getConnectionState()
{
	if (m_tcpArchiveClient != nullptr)
	{
		return m_tcpArchiveClient->getConnectionState();
	}

	return m_dummyState;
}

void TcpArchiveClientThread::beforeQuit()
{
	m_tcpArchiveClient = nullptr;
}


