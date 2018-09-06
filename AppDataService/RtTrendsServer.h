#pragma once

#include "../lib/Tcp.h"
#include "../lib/Hash.h"

#include "../Proto/network.pb.h"

#include "AppDataSource.h"

namespace RtTrends
{

	struct SignalState
	{
		SimpleAppSignalState state;

		qint64 archiveID = 0;
		quint32 samplePeriodFlags = 0;
	};

	class SignalStatesQueue
	{
	private:
		LockFreeQueue<SignalState> m_clientQueue;
		LockFreeQueue<SignalState> m_dbQueue;
	};

	class Session
	{
	public:
		Session(int id);

		int id() const { return m_id; }

		E::RtTrendsSamplePeriod samplePeriod() const { return m_samplePeriod; }
		void setSamplePeriod(E::RtTrendsSamplePeriod sp) { m_samplePeriod = sp; }

	private:
		int m_id = 0;
		E::RtTrendsSamplePeriod m_samplePeriod = E::RtTrendsSamplePeriod::sp_60s;
		QHash<Hash, SignalStatesQueue*> m_trackedSignals;

		std::atomic<qint64> m_archiveID = 0;
	};

	typedef std::shared_ptr<Session> SessionShared;

	class Server : public Tcp::Server
	{
	public:
		Server(const SoftwareInfo& sotwareInfo,
			   const SignalsToSources& signalsToSources,
			   std::shared_ptr<CircularLogger> logger);

		Tcp::Server* getNewInstance() override;

		void onServerThreadStarted() override;
		void onServerThreadFinished() override;

		void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

	private:
		void onRtTrendsManagementRequest(const char* requestData, quint32 requestDataSize);
		void setSamplePeriod(E::RtTrendsSamplePeriod newSamplePeriod);
		void appendTrackedSignals(const Network::RtTrendsManagementRequest& request);
		void deleteTrackedSignals(const Network::RtTrendsManagementRequest& request);

		void onRtTrendsGetStateChangesRequest(const char* requestData, quint32 requestDataSize);


	private:
		const SignalsToSources& m_signalsToSources;
		std::shared_ptr<CircularLogger> m_log;

		static std::atomic<int> m_globalSessionID;

		//

		Session m_session;
		AppDataSourcesIP m_trackedSources;

		//

		Network::RtTrendsManagementRequest m_rtTrendsManagementRequest;
		Network::RtTrendsManagementReply m_rtTrendsManagementReply;

		Network::RtTrendsGetStateChangesRequest m_rtTrendsGetStateChangesRequest;
		Network::RtTrendsGetStateChangesReply m_rtTrendsGetStateChangesReply;
	};

	//

	class ServerThread : public Tcp::ServerThread
	{
	public:
		ServerThread(const SoftwareInfo& sotwareInfo,
					 const HostAddressPort& listenAddressPort,
					 const SignalsToSources& signalsToSources,
					 std::shared_ptr<CircularLogger> logger);
	};

}
