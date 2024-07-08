#pragma once

#include <ClientLib/ISignalDataServer.h>
#include <SchemaClientLib/ClientSchemaManager.h>
#include "DiagConfigController.h"


class DiagnosticsSchemaManager : public SchemaClientLib::ClientSchemaManager
{
	Q_OBJECT

public:
	explicit DiagnosticsSchemaManager(DiagConfigController& configController,
									  const ClientLib::ISignalDataServer& signalDataServer,
									  QObject* parent = nullptr);

	// RealTimeTrends for schemas, SchemaItemIndicator, type = Trend
	//
public:
	//// RealTime Trends (ITrendDataProvider)
	////
	// virtual bool trendData(QUuid trendUuid,
	//					   const TrendLib::TrendSignalParam& trendSignal,
	//					   QDateTime from,
	//					   QDateTime to,
	//					   E::TimeType timeType,
	//					   E::TrendMode mode,
	//					   std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const override;

	// virtual TimeStamp maxTimeStamp(QUuid trendUuid, E::TimeType timeType) const override;

	void updateConfiguration(const DiagConfigSettings& configuration);

public:
	[[nodiscard]] DiagConfigController& configController();
	[[nodiscard]] const DiagConfigController& configController() const;

	// Data
	//
private:
	const ClientLib::ISignalDataServer& m_signalDataServer;

	//// Data for RealTimeTrends on schemas, SchemaItemIndicator, type = Trend
	////
	// RtSchemaTrend m_rtTrendSchemas;
};
