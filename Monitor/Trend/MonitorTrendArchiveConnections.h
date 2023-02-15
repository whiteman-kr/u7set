#pragma once
#include "../MonitorConfigController.h"
#include "ArchiveTrendTcpClient.h"


class MonitorTrendArchiveConnection : public QObject
{
	Q_OBJECT

public:
	MonitorTrendArchiveConnection() = delete;
	MonitorTrendArchiveConnection(const MonitorTrendArchiveConnection&) = delete;
	MonitorTrendArchiveConnection(MonitorTrendArchiveConnection&&) = delete;
	MonitorTrendArchiveConnection& operator=(const MonitorTrendArchiveConnection&) = delete;
	MonitorTrendArchiveConnection& operator=(MonitorTrendArchiveConnection&&) = delete;

	MonitorTrendArchiveConnection(const SoftwareInfo& softwareInfo,
								  MonitorSettings::ArchiveService server,
								  ILogFile* logFile);

	~MonitorTrendArchiveConnection();

public:
	void requestData(TrendLib::TrendSignalPlusServerId signalPlusServerId,
					 TimeStamp hourToRequest,
					 E::TimeType timeType);

	const TrendLib::ArchiveServer& archiveServer() const;
	ArchiveTrendTcpClient::Stat statistics() const;

signals:
	void dataReady(TrendLib::TrendSignalPlusServerId trendSignalPlusServerId, TimeStamp requestedHour, E::TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data);
	void requestError(TrendLib::TrendSignalPlusServerId trendSignalPlusServerId, TimeStamp requestedHour, E::TimeType timeType);

	void private_requestData(TrendLib::TrendSignalPlusServerId signalPlusServerId,
							 TimeStamp hourToRequest,
							 E::TimeType timeType);

private:
	ILogFile* m_logFile = nullptr;

	TrendLib::ArchiveServer m_archiveServer;

	ArchiveTrendTcpClient* m_archiveTcpClient = nullptr;	// This object deleted by m_archiveTcpClientThread
	std::unique_ptr<SimpleThread> m_archiveTcpClientThread;
};


class MonitorTrendArchiveConnections : public QObject
{
	Q_OBJECT

public:
	MonitorTrendArchiveConnections() = delete;
	MonitorTrendArchiveConnections(const MonitorTrendArchiveConnections&) = delete;
	MonitorTrendArchiveConnections(MonitorTrendArchiveConnections&&) = delete;
	MonitorTrendArchiveConnections& operator=(const MonitorTrendArchiveConnections&) = delete;
	MonitorTrendArchiveConnections& operator=(MonitorTrendArchiveConnections&&) = delete;

	MonitorTrendArchiveConnections(const MonitorConfigController& configController, ILogFile* logFile);
	~MonitorTrendArchiveConnections();

public:
	void clear();
	void createConnections();
	void updateConnections();

	size_t size() const;

	void requestData(TrendLib::TrendSignalPlusServerId signalPlusServerId,
					 TimeStamp hourToRequest,
					 E::TimeType timeType);

	ArchiveTrendTcpClient::Stat statistics() const;

signals:
	void dataReady(TrendLib::TrendSignalPlusServerId trendSignalPlusServerId, TimeStamp requestedHour, E::TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data);
	void requestError(TrendLib::TrendSignalPlusServerId trendSignalPlusServerId, TimeStamp requestedHour, E::TimeType timeType);

private:
	const MonitorConfigController& m_configController;
	ILogFile* m_logFile = nullptr;

	// All manipulations to m_connections must be done from the main thread as it is not protected with a mutex.
	//
	std::list<MonitorTrendArchiveConnection> m_connections;

	// Connections were created for these servers, keep this vector to detect when the servers really changed
	//
	std::vector<MonitorSettings::ArchiveService> m_createdConnectionsServers;
};

