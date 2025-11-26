#pragma once

#include "../OnlineLib/SoftwareEndpoint.h"
#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"

#include <AdsConnectionLib/IAppSignalUpdater.h>


namespace ClientLib
{
	class ISignalLogUpdater;

	inline const int RequestTimeIntervalMs = 20;

	// Number of signals requested by ADS_GET_APP_SIGNAL_STATE.
	// Assume RequestTimeIntervalMs is 20ms, then we have about 50 requests per second,
	// for 100K signals with 250 signals per request, full update will take approximately 8 seconds.
	// 250 signals * (1'000ms / RequestTimeIntervalMs) rps = 12500 signals per seconds.
	// 100'000 / 12500 = 8 signals per second.
	//
	inline const int MaxStateRequestCount = ADS_GET_APP_SIGNAL_STATE_MAX / 8; // 250 signals per ADS_GET_APP_SIGNAL_STATE

	// clang-format off
	// 
	// Workflow of the TcpSignalClient:
	//
	//                 ADS_GET_APP_SIGNAL_LIST_START
	//                         |
	//                 ADS_GET_APP_SIGNAL_LIST_NEXT
	//                         |
	//                 ADS_GET_APP_SIGNAL_PARAM
	//                         |
	//                 ADS_GET_APP_SIGNAL_STATE_CHANGES <----+
	//                         |                             |
	//          pending states ?---------------------------->|
	//                         |                             |
	//                 ADS_GET_APP_SIGNAL_STATE              |    Request MaxStateRequestCount (250) signals.
	//                         |                             |
	//    discrete log allowed ?---------------------------->|
	//                         |                             |
	//                 ADS_GET_DISCRETES_LOG                 |
	//                         |                             |
	//                         +-----------------------------+
	//
	// clang-format on


	class TcpSignalClient : public Tcp::Client,
							public TcpClientStatistics,
							public HasLogFile
	{
		Q_OBJECT

	public:
		TcpSignalClient(const SoftwareInfo& softwareInfo,
						const SoftwareEndpoint::AppDataService& adsInfo,
						IAppSignalUpdater& signalUpdater,
						ISignalLogUpdater* signalLogUpdater,
						ILogFile* logFile);
		virtual ~TcpSignalClient();

	public:
		virtual void onClientThreadStarted() override;
		virtual void onClientThreadFinished() override;

		virtual void onConnection() override;
		virtual void onDisconnection() override;

		virtual void onReplyTimeout() override;

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	protected:
		void resetToGetSignalList();
		void resetToGetState(bool resetStateIndex);

		void requestSignalListStart();
		void processSignalListStart(const QByteArray& data);

		void requestSignalListNext(int part);
		void processSignalListNext(const QByteArray& data);

		void requestSignalParam(int startIndex);
		void processSignalParam(const QByteArray& data);

		void requestSignalStateChanges();
		void processSignalStateChanges(const QByteArray& data);

		void requestSignalState(int startIndex);
		void processSignalState(const QByteArray& data);

		void requestSignalLog();
		void processSignalLog(const QByteArray& data);

		void requestAckSignalLog();
		void processAckSignalLog(const QByteArray& data);

	public:
		bool signalParamsLoaded() const;
		bool signalStatesLoaded() const;

		const SoftwareEndpoint::AppDataService& server() const;

	private:
		void reset();
		void checkTimeDiscrepancy(qint64 serverUtcTimeMs, qint64 serverLocalTimeMs);

	private:
		SoftwareEndpoint::AppDataService m_serverSettings;
		IAppSignalUpdater& m_signalUpdater;

		ClientLib::IAppSignalUpdater::SourceIdType sourceId() const;

	private:
		std::atomic<bool> m_signalParamsLoaded{false};
		std::atomic<bool> m_signalStatesLoaded{false};

		ISignalLogUpdater* m_signalLogUpdater = nullptr;

		// Cache protobuf messages
		//
		std::vector<Hash> m_signalList;
		std::set<Hash> m_busSignalHashes; // Bus signal hash set. These hashes are later removed from m_signalList
		std::set<Hash> m_signalStatesSet; // Signal hash is added here when signal state is received

		int m_lastSignalParamStartIndex = 0;

		int m_lastSignalStateStartIndex = 0;

		// Check that the server and client time is the same.
		//
		QDate m_timeDiscrepancyCheckDate; // When was the last time the time discrepancy was checked?
	};

} // namespace ClientLib
