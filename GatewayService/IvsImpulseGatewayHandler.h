#pragma once

#include "../GatewayLib/GatewayDescription.h"
#include "GatewayHandler.h"
#include "AppDataServiceClient.h"
#include "IvsImpulseCommThread.h"

namespace Gateway
{
	class IvsImpulseListInfo
	{
	public:
		IvsImpulseSignalListShared info;

		int startIndex = -1;
		int size = -1;

		quint16 eventsPacketNo = 0;

		std::map<Hash, std::vector<int>> hashToListIndexes;		// Hash(appSignalID) => indexes in signal list
																// vector is required if same signal repeated in list several times

		SpinLock stateChangesMutex;

		std::vector<GatewayAppSignalState> stateChangesToRead;
		std::vector<GatewayAppSignalState> stateChangesToWrite;

		bool hasStateChanges()
		{
			SpinLockGuard ml(stateChangesMutex);
			return !stateChangesToRead.empty();
		}
	};

	using IvsImpulseListInfoShared = std::shared_ptr<IvsImpulseListInfo>;

	class IvsImpulseHandler : public Handler
	{
	public:
		IvsImpulseHandler(const SoftwareInfo& swInfo,
						  const GatewayServiceSettings& settings,
						  IvsImpulseGatewayShared gateway,
						  const AppSignals& appSignals,
						  CircularLoggerShared log,
						  bool logGatewayPackets);

		virtual ~IvsImpulseHandler();

		virtual void run() override;
		virtual void shutdown() override;

		virtual void onAppDataSrvConnected() override;
		virtual void onAppDataSrvDisconnected() override;
		virtual void planNextPreparedRequest(PreparedRequest& request) override;

		virtual void updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply) override;
		virtual void processGatewayStateChanges(const Network::GetGatewayAppSignalStateChangesReply& getStateChangesReply) override;

	private:
		bool init();
		virtual void prepareRequests() override;

	private:
		IvsImpulseGatewayShared m_gateway;

		AppSignalStates m_states;
		std::map<Hash, std::vector<int>> m_hashToStatesIndexes;
		std::atomic_bool m_signalStatesUpdated = { false };

		std::vector<IvsImpulseListInfoShared> m_lists;

		// signal hash => lists where this signal live
		//
		std::map<Hash, std::set<IvsImpulseListInfoShared>> m_hashToLists;

		IvsImpulseCommThread* m_ivsImpulseCommThread = nullptr;

		friend class IvsImpulseCommThreadWorker;
		friend class AppDataServiceClient;
	};

	using IvsImpulseHandlerShared = std::shared_ptr<IvsImpulseHandler>;
}
