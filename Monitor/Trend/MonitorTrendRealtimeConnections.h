#pragma once
#include "../MonitorConfigController.h"
#include "../../lib/ISignalDataServer.h"
#include "RtTrendTcpClient.h"


class MonitorTrendRealtimeConnection : public QObject
{
	Q_OBJECT
public:
	MonitorTrendRealtimeConnection() = delete;
	MonitorTrendRealtimeConnection(const MonitorTrendRealtimeConnection&) = delete;
	MonitorTrendRealtimeConnection(MonitorTrendRealtimeConnection&&) = delete;
	MonitorTrendRealtimeConnection& operator=(const MonitorTrendRealtimeConnection&) = delete;
	MonitorTrendRealtimeConnection& operator=(MonitorTrendRealtimeConnection&&) = delete;

	MonitorTrendRealtimeConnection(const SoftwareInfo& softwareInfo,
								  MonitorSettings::AppDataService server,
								  ILogFile* logFile);

	~MonitorTrendRealtimeConnection();

public:
	bool setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals);

	const MonitorSettings::AppDataService& server() const;
	RtTrendTcpClient::Stat statistics() const;

signals:
	void dataReady(QString sourceEquipmentId, std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState);
	void requestError(QString text);
	void connectionLost();

private:
	ILogFile* m_logFile = nullptr;

	MonitorSettings::AppDataService m_server;

	RtTrendTcpClient* m_rtTcpClient = nullptr;			// This object deleted by m_rtTcpClientThread
	std::unique_ptr<SimpleThread> m_rtTcpClientThread;
};


class MonitorTrendRealtimeConnections : public QObject
{
	Q_OBJECT

public:
	MonitorTrendRealtimeConnections() = delete;
	MonitorTrendRealtimeConnections(const MonitorTrendRealtimeConnections&) = delete;
	MonitorTrendRealtimeConnections(MonitorTrendRealtimeConnections&&) = delete;
	MonitorTrendRealtimeConnections& operator=(const MonitorTrendRealtimeConnections&) = delete;
	MonitorTrendRealtimeConnections& operator=(MonitorTrendRealtimeConnections&&) = delete;

	MonitorTrendRealtimeConnections(const MonitorConfigController& configController,
									const ISignalDataServer& signalDataServer,
									ILogFile* logFile);
	~MonitorTrendRealtimeConnections();

public:
	void clear();
	void createConnections();
	void updateConnections();

	[[nodiscard]] size_t size() const;

	bool setData(E::RtTrendsSamplePeriod samplePeriod, const QStringList& trendSignals);

	RtTrendTcpClient::Stat statistics() const;

signals:
	void dataReady(QString sourceEquipmentId, std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem minState, TrendLib::TrendStateItem maxState);
	void requestError(QString text);
	void connectionLost();

private:
	const MonitorConfigController& m_configController;
	const ISignalDataServer& m_signalDataServer;
	ILogFile* m_logFile = nullptr;

	// All manipulations to m_connections must be done from the main thread as it is not protected with a mutex.
	//
	std::list<MonitorTrendRealtimeConnection> m_connections;

	// Connections were created for these servers, keep this vector to detect when the servers really changed
	//
	std::vector<MonitorSettings::AppDataService> m_createdConnectionsServers;
};

