#pragma once

#include "../lib/ISignalDataServer.h"
#include "../OnlineLib/Tcp.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "../TrendView/TrendSignalState.h"

namespace ClientLib
{
	//     onConnection()
	//            |
	//	 startRequestCycle()    <-------------------------------------------+
	//	          |															|
	//   requestTrendManagement() - RT_TRENDS_MANAGEMENT					|
	//   processTrendManagement() - RT_TRENDS_MANAGEMENT					|
	//            |															|
	//   requestTrendStateChanges() - RT_TRENDS_GET_STATE_CHANGES			|
	//   processTrendStateChanges() - RT_TRENDS_GET_STATE_CHANGES			|
	//            |															|
	//	 emit dataReady(...)												|
	//            |															|
	//            +---------------------------------------------------------+
	//
	class RtTrendTcpClient : public Tcp::Client, public TcpClientStatistics
	{
		Q_OBJECT

	public:
		RtTrendTcpClient(const SoftwareInfo& softwareInfo,
						 const HostAddressPort& serverAddressPort,
						 QString serviceEquipmentId,
						 const ISignalDataServer& signalDataServer,
						 ILogFile* logFile);
		virtual ~RtTrendTcpClient();

		// Methods
		//
	public:
		bool setSignals(const QStringList& appSignalIds);
		bool setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals);

		void setSamplePeriod(E::RtTrendsSamplePeriod samplePeriod);
		E::RtTrendsSamplePeriod samplePeriod() const;

	protected:
		virtual void onClientThreadStarted() override;
		virtual void onClientThreadFinished() override;

		virtual void onConnection() override;
		virtual void onDisconnection() override;
		virtual void onReplyTimeout() override;

		virtual void processReply(quint32 requestID, const char* replyData, quint32 replyDataSize) override;

	protected:
		void startRequestCycle();

		void requestTrendManagement();
		void processTrendManagement(const QByteArray& data);

		void requestTrendStateChanges();
		void processTrendStateChanges(const QByteArray& data);

	signals:
		void dataReady(QString sourceEquipmentId,
					   std::shared_ptr<TrendLib::RealtimeData> data,
					   TrendLib::TrendStateItem minState,
					   TrendLib::TrendStateItem maxState);
		void requestError(QString text);
		void connectionLost(QString sourceEquipmentId);

		// Staticstic
		//
	public:
		struct Stat
		{
			QString text;
			int requestQueueSize = 0;
			int requestCount = 0;
			int replyCount = 0;
			int isConnected = 0;		// It must be int for summing up statistics for several connections.
		};

		Stat stat() const;
		void setStat(const Stat& stat);

		void setStatText(const QString& text);
		void setStatRequestQueueSize(int value);

		void incStatRequestCount();
		void incStatReplyCount();

		// Data
		//
	private:
		const ISignalDataServer& m_signalDataServer;
		HasLogFile m_logFile;

		mutable QMutex m_dataMutex;

		E::RtTrendsSamplePeriod m_samplePeriod = E::RtTrendsSamplePeriod::sp_1s;
		std::set<QString> m_signalSet;

	private:
		std::set<Hash> m_trackedSignals;		// Currently tracked signals by AppDataService

		// Statisctics and state variables
		//
		mutable QMutex m_statMutex;
		Stat m_stat;
	};
}
