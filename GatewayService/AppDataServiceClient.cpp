#include <QTimer>

#include "AppDataServiceClient.h"
#include "IvsImpulseGatewayHandler.h"

namespace Gateway
{
	AppDataServiceClient::AppDataServiceClient(const SoftwareInfo& softwareInfo,
												const HostAddressPort& serverAddressPort1,
												const HostAddressPort& serverAddressPort2,
												const QString& clientDescription,
												Handler& handler,
												CircularLoggerShared logger) :
		Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2, clientDescription),
		m_handler(handler)
	{
		setLogger(logger);
	}

	void AppDataServiceClient::onClientThreadStarted()
	{
		m_timer = std::make_unique<QTimer>(this);
		m_timer->setSingleShot(true);

		connect(m_timer.get(), &QTimer::timeout, this, &AppDataServiceClient::onTimer);

		m_handler.planNextPreparedRequest(m_request);

		sendRequest();
	}

	void AppDataServiceClient::onClientThreadFinished()
	{
		m_timer->stop();
	}

	void AppDataServiceClient::onConnection()
	{
		Tcp::Client::onConnection();

		m_handler.onAppDataSrvConnected();

		m_handler.planNextPreparedRequest(m_request);
		sendRequest();
	}

	void AppDataServiceClient::onDisconnection()
	{
		m_timer->stop();

		m_handler.onAppDataSrvDisconnected();

		Tcp::Client::onDisconnection();
	}

	void AppDataServiceClient::onTimer()
	{
		if (m_isWaitReplyTimeout == true)
		{
			m_handler.planNextPreparedRequest(m_request);
			m_isWaitReplyTimeout = false;
		}

		sendRequest();
	}

	void AppDataServiceClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		switch(requestID)
		{
		case ADS_GET_APP_SIGNAL_STATE:
			onGetAppSignalStateReply(replyData, replyDataSize);
			break;

		case ADS_GET_APP_SIGNAL_STATE_CHANGES:
			onGetAppSignalStateChangesReply(replyData, replyDataSize);
			break;

		case ADS_GET_APP_SIGNAL_STATE_CONST_SIZE:
			onGetAppSignalStateReply(replyData, replyDataSize);
			break;

		case ADS_GATEWAY_GET_APP_SIGNAL_STATE_CHANGES:
			onGatewayGetAppSignalStateChangesReply(replyData, replyDataSize);
			break;

		default:
			Q_ASSERT(false);
		}

		m_handler.planNextPreparedRequest(m_request);

		sendRequest();
	}

	void AppDataServiceClient::onGetAppSignalStateReply(const char* replyData, quint32 replyDataSize)
	{
		m_getStatesReply.Clear();

		bool result = m_getStatesReply.ParseFromArray(replyData, replyDataSize);

		if (result == false)
		{
			Q_ASSERT(false);
			return;
		}

		m_handler.updateSignalStates(m_getStatesReply);
	}

	void AppDataServiceClient::onGetAppSignalStateChangesReply(const char* replyData, quint32 replyDataSize)
	{
		m_getStateChangesReply.Clear();

		bool result = m_getStateChangesReply.ParseFromArray(replyData, replyDataSize);

		if (result == false)
		{
			Q_ASSERT(false);
			return;
		}

		m_handler.processStateChanges(m_getStateChangesReply);
	}

	void AppDataServiceClient::onGatewayGetAppSignalStateChangesReply(const char* replyData, quint32 replyDataSize)
	{
		m_gwGetStateChangesReply.Clear();

		bool result = m_gwGetStateChangesReply.ParseFromArray(replyData, replyDataSize);

		if (result == false)
		{
			Q_ASSERT(false);
			return;
		}

		m_handler.processGatewayStateChanges(m_gwGetStateChangesReply);

		emit sendStateChanges();
	}

	void AppDataServiceClient::sendRequest()
	{
		if (m_request.hasRequest() == false)
		{
			if (m_request.delayMs > 0)
			{
				m_timer->start(m_request.delayMs);
				m_request.delayMs = 0;
				return;
			}

			m_timer->start(TIMER_IDLE_INTERVAL);
			return;
		}

		if (m_request.delayMs == 0)
		{
			if (isClearToSendRequest() == false)
			{
				m_timer->start(TIMER_WAIT_CLEAR_TO_SEND_INTERVAL);
				return;
			}

			Tcp::Client::sendRequest(m_request.ID, m_request.data);

//			DEBUG_LOG_MSG(log(), QString("=== REquest %1 time %2").arg(m_request.ID).arg(QDateTime::currentMSecsSinceEpoch()));

			m_request.clear();

			m_timer->start(TIMER_WAIT_REPLY_TIMEOUT);
			m_isWaitReplyTimeout = true;
			return;
		}

		m_timer->start(m_request.delayMs);
		m_request.delayMs = 0;
	}
}
