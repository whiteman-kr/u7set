#pragma once

#include "../OnlineLib/Tcp.h"
#include "AppSignalState.h"
#include "GatewayHandler.h"

namespace Gateway
{
	class IvsImpulseListInfo;
	class IvsImpulseHandler;

	class AppDataServiceClient : public Tcp::Client
	{
		Q_OBJECT

		inline static const int GET_STATES_REQUEST_INTERVAL = 250;	// ms

	public:
		AppDataServiceClient(const SoftwareInfo& softwareInfo,
							 const HostAddressPort& serverAddressPort1,
							 const HostAddressPort& serverAddressPort2,
							 const QString& clientDescription,
							 Handler& handler,
							 CircularLoggerShared logger);
	private:
		virtual void onClientThreadStarted() override;
		virtual void onClientThreadFinished() override;

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		void onTimer();

		void sendGetStatesRequest();

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

		void onGetAppSignalStateReply(const char* replyData, quint32 replyDataSize);
		void onGatewayGetAppSignalStateChangesReply(const char* replyData, quint32 replyDataSize);

	signals:
		void sendStateChanges();

	private:
		Handler& m_handler;

		std::unique_ptr<QTimer> m_timer;
		bool m_needGetStates = false;
		qint64 m_lastGetStatesRequestTime = 0;

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
								   Handler& handler,
								   CircularLoggerShared logger)
		{
			addWorker(new AppDataServiceClient(softwareInfo,
											   serverAddressPort1,
											   serverAddressPort2,
											   clientDescription,
											   handler,
											   logger));
		}

		AppDataServiceClient* client()
		{
			Q_ASSERT(m_workers.size() == 1);

			return dynamic_cast<AppDataServiceClient*>(*m_workers.begin());
		}
	};
}
