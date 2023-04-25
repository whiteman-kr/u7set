#pragma once

#include "GatewayDescription.h"
#include "GatewayHandler.h"
#include "AppDataServiceClient.h"

namespace Gateway
{
	class IvsImpulseHandler : public Handler
	{
	public:
		IvsImpulseHandler(const SoftwareInfo& swInfo,
						  const GatewayServiceSettings& settings,
						  IvsImpulseGatewayShared gateway,
						  const AppSignals& appSignals);
		bool init();

		virtual void run() override;
		virtual void shutdown() override;

	private:
		IvsImpulseGatewayShared m_gateway;
		const AppSignals& m_appSignals;

		AppSignalStates m_states;

		AppDataServiceClient* m_appDataServiceClient = nullptr;
	};

	using IvsImpulseHandlerShared = std::shared_ptr<IvsImpulseHandler>;
}
