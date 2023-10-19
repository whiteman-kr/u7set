#pragma once

#include "../lib/ISignalDataServer.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaManager.h"
#include "MonitorConfigController.h"
#include "./Trend/RtSchemaTrend.h"

class MonitorSchemaManager : public VFrame30::SchemaManager
{
	Q_OBJECT

public:
	explicit MonitorSchemaManager(MonitorConfigController& configController,
								  const ISignalDataServer& signalDataServer,
								  QObject* parent = nullptr);
	virtual ~MonitorSchemaManager();

public:
	[[nodiscard]] bool hasSchema(QString schemaId) const;

protected:
	[[nodiscard]] virtual std::shared_ptr<VFrame30::Schema> loadSchema(QString schemaId) override;

public:
	[[nodiscard]] virtual int schemaCount() const override;
	[[nodiscard]] virtual std::shared_ptr<VFrame30::Schema> schemaByIndex(int schemaIndex,
																		  std::shared_ptr<VFrame30::Context> context) override;

	[[nodiscard]] virtual QString schemaCaptionById(const QString& schemaId) const override;
	[[nodiscard]] virtual QString schemaCaptionByIndex(int schemaIndex) const override;
	[[nodiscard]] virtual QString schemaIdByIndex(int schemaIndex) const override;

	// RealTimeTrends for schemas, SchemaItemIndicator, type = Trend
	//
public:

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

	void updateConfiguration(const ConfigSettings& configuration);

public:
	[[nodiscard]] MonitorConfigController& monitorConfigController();
	[[nodiscard]] const MonitorConfigController& monitorConfigController() const;

	// Data
	//
private:
	MonitorConfigController& m_configController;
	const ISignalDataServer& m_signalDataServer;

	// Data for RealTimeTrends on schemas, SchemaItemIndicator, type = Trend
	//
	RtSchemaTrend m_rtTrendSchemas;
};

