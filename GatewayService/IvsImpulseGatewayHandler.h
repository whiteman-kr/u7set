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

		quint16 iventsPacketNo = 0;

		std::vector<GatewayAppSignalState> stateChanges;
	};

	using IvsImpulseListInfoShared = std::shared_ptr<IvsImpulseListInfo>;

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
		std::atomic_bool m_signalStatesUpdated = { false };

		std::vector<IvsImpulseListInfoShared> m_lists;

		// signal hash => lists where this signal live
		//
		std::map<Hash, std::vector<IvsImpulseListInfoShared>> m_hashToLists;

		AppDataServiceClientThread* m_appDataServiceClientThread = nullptr;
		IvsImpulseCommThread* m_ivsImpulseCommThread = nullptr;

		friend class IvsImpulseCommThreadWorker;
	};

	using IvsImpulseHandlerShared = std::shared_ptr<IvsImpulseHandler>;
}
