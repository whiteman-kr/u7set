#include "MonitorSchemaManager.h"

MonitorSchemaManager::MonitorSchemaManager(MonitorConfigController* configController, QObject* parent /*= nullptr*/) :
	VFrame30::SchemaManager(parent),
	m_configController(configController)
{
	assert(m_configController);

	connect(m_configController, &MonitorConfigController::configurationArrived, this, &MonitorSchemaManager::slot_configurationArrived);

	return;
}

std::shared_ptr<VFrame30::Schema>  MonitorSchemaManager::loadSchema(QString schemaId)
{
	QByteArray data;
	QString errorString;

	bool result = m_configController->getFileBlockedById(schemaId, &data, &errorString);
	if (result == false)
	{
		return std::shared_ptr<VFrame30::Schema>();
	}

	std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(data);
	if (schema == nullptr)
	{
		return schema;
	}

	return schema;
}

void MonitorSchemaManager::slot_configurationArrived(ConfigSettings configuration)
{
	clear();
	setGlobalScript(configuration.globalScript);
	return;
}

