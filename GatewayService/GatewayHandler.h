#pragma once

#include "../GatewayLib/GatewayDescription.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/CircularLogger.h"
#include "../OnlineLib/GrpcAdsClient.h"

namespace Gateway
{
	class AppDataServiceClientThread;
	class AppDataServiceClient;

	struct PreparedRequest
	{
		uint32_t ID;
		std::vector<char> data;
		int delayMs = 0;		// 0 - send now

		void clear();
		void setRequest(const PreparedRequest& rq, int delay);
		void setDelay(int delay);
		bool hasRequest() const;
	};

	class Handler
	{
	private:
		static constexpr qint64 GW_LOG_PERIOD_SECS = 60 * 60;		// 1 hour

	public:
		Handler(const QString& gatewayID, const SoftwareInfo& swInfo,
				const GatewayServiceSettings& settings,
				const AppSignals& appSignals,
				CircularLoggerShared log, bool logGatewayPackets);
		virtual ~Handler();

		virtual void run() = 0;
		virtual void shutdown();

		virtual void runAppDataSrvClient();
		virtual void stopAppDataSrvClient();

		virtual void onAppDataSrvConnected();
		virtual void onAppDataSrvDisconnected();
		virtual void planNextPreparedRequest(PreparedRequest& rqPlan);
		virtual void onAppDataRequestSent(quint32 requestID, qint64 nowMs);

		virtual void getRequiredSignalsHashes(std::set<Hash>* hashes) const;
		virtual void getEventSignalsHashes(std::set<Hash>* hashes) const;

		// virtual void updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply);
		// virtual void processStateChanges(const Network::GetAppSignalStateChangesReply& getStateChangesReply);
		// virtual void processGatewayStateChanges(const Network::GetGatewayAppSignalStateChangesReply& getGatewayStateChangesReply);

		CircularLoggerShared log();

		QString gatewayID() const;

		bool enableLogging() const;
		void logRequest(const QString& msg, CircularLogger::RecordType recType = CircularLogger::RecordType::Message);
		void logReply(const QString& msg, CircularLogger::RecordType recType = CircularLogger::RecordType::Message);

	protected:
		virtual void prepareRequests();

	private:
		void writeToGwLog(const QString& msg, CircularLogger::RecordType recType);
		void closeGwLog();

	protected:
		QString m_gatewayID;
		const SoftwareInfo m_swInfo;
		const GatewayServiceSettings m_settings;
		const AppSignals& m_appSignals;
		CircularLoggerShared m_log;

		bool m_logGatewayPackets = false;
		qint64 m_logStartTimeSecs = 0;
		CircularLoggerShared m_gwLog = nullptr;				// log of gateway request/reply packets
		bool m_lastMsgIsRequest = true;

		std::vector<PreparedRequest> m_requests;
		size_t m_requestIndex = 0;

		// request planning
		//
		std::optional<size_t> m_changesRequestIndex;
		bool m_hasPendingChanges = false;
		int m_changesRequestCount = 0;

		bool m_shutdownCalled = false;

		std::mutex m_adsClientMutex;
		std::unique_ptr<GrpcAdsClient> m_adsClient;
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
				  QString logGatewayIDs);			// copy Ok
		void run();
		void shutdown();

		void clear();

	private:
		std::vector<HandlerShared> m_handlers;
	};
}
