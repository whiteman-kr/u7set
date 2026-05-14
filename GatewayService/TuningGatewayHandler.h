#pragma once

#include <asio/error_code.hpp>

#include <unordered_map>

#include "../GatewayLib/TuningGateway.h"

#include "GatewayHandler.h"
#include "TuningGatewayServer.h"

using namespace asio;

namespace Gateway
{
	class TuningGatewayHandler : public Handler
	{
	public:
		TuningGatewayHandler(const SoftwareInfo& swInfo,
						  const GatewayServiceSettings& settings,
						  TuningGatewayShared gateway,
						  const AppSignals& appSignals,
						  CircularLoggerShared log,
						  bool logGatewayPackets);

		virtual ~TuningGatewayHandler();

		virtual void run() override;
		virtual void shutdown() override;

	private:
		void runTuningGatewayServer();
		void stopTuningGatewayServer();

	private:
		TuningGatewayShared m_gateway;

		std::mutex m_tunGatewayServerMutex;
		std::unique_ptr<TuningGatewayServer> m_tunGatewayServer;

		using AsyncTuningGatewayServer = AsyncTcpServer<TuningGatewaySession>;
		
		std::unique_ptr<AsyncTuningGatewayServer> m_asyncTunGatewayServer;
	};

	using TuningGatewayHandlerShared = std::shared_ptr<TuningGatewayHandler>;
}
