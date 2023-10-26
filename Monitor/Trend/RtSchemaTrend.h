#pragma once
#include <map>
#include "../MonitorConfigController.h"
#include "../ClientLib/RtDataProvider.h"
#include "../../TrendView/TrendSignalSet.h"

//
// RtSignal: Data for RealTimeTrends on schemas, SchemaItemIndicator, type = Trend
//
class RtSchemaTrendSignal
{
public:
	RtSchemaTrendSignal(const QString& appSignalId) :
		m_archive(appSignalId, "")		// Realtime trend does not care about archive server, it ignores it
	{
	}
	RtSchemaTrendSignal(const RtSchemaTrendSignal& src) = default;

	[[nodiscard]] const QString& appSignalId() const
	{
		return m_archive.trendSignalPlusServerId.appSignalId;
	}

	[[nodiscard]] const TrendLib::TrendArchive& archive() const
	{
		return m_archive;
	}

	[[nodiscard]] TrendLib::TrendArchive& archive()
	{
		return m_archive;
	}

private:
	TrendLib::TrendArchive m_archive;
};


//
// RtSchemaTrendDataProvider
//
class RtSchemaTrendDataProvider : public QObject
{
	Q_OBJECT

public:
	RtSchemaTrendDataProvider(const ISignalDataServer& signalDataServer, ILogFile* logFile);
	~RtSchemaTrendDataProvider();

public:
	void updateConnections(const SoftwareInfo& softwareInfo,
						   const std::vector<SoftwareEndpoint::AppDataService>& appDataServices);

	bool trendData(const QString& appSignalId, std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const;
	TimeStamp maxTimeStamp() const;

	
	void setParams(E::RtTrendsSamplePeriod samplePeriod, E::TimeType timeType, int durationSeconds);
	void updateSignals(const QStringList& appSignalIds);

private slots:
	void slot_realtimeDataReceived(QString sourceEquipmentId,
								   std::shared_ptr<TrendLib::RealtimeData> data,
								   TrendLib::TrendStateItem /*minState*/,
								   TrendLib::TrendStateItem /*maxState*/);
	void slot_connectionLost(QString sourceEquipmentId);

private:
	void appendRealtimeData(QString sourceEquipmentId, Hash signalHash, const std::vector<TrendLib::TrendStateItem>& states);
	void appendRealtimeData_unsafe(QString sourceEquipmentId, E::TimeType timeType, const std::vector<TrendLib::TrendStateItem>& states, TrendLib::TrendArchive* archive);
	void trimArchive_unsafe(int durationSeconds, TrendLib::TrendArchive* archive);

private:
	ClientLib::RtDataProvider m_dataProvider;

	mutable QMutex m_signalMutex;

	int m_durationSeconds = 0;
	E::RtTrendsSamplePeriod m_samplePeriod = E::RtTrendsSamplePeriod::sp_1s;
	E::TimeType m_timeType = E::TimeType::Local;
	std::map<Hash, RtSchemaTrendSignal> m_trendSignals;		// Key is hash from appSignalId
};


//
// RealTime trend class for SchemaItemIndicator, type Trend
// Each schema item contains it's own RtDataProvider
//
class RtSchemaTrend : public QObject
{
	Q_OBJECT

public:
	RtSchemaTrend(const MonitorConfigController& configController, const ISignalDataServer& signalDataServer);
	virtual ~RtSchemaTrend();

public:
	void updateRealtimeConnections();		// Updates m_rtConnections from SchemaDetaisSet

	bool trendData(QUuid trendUuid,
				   const QString& appSignalId,
				   std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const;

	TimeStamp maxTimeStamp(QUuid trendUuid) const;

private:
	const MonitorConfigController& m_configController;
	const ISignalDataServer& m_signalDataServer;

	// trendData() is called from other thread, so we don't wnat to delete data provider
	// while trendData() is working
	//
	mutable QMutex m_mutex;

	std::map<QUuid, RtSchemaTrendDataProvider> m_dataProviders;	// Key is SchemaItem uuid, receiver of data
};

