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

		virtual void onAppDataSrvConnected() override;
		virtual void onAppDataSrvDisconnected() override;
		virtual void planNextPreparedRequest(PreparedRequest& request) override;

		virtual void getRequiredSignalsHashes(std::set<Hash>* hashes) const override;
		virtual void getEventSignalsHashes(std::set<Hash>* hashes) const override;

		virtual void updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply) override;
		virtual void processStateChanges(const Network::GetAppSignalStateChangesReply& getStateChangesReply) override;

	private:
		bool init();
		void prepareRequests();

		void runAdsGatewayServer();
		void stopAdsGatewayServer();

	private:
		AdsGatewayShared m_gateway;

		std::mutex m_adsGatewayServerMutex;
		std::unique_ptr<AdsGatewayServer> m_adsGatewayServer;

		std::vector<PreparedRequest> m_requests;
		size_t m_requestIndex = 0;

		bool m_hasPendingChanges = false;
		int m_chagesRequestCount = 0;

		//

		mutable std::set<Hash> m_hashesToUpdate;

		//

		QString m_logStr;

		friend class AppDataServiceClient;
	};

	using AdsGatewayHandlerShared = std::shared_ptr<AdsGatewayHandler>;
}
