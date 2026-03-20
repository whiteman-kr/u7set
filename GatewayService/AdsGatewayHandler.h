#pragma once

#include <asio/error_code.hpp>

#include <unordered_map>

#include "../GatewayLib/AdsGateway.h"

#include "GatewayHandler.h"
#include "AppDataServiceClient.h"
#include "AdsGatewayServer.h"
#include "../Metrology/GrpcAdsClient.h""

using namespace asio;

namespace Gateway
{
	class AdsGatewayHandler : public Handler, public IAppSignalStateUpdater
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

		virtual void updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply) override;
		virtual void processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply) override;
		virtual void processGatewayAppSignalStateChanges(const Grpc::GetGatewayAppSignalStateChangesReply& reply) override;

	private:
		virtual void prepareRequests() override;

		virtual void runAppDataSrvClient() override;
		virtual void stopAppDataSrvClient() override;

		void runAdsGatewayServer();
		void stopAdsGatewayServer();

	private:
		AdsGatewayShared m_gateway;

		std::mutex m_adsGatewayServerMutex;
		std::unique_ptr<AdsGatewayServer> m_adsGatewayServer;
	};

	using AdsGatewayHandlerShared = std::shared_ptr<AdsGatewayHandler>;
}
