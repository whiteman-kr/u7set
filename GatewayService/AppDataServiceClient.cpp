#include "AppDataServiceClient.h"

namespace Gateway
{
	AppDataServiceClient::AppDataServiceClient(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& serverAddressPort1,
											   const HostAddressPort& serverAddressPort2,
											   const QString& clientDescription,
											   AppSignalStates& states) :
		Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2, clientDescription),
		m_states(states),
		m_timer(this)
	{
	}

	void AppDataServiceClient::onClientThreadStarted()
	{
		m_getStatesRequest.mutable_signalhashes()->Reserve(TO_INT(m_states.size()));

		for(const AppSignalState& st : m_states)
		{
			m_getStatesRequest.add_signalhashes(st.hash());
		}

		m_timer.setTimerType(Qt::PreciseTimer);
		m_timer.setInterval(200);
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

		Network::GetAppSignalStateChangesRequest initialRequest;

		initialRequest.set_receivemode(TO_INT(SimpleAppSignalStatesQueue::ReceiveMode::SelectedSignals));
		initialRequest.mutable_selectedhashes()->Reserve(TO_INT(m_states.size()));

		for(const auto& st : m_states)
		{
			if (st.isWorkable() == true)
			{
				initialRequest.add_selectedhashes(st.hash());
			}
		}

		sendRequest(ADS_GET_APP_SIGNAL_STATE_CHANGES, initialRequest);
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
			return;
		}

		sendRequest(ADS_GET_APP_SIGNAL_STATE_CONST_SIZE, m_getStatesRequest);
	}

	void AppDataServiceClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		switch(requestID)
		{
		case ADS_GET_APP_SIGNAL_STATE_CONST_SIZE:
			onGetAppSignalStateReply(replyData, replyDataSize);
			break;

		case ADS_GET_APP_SIGNAL_STATE_CHANGES:
			onGetAppSignalStateChangesReply(replyData, replyDataSize);
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

		int replyStatesSize = m_getStatesReply.appsignalstates_size();

		if (replyStatesSize != TO_INT(m_states.size()))
		{
			Q_ASSERT(false);
			return;
		}

		for(int i = 0; i < replyStatesSize; i++)
		{
			m_states[i].updateState(m_getStatesReply.appsignalstates(i));
		}

		if (m_getStatesReply.statechangesqueuesize() != 0)
		{
			sendRequest(ADS_GET_APP_SIGNAL_STATE_CHANGES, m_getStateChangesRequest);
		}
	}

	void AppDataServiceClient::onGetAppSignalStateChangesReply(const char* replyData, quint32 replyDataSize)
	{
		bool result = m_getStateChangesReply.ParseFromArray(replyData, replyDataSize);

		if (result == false)
		{
			Q_ASSERT(false);
			return;
		}
	}
}
