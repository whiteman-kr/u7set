#pragma once

#include "../OnlineLib/Tcp.h"
#include "../lib/Hash.h"

#include "../Proto/network.pb.h"

#include "AppDataService.h"
#include "AppDataSource.h"

namespace RtTrends
{

	class SignalStatesQueue
	{
	public:
		SignalStatesQueue(Hash signalHash, int queueSize);

		Hash signalHash() const { return m_signalHash; }
		void push(const SimpleAppSignalState& state, const QThread* thread);

		FastThreadSafeQueue<SimpleAppSignalState>& clientQueue() { return m_clientQueue; }

	private:
		Hash m_signalHash = 0;
		FastThreadSafeQueue<SimpleAppSignalState> m_clientQueue;
	};

	class Session
	{
	public:
		Session(AppDataServiceWorker& service);
		~Session();

		int id() const { return m_id; }

		E::RtTrendsSamplePeriod samplePeriod() const { return m_samplePeriod; }
		void setSamplePeriod(E::RtTrendsSamplePeriod sp) { m_samplePeriod = sp; }

		bool containsSignal(Hash signalHash);
		bool appendSignal(Hash signalHash);
		bool deleteSignal(Hash signalHash);

		void pushSignalState(Hash signalHash, const SimpleAppSignalState& state, const QThread* thread);

		void getTrackedSignalHashes(QVector<Hash>* hashes);

		const QHash<Hash, SignalStatesQueue*>& trackedSignals() const { return m_trackedSignals; }

	private:
		static std::atomic<int> m_globalID;

		//

		int m_id = 0;

		DynamicAppSignalStates& m_signalStates;
		const SignalsToSources& m_signalToSources;

		//

		E::RtTrendsSamplePeriod m_samplePeriod = E::RtTrendsSamplePeriod::sp_1s;
		int m_samplePeriodCounter = 0;

		QMultiHash<Hash, SignalStatesQueue*> m_trackedSignals;
	};

	typedef std::shared_ptr<Session> SessionShared;

	class Server : public Tcp::Server
	{
	public:
		Server(AppDataServiceWorker& appDataService);

		Tcp::Server* getNewInstance() override;

		void onServerThreadStarted() override;
		void onServerThreadFinished() override;

		void onConnectedSoftwareInfoChanged() override;

		void processRequest(quint32 requestID, const char* requestData, quint32 requestDataSize) override;

	private:
		void onRtTrendsManagementRequest(const char* requestData, quint32 requestDataSize);

		void setSamplePeriod(E::RtTrendsSamplePeriod newSamplePeriod);

		void appendTrackedSignals(const Network::RtTrendsManagementRequest& request);
		bool appendTrackedSignal(Hash signalHash, E::RtTrendsSamplePeriod samplePeriod);

		void removeTrackedSignals(const Network::RtTrendsManagementRequest& request);
		bool removeTrackedSignal(Hash signalHash);

		void onRtTrendsGetStateChangesRequest(const char* requestData, quint32 requestDataSize);

	private:
		AppDataServiceWorker& m_appDataService;
		const SignalsToSources& m_signalsToSources;
		DynamicAppSignalStates& m_signalStates;
		std::shared_ptr<CircularLogger> m_log;

		//

		std::shared_ptr<Session> m_session;

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
		ServerThread(const HostAddressPort& listenAddressPort,
					 AppDataServiceWorker& appDataService);
	};

}
