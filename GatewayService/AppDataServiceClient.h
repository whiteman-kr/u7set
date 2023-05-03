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
							 AppSignalStates& states,
							 std::atomic_bool& signalStatesUpdated);
	private:
		virtual void onClientThreadStarted() override;
		virtual void onClientThreadFinished() override;

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		void onTimer();

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

		void onGetAppSignalStateReply(const char* replyData, quint32 replyDataSize);
		void onGatewayGetAppSignalStateChangesReply(const char* replyData, quint32 replyDataSize);

	private:
		QTimer m_timer;

		AppSignalStates& m_states;
		std::atomic_bool& m_signalStatesUpdated;

		Network::GetAppSignalStateRequest m_getStatesRequest;
		Network::GetAppSignalStateReply m_getStatesReply;

		Network::GatewayGetAppSignalStateChangesRequest m_gwGetStateChangesRequest;
		Network::GatewayGetAppSignalStateChangesReply m_gwGetStateChangesReply;
	};

	class AppDataServiceClientThread : public SimpleThread
	{
	public:
		AppDataServiceClientThread(const SoftwareInfo& softwareInfo,
								   const HostAddressPort& serverAddressPort1,
								   const HostAddressPort& serverAddressPort2,
								   const QString& clientDescription,
								   AppSignalStates& states,
								   std::atomic_bool& signalStatesUpdated)
		{
			addWorker(new AppDataServiceClient(softwareInfo,
											   serverAddressPort1,
											   serverAddressPort2,
											   clientDescription,
											   states,
											   signalStatesUpdated));
		}
	};
}
