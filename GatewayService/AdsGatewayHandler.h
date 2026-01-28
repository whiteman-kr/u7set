#pragma once

#include <asio/error_code.hpp>

#include <unordered_map>

#include "../GatewayLib/AdsGateway.h"

#include "GatewayHandler.h"
#include "AppDataServiceClient.h"
#include "AdsGatewayServer.h"

using namespace asio;

namespace Gateway
{
	class AdsGatewayHandler : public Handler
	{
	public:
		AdsGatewayHandler(const SoftwareInfo& swInfo,
						  const GatewayServiceSettings& settings,
						  AdsGatewayShared gateway,
						  const AppSignals& appSignals,
						  CircularLoggerShared log,
						  bool logGatewayPackets);

		virtual ~AdsGatewayHandler();

		virtual void run() override;
		virtual void shutdown() override;

		virtual void getRequiredSignalsHashes(std::set<Hash>* hashes) const override;
		virtual void getEventSignalsHashes(std::set<Hash>* hashes) const override;

		virtual void updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply) override;
		virtual void processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply) override;

		virtual void setConnectedToAppDataSrv(bool connected) override;

	private:
		bool init();

		void runAppDataSrvClient();
		void stopAppDataSrvClient();

		void runAdsGatewayServer();
		void stopAdsGatewayServer();

	private:
		const SoftwareInfo m_softwareInfo;
		HostAddressPort m_appDataService1;
		HostAddressPort m_appDataService2;

		AdsGatewayShared m_gateway;
		const AppSignals& m_appSignals;

		std::unique_ptr<AppDataServiceClientThread> m_appDataSrvClientThread;

		std::mutex m_adsGatewayServerMutex;
		std::unique_ptr<AdsGatewayServer> m_adsGatewayServer;

		//

		mutable std::set<Hash> m_hashesToUpdate;

		//

		QString m_logStr;

		friend class AppDataServiceClient;
	};

	using AdsGatewayHandlerShared = std::shared_ptr<AdsGatewayHandler>;
}
