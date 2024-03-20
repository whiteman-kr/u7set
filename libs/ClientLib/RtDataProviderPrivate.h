#pragma once
#include "../OnlineLib/SoftwareSettings.h"
#include "../lib/ISignalDataServer.h"
#include "./include/ClientLib/RtTrendConnectionStatistics.h"

#include "RtTrendTcpClient.h"

namespace ClientLib
{
	//
	// Single connection to AppDataService for providing real time data for trends
	//
	class RtConnection : public QObject
	{
		Q_OBJECT

	public:
		RtConnection() = delete;
		RtConnection(const RtConnection&) = delete;
		RtConnection(RtConnection&&) = delete;
		RtConnection& operator=(const RtConnection&) = delete;
		RtConnection& operator=(RtConnection&&) = delete;

		RtConnection(const SoftwareInfo& softwareInfo,
					 SoftwareEndpoint::AppDataService server,
					 const ISignalDataServer& signalDataServer,
					 ILogFile* logFile);

		~RtConnection();

	public:
		bool setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals);
		void setSamplePeriod(E::RtTrendsSamplePeriod samplePeriod);

		const SoftwareEndpoint::AppDataService& server() const;
		RtTrendConnectionStatistics statistics() const;

	signals:
		void dataReady(QString sourceEquipmentId, std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState);
		void requestError(QString text);
		void connectionLost(QString sourceEquipmentId);

	private:
		ILogFile* m_logFile = nullptr;

		SoftwareEndpoint::AppDataService m_server;

		RtTrendTcpClient* m_rtTcpClient = nullptr; // This object deleted by m_rtTcpClientThread
		std::unique_ptr<SimpleThread> m_rtTcpClientThread;
	};


	//
	// Real time trends data provdider - connects to all real time sources (app data service)
	//
	class RtDataProviderPrivate : public QObject
	{
		Q_OBJECT

	public:
		RtDataProviderPrivate() = delete;
		RtDataProviderPrivate(const RtDataProviderPrivate&) = delete;
		RtDataProviderPrivate(RtDataProviderPrivate&&) = delete;
		RtDataProviderPrivate& operator=(const RtDataProviderPrivate&) = delete;
		RtDataProviderPrivate& operator=(RtDataProviderPrivate&&) = delete;

		RtDataProviderPrivate(const ISignalDataServer& signalDataServer, ILogFile* logFile);
		~RtDataProviderPrivate();

	public:
		void clear();
		void createConnections(const SoftwareInfo& softwareInfo,
							   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices);
		void updateConnections(const SoftwareInfo& softwareInfo,
							   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices);

		bool setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals);
		void setSamplePeriod(E::RtTrendsSamplePeriod samplePeriod);

		[[nodiscard]] size_t size() const;
		[[nodiscard]] RtTrendConnectionStatistics statistics() const;

		[[nodiscard]] bool allConnected(std::chrono::milliseconds timeout) const;

	signals:
		void dataReady(QString sourceEquipmentId, std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState);
		void requestError(QString text);
		void connectionLost(QString sourceEquipmentId);

	private:
		const ISignalDataServer& m_signalDataServer;
		ILogFile* m_logFile = nullptr;

		// All manipulations to m_connections must be done from the main thread as it is not protected with a mutex.
		//
		std::list<RtConnection> m_connections;

		// Connections were created for these servers, keep this vector to detect when the servers really changed
		//
		std::vector<SoftwareEndpoint::AppDataService> m_createdConnectionsServers;
	};
} // namespace ClientLib
