#pragma once

#include "../SchemaClientLib/ClientSchemaManager.h"
#include "../lib/ISignalDataServer.h"
#include "./Trend/RtSchemaTrend.h"


class MonitorConfigController;


class MonitorSchemaManager : public SchemaClientLib::ClientSchemaManager
{
	Q_OBJECT

public:
	explicit MonitorSchemaManager(MonitorConfigController& configController,
								  const ISignalDataServer& signalDataServer,
								  QObject* parent = nullptr);

public:
	// RealTimeTrends for schemas, SchemaItemIndicator, type = Trend
	// RealTime Trends (ITrendDataProvider)
	//
	virtual bool trendData(QUuid trendUuid,
						   const TrendLib::TrendSignalParam& trendSignal,
						   QDateTime from,
						   QDateTime to,
						   E::TimeType timeType,
						   E::TrendMode mode,
						   std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const override;

	virtual TimeStamp maxTimeStamp(QUuid trendUuid, E::TimeType timeType) const override;

	void updateConfiguration(const MonitorConfigSettings& configuration);

public:
	[[nodiscard]] MonitorConfigController& configController();
	[[nodiscard]] const MonitorConfigController& configController() const;

	// Data
	//
private:
	const ISignalDataServer& m_signalDataServer;

	// Data for RealTimeTrends on schemas, SchemaItemIndicator, type = Trend
	//
	RtSchemaTrend m_rtTrendSchemas;
};
