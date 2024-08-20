#include "MonitorSchemaManager.h"
#include "MonitorConfigController.h"

MonitorSchemaManager::MonitorSchemaManager(MonitorConfigController& configController,
										   const ClientLib::ISignalDataServer& signalDataServer,
										   QObject* parent /*= nullptr*/) :
	SchemaClientLib::ClientSchemaManager(configController, parent),
	m_signalDataServer(signalDataServer),
	m_rtTrendSchemas(configController, signalDataServer)
{
	return;
}

bool MonitorSchemaManager::trendData(QUuid trendUuid,
									 const TrendLib::TrendSignalParam& trendSignal,
									 QDateTime /*from*/,
									 QDateTime /*to*/,
									 E::TimeType /*timeType*/,
									 E::TrendMode /*mode*/,
									 std::list<std::shared_ptr<TrendLib::OneHourData>>* outData) const
{
	return m_rtTrendSchemas.trendData(trendUuid, trendSignal.appSignalId(), outData);
}

TimeStamp MonitorSchemaManager::maxTimeStamp(QUuid trendUuid, E::TimeType /*timeType*/) const
{
	return m_rtTrendSchemas.maxTimeStamp(trendUuid);
}

void MonitorSchemaManager::updateConfiguration(const MonitorConfigSettings& /*configuration*/)
{
	clear();

	// Schemas Realtime Trends
	// At this point m_configController already has SchemaDetails.pbuf,so we can use it
	//
	m_rtTrendSchemas.updateRealtimeConnections();
	return;
}

MonitorConfigController& MonitorSchemaManager::configController()
{
	return static_cast<MonitorConfigController&>(SchemaClientLib::ClientSchemaManager::configController());
}

const MonitorConfigController& MonitorSchemaManager::configController() const
{
	return static_cast<const MonitorConfigController&>(SchemaClientLib::ClientSchemaManager::configController());
}

