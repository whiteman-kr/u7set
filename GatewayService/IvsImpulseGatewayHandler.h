#pragma once

#include "GatewayDescription.h"
#include "GatewayHandler.h"
#include "AppDataServiceClient.h"
#include "IvsImpulseCommThread.h"

namespace Gateway
{
	struct IvsImpulseListInfo
	{
		IvsImpulseSignalListShared info;
		int startIndex = -1;
		int size = -1;
	};

	class IvsImpulseHandler : public Handler
	{
	public:
		IvsImpulseHandler(const SoftwareInfo& swInfo,
						  const GatewayServiceSettings& settings,
						  IvsImpulseGatewayShared gateway,
						  const AppSignals& appSignals);

		~IvsImpulseHandler();

		virtual void run() override;
		virtual void shutdown() override;

	private:
		bool init();

	private:
		const SoftwareInfo m_softwareInfo;
		HostAddressPort m_appDataService1;
		HostAddressPort m_appDataService2;

		IvsImpulseGatewayShared m_gateway;
		const AppSignals& m_appSignals;

		AppSignalStates m_states;
		std::vector<IvsImpulseListInfo> m_lists;

		AppDataServiceClientThread* m_appDataServiceClientThread = nullptr;
		IvsImpulseCommThread* m_ivsImpulseCommThread = nullptr;

		friend class IvsImpulseCommThreadWorker;
	};

	using IvsImpulseHandlerShared = std::shared_ptr<IvsImpulseHandler>;
}
