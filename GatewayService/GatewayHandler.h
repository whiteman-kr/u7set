#pragma once

#include "GatewayDescription.h"
#include "../AppSignalLib/AppSignal.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/CircularLogger.h"

namespace Gateway
{
	class Handler
	{
	public:
		Handler(const SoftwareInfo& swInfo,
				const GatewayServiceSettings& settings,
				CircularLoggerShared log,
				bool logGatewayPackets);

		virtual void run() = 0;
		virtual void shutdown() = 0;

	protected:
		const SoftwareInfo& m_swInfo;
		const GatewayServiceSettings& m_settings;
		CircularLoggerShared m_log;

		bool m_logGatewayPackets = false;
	};

	using HandlerShared = std::shared_ptr<Handler>;

	class Handlers
	{
	public:
		Handlers();

		bool init(const Gateways& gateways,
				  const SoftwareInfo& swInfo,
				  const GatewayServiceSettings& settings,
				  const AppSignals& appSignals,
				  CircularLoggerShared log,
				  bool logGatewayPackets);
		void run();
		void shutdown();

		void clear();

	private:
		std::vector<HandlerShared> m_handlers;
	};
}
