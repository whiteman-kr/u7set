#pragma once
#include <map>
#include "./Trend/RtTrendTcpClient.h"
#include "../MonitorConfigController.h"

//
// RtSignal: Data for RealTimeTrends on schemas, SchemaItemIndicator, type = Trend
//
class RtSignal
{
public:
	RtSignal(const QString& appSignalId) :
		m_archive(appSignalId, "")		// Realtime trend does not care about archive server, it ignores it
	{
	}
	RtSignal(const RtSignal& src) = default;

	const QString& appSignalId() const
	{
		return m_archive.trendSignalPlusServerId.archiveServerId;
	}

	const TrendLib::TrendArchive& archive() const
	{
		return m_archive;
	}

	TrendLib::TrendArchive& archive()
	{
		return m_archive;
	}

private:
	TrendLib::TrendArchive m_archive;
};

//
// RtConnection
//
class RtConnection : public QObject
{
	Q_OBJECT

public:
	RtConnection() = default;
	~RtConnection();

	bool trendData(const QString& appSignalId, std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const;
	TimeStamp maxTimeStamp() const;

	bool hasConnectionThread() const;
	void createConnectionThread(MonitorConfigController* configController);
	
	void setParams(E::RtTrendsSamplePeriod samplePeriod, E::TimeType timeType, int durationSeconds);
	void updateSignals(const QStringList appSignalIds);

private slots:
	void slot_realtimeDataReceived(QString sourceEquipmentId,
								   std::shared_ptr<TrendLib::RealtimeData> data,
								   TrendLib::TrendStateItem /*minState*/,
								   TrendLib::TrendStateItem /*maxState*/);
	void slot_connectionLost();

private:
	void appendRealtimeData(QString sourceEquipmentId, Hash signalHash, const std::vector<TrendLib::TrendStateItem>& states);
	void appendRealtimeData_unsafe(E::TimeType timeType, const std::vector<TrendLib::TrendStateItem>& states, TrendLib::TrendArchive* archive);
	void trimArchive_unsafe(int durationSeconds, TrendLib::TrendArchive* archive);

private:
	mutable QMutex m_mutex;

	RtTrendTcpClient* m_tcpClient = nullptr;
	SimpleThread* m_tcpClientThread = nullptr;

	int m_durationSeconds = 0;
	E::RtTrendsSamplePeriod m_samplePeriod = E::RtTrendsSamplePeriod::sp_1s;
	E::TimeType m_timeType = E::TimeType::Local;
	std::map<Hash, RtSignal> m_trendSignals;		// Key is hash from appSignalId

	TrendLib::TrendStateItem m_minState{};
	TrendLib::TrendStateItem m_maxState{};
};

//
// RealTime trend class for SchemaItemIndicator, type Trend
//
class RtTrendSchema : public QObject
{
	Q_OBJECT

public:
	RtTrendSchema();
	RtTrendSchema(MonitorConfigController* configController);
	virtual ~RtTrendSchema();

public:
	void updateRealtimeConnections();		// Updates m_rtConnections from SchemaDetaisSet

	bool trendData(QUuid trendUuid,
				   const QString& appSignalId,
				   std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const;

	TimeStamp maxTimeStamp(QUuid trendUuid) const;

private:
	MonitorConfigController* const m_configController = nullptr;

	mutable QMutex m_mutex;
	std::map<QUuid, RtConnection> m_rtConnections;	// Key is SchemaItem uuid, receiver of data
};

