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

		virtual void onAppDataSrvConnected() override;
		virtual void onAppDataSrvDisconnected() override;
		virtual void planNextPreparedRequest(PreparedRequest& request) override;

		virtual void getRequiredSignalsHashes(std::set<Hash>* hashes) const override;
		virtual void getEventSignalsHashes(std::set<Hash>* hashes) const override;

		void updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply);
		void processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply);
		void invalidateSignals();

	private:
		virtual void prepareRequests() override;

		virtual void runAppDataSrvClient() override;

		void runAdsGatewayServer();
		void stopAdsGatewayServer();

	private:
		AdsGatewayShared m_gateway;

		std::mutex m_adsGatewayServerMutex;

		using AsyncAdsGatewayServer = AsyncTcpServer<TuningGatewaySession>;

		std::unique_ptr<AsyncAdsGatewayServer> m_asyncAdsGatewayServer;

	};

	using AdsGatewayHandlerShared = std::shared_ptr<AdsGatewayHandler>;

	class AdsGatewayAppSignalStateUpdater : public IAppSignalStateUpdater
	{
	public:
		AdsGatewayAppSignalStateUpdater(AdsGatewayHandler& handler);

		virtual void adsConnected() override;
		virtual void adsDisconnected() override;

		virtual void updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply) override;
		virtual void processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply) override;
		virtual void processGatewayAppSignalStateChanges(const Grpc::GetGatewayAppSignalStateChangesReply& reply) override;

	private:
		AdsGatewayHandler& m_handler;
	};
}
