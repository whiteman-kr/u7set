#include "TcpArchiveClient.h"
#include "AppDataService.h"

// ------------------------------------------------------------------------------
//
// TcpArchiveClient class implementation
//
// ------------------------------------------------------------------------------

TcpArchiveClient::TcpArchiveClient(const SoftwareInfo& softwareInfo,
								   const HostAddressPort& archiveSrviceAddressPort,
								   AppDataServiceWorker& appDataService) :
	Tcp::Client(softwareInfo, archiveSrviceAddressPort, "TcpArchiveClient"),
	m_appDataService(appDataService),
	m_logger(appDataService.logger()),
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
	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveClient thread started, archive server %1").
								arg(serverAddressPort(0).addressPortStr()));

	m_signalStatesQueue = std::make_shared<SimpleAppSignalStatesQueue>(10000);

	connect(m_signalStatesQueue.get(), &SimpleAppSignalStatesQueue::queueNotEmpty, this, &TcpArchiveClient::onSignalStatesQueueIsNotEmpty);

	m_appDataService.registerDestSignalStatesQueue(m_signalStatesQueue, true, "TcpArchiveClient");

	connect(&m_timer, &QTimer::timeout, this, &TcpArchiveClient::onTimer);

	m_timer.setInterval(2000);
	m_timer.start();
}

void TcpArchiveClient::onClientThreadFinished()
{
	m_appDataService.unregisterDestSignalStatesQueue(m_signalStatesQueue);

	DEBUG_LOG_MSG(m_logger, QString("TcpArchiveClient thread finished, archive server %1").
								arg(serverAddressPort(0).addressPortStr()));
}

bool TcpArchiveClient::sendSignalStatesToArchiveRequest(bool sendNow)
{
	if (isClearToSendRequest() == false)
	{
		return false;
	}

	if (sendNow == false && m_signalStatesQueue->size() < 200)
	{
		return false;
	}

	Network::SaveAppSignalsStatesToArchiveRequest request;

	int count = 0;

	do
	{
		SimpleAppSignalState state;

		bool res = m_signalStatesQueue->pop(&state);

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

	sendRequest(ARCHS_SAVE_APP_SIGNALS_STATES, request);

	m_timer.start();

	return true;
}

void TcpArchiveClient::onSaveAppSignalsStatesReply(const char* replyData, quint32 replyDataSize)
{
	Network::SaveAppSignalsStatesToArchiveReply msg;

	msg.ParseFromArray(replyData, replyDataSize);

	E::NetworkError errorCode = static_cast<E::NetworkError>(msg.error());

	if (errorCode == E::NetworkError::Success)
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

// ------------------------------------------------------------------------------
//
// TcpArchiveClient class implementation
//
// ------------------------------------------------------------------------------

Tcp::ConnectionState TcpArchiveClientThread::m_emptyState;

TcpArchiveClientThread::TcpArchiveClientThread(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& archiveServiceAddressPort,
											   AppDataServiceWorker& appDataService)
{
	m_tcpArchiveClient = new TcpArchiveClient(softwareInfo,
											   archiveServiceAddressPort,
											   appDataService);
	addWorker(m_tcpArchiveClient);

	setObjectName("TcpArchiveClientThread");
}

Tcp::ConnectionState TcpArchiveClientThread::getConnectionState()
{
	if (m_tcpArchiveClient != nullptr)
	{
		return m_tcpArchiveClient->getConnectionState();
	}

	return m_emptyState;
}
