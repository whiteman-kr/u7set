#pragma once

#include <asio/error_code.hpp>

#include <unordered_map>

#include "../GatewayLib/AdsGateway.h"

#include "GatewayHandler.h"
#include "AppDataServiceClient.h"
#include "AdsGatewayServer.h"
#include "../OnlineLib/GrpcAdsClient.h"

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

	private:
		void runAdsGatewayServer();
		void stopAdsGatewayServer();

	private:
		AdsGatewayShared m_gateway;

		std::mutex m_adsGatewayServerMutex;

		using AsyncAdsGatewayServer = AsyncTcpServer<AdsGatewaySession>;
		std::unique_ptr<AsyncAdsGatewayServer> m_asyncAdsGatewayServer;
	};

	using AdsGatewayHandlerShared = std::shared_ptr<AdsGatewayHandler>;
}
