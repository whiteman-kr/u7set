#include "AppDataServiceClient.h"

namespace Gateway
{
	AppDataServiceClient::AppDataServiceClient(const SoftwareInfo& softwareInfo,
											   const HostAddressPort& serverAddressPort1,
											   const HostAddressPort& serverAddressPort2,
											   const QString& clientDescription,
											   AppSignalStates& states) :
		Tcp::Client(softwareInfo, serverAddressPort1, serverAddressPort2, clientDescription)
	{
		AppSignalStatesIterator it = states.begin();

		sqwsqwsqwwdsqwd
				dqwdwd

		while(it != states.end())
		{
			m_getStatesRequest.signalHashes.add();
			m_stateIterator.push_back(it);
			it++;
		}
	}

	void AppDataServiceClient::onClientThreadStarted()
	{
		connect(&m_timer, &QTimer::timeout, this, &AppDataServiceClient::onTimer);

		m_timer.setInterval(100);
		m_timer.start();
	}

	void AppDataServiceClient::onClientThreadFinished()
	{

	}

	void AppDataServiceClient::onConnection()
	{
		Tcp::Client::onConnection();

		m_timerCtr = 0;
	}

	void AppDataServiceClient::onDisconnection()
	{
		Tcp::Client::onDisconnection();
	}

	void AppDataServiceClient::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
	}

	void AppDataServiceClient::onTimer()
	{
		if (isClearToSendRequest() == false)
		{
			return;
		}

		bool periodicStatesRequest = (m_timerCtr & 3) == 0;

		if (periodicStatesRequest == true)
		{
			sendRequest(ADS_GET_APP_SIGNAL_STATE, m_getStatesRequest);
		}
		else
		{
			sendRequest(ADS_GET_APP_SIGNAL_STATE_CHANGES, m_getStateChangesRequest);
		}

		m_timerCtr++;
	}

}
