#pragma once

#include "../OnlineLib/Tcp.h"
#include "../CommonLib/Hash.h"
#include "AppSignalState.h"

namespace Gateway
{
	class AppDataServiceClient : public Tcp::Client
	{
	public:
		AppDataServiceClient(const SoftwareInfo& softwareInfo,
							 const HostAddressPort& serverAddressPort1,
							 const HostAddressPort& serverAddressPort2,
							 const QString& clientDescription,
							 AppSignalStates& states);
	private:
		virtual void onClientThreadStarted() override;
		virtual void onClientThreadFinished() override;

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

		void onTimer();

	private:
		QTimer m_timer;
		int m_timerCtr = 0;

		std::vector<AppSignalStatesIterator> m_stateIterator;

		Network::GetAppSignalStateRequest m_getStatesRequest;
		Network::GetAppSignalStateChangesRequest m_getStateChangesRequest;
	};

}
