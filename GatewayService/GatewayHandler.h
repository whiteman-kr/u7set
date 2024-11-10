#pragma once

#include "GatewayDescription.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "../OnlineLib/CircularLogger.h"

namespace Gateway
{
	class Handler
	{
	private:
		static const qint64 GW_LOG_PERIOD_SECS = 60 * 60;		// 1 hour

	public:
		Handler(const QString& gatewayID, const SoftwareInfo& swInfo,
				const GatewayServiceSettings& settings,
				CircularLoggerShared log, bool logGatewayPackets);
		virtual ~Handler();

		virtual void run() = 0;
		virtual void shutdown();

		virtual void getRequiredSignalsHashes(std::set<Hash>* hashes) const;
		virtual void getEventSignalsHashes(std::set<Hash>* hashes) const;

		virtual void updateSignalStates(const Network::GetAppSignalStateReply& getStatesReply);
		virtual void processStateChanges(const Network::GatewayGetAppSignalStateChangesReply& getStateChangesReply);

		CircularLoggerShared log();

		bool enableLogging() const;
		void logRequest(const QString& msg, CircularLogger::RecordType recType = CircularLogger::RecordType::Message);
		void logReply(const QString& msg, CircularLogger::RecordType recType = CircularLogger::RecordType::Message);

	private:
		void writeToGwLog(const QString& msg, CircularLogger::RecordType recType);

	protected:
		QString m_gatewayID;
		const SoftwareInfo& m_swInfo;
		const GatewayServiceSettings& m_settings;
		CircularLoggerShared m_log;

		bool m_logGatewayPackets = false;
		qint64 m_logStartTimeSecs = 0;
		CircularLoggerShared m_gwLog = nullptr;				// log of gateway request/reply packets
		bool m_lastMsgIsRequest = true;

		bool m_shutwownCalled = false;
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
