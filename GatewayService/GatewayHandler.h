#pragma once

#include "GatewayDescription.h"
#include "../AppSignalLib/AppSignal.h"

namespace Gateway
{
	class Handler
	{
	public:
		Handler();

		virtual void run() = 0;
		virtual void shutdown() = 0;
	};

	using HandlerShared = std::shared_ptr<Handler>;

	class Handlers
	{
	public:
		Handlers();

		bool init(const Gateways& gateways, const AppSignals& appSignals);
		void run();
		void shutdown();

		void clear();

	private:
		std::vector<HandlerShared> m_handlers;
	};
}
