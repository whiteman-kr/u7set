#pragma once

#include "../GatewayLib/GatewayDescription.h"
#include "GatewayHandler.h"
#include "AppDataServiceClient.h"
#include "IvsImpulseCommThread.h"
#include "../OnlineLib/GrpcAdsClient.h"

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

	class IvsImpulseHandler : public QObject, public Handler
	{
		Q_OBJECT

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

		void updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply);
		void processGatewayAppSignalStateChanges(const Grpc::GetGatewayAppSignalStateChangesReply& reply);
		void invalidateSignals();

	signals:
		void sendGatewayStateChanges();

	private:
		bool init();
		virtual void runAppDataSrvClient() override;
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

	class IvsImpulseAppSignalStateUpdater : public IAppSignalStateUpdater
	{
	public:
		IvsImpulseAppSignalStateUpdater(IvsImpulseHandler& handler);

		virtual void adsConnected() override;
		virtual void adsDisconnected() override;

		virtual void updateAppSignalStates(const Grpc::GetAppSignalStateReply& reply) override;
		virtual void processAppSignalStateChanges(const Grpc::GetAppSignalStateChangesReply& reply) override;
		virtual void processGatewayAppSignalStateChanges(const Grpc::GetGatewayAppSignalStateChangesReply& reply) override;

	private:
		IvsImpulseHandler& m_handler;
	};

}
