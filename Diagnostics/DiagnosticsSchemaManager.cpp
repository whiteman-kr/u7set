#include "DiagnosticsSchemaManager.h"


DiagnosticsSchemaManager::DiagnosticsSchemaManager(DiagConfigController& configController,
										   const ISignalDataServer& signalDataServer,
										   QObject* parent /*= nullptr*/) :
	SchemaClientLib::ClientSchemaManager(configController, parent),
	m_signalDataServer(signalDataServer)
	//m_rtTrendSchemas(configController, signalDataServer)
{
	return;
}

//bool DiagnosticsSchemaManager::trendData(QUuid trendUuid,
//									 const TrendLib::TrendSignalParam& trendSignal,
//									 QDateTime /*from*/,
//									 QDateTime /*to*/,
//									 E::TimeType /*timeType*/,
//									 E::TrendMode /*mode*/,
//									 std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const
//{
//	return m_rtTrendSchemas.trendData(trendUuid, trendSignal.appSignalId(), outData);
//}

//TimeStamp DiagnosticsSchemaManager::maxTimeStamp(QUuid trendUuid, E::TimeType /*timeType*/) const
//{
//	return m_rtTrendSchemas.maxTimeStamp(trendUuid);
//}

void DiagnosticsSchemaManager::updateConfiguration(const DiagConfigSettings& /*configuration*/)
{
	clear();

	// Schemas Realtime Trends
	// At this point m_configController already has SchemaDetails.pbuf,so we can use it
	//
	//m_rtTrendSchemas.updateRealtimeConnections();
	return;
}

DiagConfigController& DiagnosticsSchemaManager::configController()
{
	return static_cast<DiagConfigController&>(SchemaClientLib::ClientSchemaManager::configController());
}

const DiagConfigController& DiagnosticsSchemaManager::configController() const
{
	return static_cast<const DiagConfigController&>(SchemaClientLib::ClientSchemaManager::configController());
}

