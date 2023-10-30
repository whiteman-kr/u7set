#include "MonitorSchemaManager.h"

MonitorSchemaManager::MonitorSchemaManager(MonitorConfigController& configController,
										   const ISignalDataServer& signalDataServer,
										   QObject* parent /*= nullptr*/) :
	VFrame30::SchemaManager(parent),
	m_configController(configController),
	m_signalDataServer(signalDataServer),
	m_rtTrendSchemas(configController, signalDataServer)
{

	return;
}

MonitorSchemaManager::~MonitorSchemaManager()
{
	return;
}

bool MonitorSchemaManager::hasSchema(QString schemaId) const
{
	return m_configController.hasFileId(schemaId);
}


std::shared_ptr<VFrame30::Schema> MonitorSchemaManager::loadSchema(QString schemaId)
{
	QByteArray data;
	QString errorString;

	bool result = m_configController.getFileBlockedById(schemaId, &data, &errorString);
	if (result == false)
	{
		return {};
	}

	return VFrame30::Schema::Create(data);
}

int MonitorSchemaManager::schemaCount() const
{
	return m_configController.schemaCount();
}

std::shared_ptr<VFrame30::Schema> MonitorSchemaManager::schemaByIndex(int schemaIndex,
																	  std::shared_ptr<VFrame30::Context> context)
{
	if (schemaIndex < 0 ||
		context == nullptr)
	{
		Q_ASSERT(context);
		return {};
	}

	QString schemaId = schemaIdByIndex(schemaIndex);
	if (schemaId.isEmpty() == true)
	{
		return {};
	}

	return schema(schemaId, std::move(context));
}

QString MonitorSchemaManager::schemaCaptionById(const QString& schemaId) const
{
	return m_configController.schemaCaptionById(schemaId);
}

QString MonitorSchemaManager::schemaCaptionByIndex(int schemaIndex) const
{
	return m_configController.schemaCaptionByIndex(schemaIndex);
}

QString MonitorSchemaManager::schemaIdByIndex(int schemaIndex) const
{
	return m_configController.schemaIdByIndex(schemaIndex);
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

void MonitorSchemaManager::updateConfiguration(const ConfigSettings& /*configuration*/)
{
	clear();

	// Schemas Realtime Trends
	// At this point m_configController already has SchemaDetails.pbuf,so we can use it
	//
	m_rtTrendSchemas.updateRealtimeConnections();
	return;
}

MonitorConfigController& MonitorSchemaManager::monitorConfigController()
{
	return m_configController;
}

const MonitorConfigController& MonitorSchemaManager::monitorConfigController() const
{
	return m_configController;
}

