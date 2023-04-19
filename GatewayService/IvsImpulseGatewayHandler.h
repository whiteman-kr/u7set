#pragma once

#include "GatewayDescription.h"
#include "GatewayHandler.h"

namespace Gateway
{
	class IvsImpulseHandler : public Handler
	{
	public:
		IvsImpulseHandler();
		bool init(IvsImpulseGatewayShared gateway, const AppSignals& appSignals);

		virtual void run() override;
		virtual void shutdown() override;

	private:
		IvsImpulseGatewayShared m_gateway;
	};

	using IvsImpulseHandlerShared = std::shared_ptr<IvsImpulseHandler>;
}
