#include "AppDataServiceClient.h"
#include "IvsImpulseGatewayHandler.h"

namespace Gateway
{
	AppDataServiceClient::AppDataServiceClient(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& serverAddressPort1,
											   const HostAddressPort& serverAddressPort2,
											   const QString& clientDescription,
											   Handler* handler,
											   CircularLoggerShared logger) :
		Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2, clientDescription),
		m_handler(handler),
		m_timer(this)
	{
		setLogger(logger);
	}

	void AppDataServiceClient::onClientThreadStarted()
	{
		std::set<Hash> hashes;

		m_handler->getRequiredSignalsHashes(&hashes);

		m_getStatesRequest.mutable_signalhashes()->Reserve(TO_INT(hashes.size()));

		for(Hash h : hashes)
		{
			m_getStatesRequest.add_signalhashes(h);
		}

		m_timer.setTimerType(Qt::PreciseTimer);
		m_timer.setInterval(GET_STATES_REQUEST_INTERVAL);
		m_timer.setSingleShot(false);

		connect(&m_timer, &QTimer::timeout, this, &AppDataServiceClient::onTimer);
	}

	void AppDataServiceClient::onClientThreadFinished()
	{
		m_timer.stop();
	}

	void AppDataServiceClient::onConnection()
	{
		Tcp::Client::onConnection();

		m_timer.start();

		//

		Network::GatewayGetAppSignalStateChangesRequest initialRequest;

		std::set<Hash> eventHashes;

		m_handler->getEventSignalsHashes(&eventHashes);

		initialRequest.mutable_signalshashes()->Reserve(TO_INT(eventHashes.size()));

		for(Hash h : eventHashes)
		{
			initialRequest.add_signalshashes(h);
		}

		sendRequest(ADS_GATEWAY_GET_APP_SIGNAL_STATE_CHANGES, initialRequest);
	}

	void AppDataServiceClient::onDisconnection()
	{
		Tcp::Client::onDisconnection();

		m_timer.stop();
	}

	void AppDataServiceClient::onTimer()
	{
		if (isClearToSendRequest() == false)
		{
			m_needGetStates = true;
			return;
		}

		sendGetStatesRequest();
	}

	void AppDataServiceClient::sendGetStatesRequest()
	{
		sendRequest(ADS_GET_APP_SIGNAL_STATE_CONST_SIZE, m_getStatesRequest);
		m_needGetStates = false;
		m_lastGetStatesRequestTime = QDateTime::currentMSecsSinceEpoch();
	}

	void AppDataServiceClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		switch(requestID)
		{
		case ADS_GET_APP_SIGNAL_STATE_CONST_SIZE:
			onGetAppSignalStateReply(replyData, replyDataSize);
			break;

		case ADS_GATEWAY_GET_APP_SIGNAL_STATE_CHANGES:
			onGatewayGetAppSignalStateChangesReply(replyData, replyDataSize);
			break;

		default:
			Q_ASSERT(false);
		}
	}

	void AppDataServiceClient::onGetAppSignalStateReply(const char* replyData, quint32 replyDataSize)
	{
		bool result = m_getStatesReply.ParseFromArray(replyData, replyDataSize);

		if (result == false)
		{
			Q_ASSERT(false);
			return;
		}

		m_handler->updateSignalStates(m_getStatesReply);

		sendRequest(ADS_GATEWAY_GET_APP_SIGNAL_STATE_CHANGES, m_gwGetStateChangesRequest);
	}

	void AppDataServiceClient::onGatewayGetAppSignalStateChangesReply(const char* replyData, quint32 replyDataSize)
	{
		bool result = m_gwGetStateChangesReply.ParseFromArray(replyData, replyDataSize);

		if (result == false)
		{
			Q_ASSERT(false);
			return;
		}

		DEBUG_LOG_MSG(log(), QString("state changes %1").arg(m_gwGetStateChangesReply.appsignalstates_size()));

		m_handler->processStateChanges(m_gwGetStateChangesReply);

		if (m_needGetStates == true)
		{
			sendGetStatesRequest();
		}
		else
		{
			if (m_gwGetStateChangesReply.pendingstatescount() > 0 &&
				(QDateTime::currentMSecsSinceEpoch() - m_lastGetStatesRequestTime <
											static_cast<qint64>(GET_STATES_REQUEST_INTERVAL * 0.9)))
			{
				sendRequest(ADS_GATEWAY_GET_APP_SIGNAL_STATE_CHANGES, m_gwGetStateChangesRequest);
			}
		}

		emit sendStateChanges();
	}
}
