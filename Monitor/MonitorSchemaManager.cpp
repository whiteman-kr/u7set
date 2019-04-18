#include "MonitorSchemaManager.h"

MonitorSchemaManager::MonitorSchemaManager(MonitorConfigController* configController, QObject* parent /*= nullptr*/) :
	VFrame30::SchemaManager(parent),
	m_configController(configController)
{
	Q_ASSERT(m_configController);

	connect(m_configController, &MonitorConfigController::configurationArrived, this, &MonitorSchemaManager::slot_configurationArrived);

	return;
}

bool MonitorSchemaManager::hasSchema(QString schemaId) const
{
	if (m_configController == nullptr)
	{
		Q_ASSERT(m_configController);
		return false;
	}

	return m_configController->hasFileId(schemaId);
}


std::shared_ptr<VFrame30::Schema> MonitorSchemaManager::loadSchema(QString schemaId)
{
	QByteArray data;
	QString errorString;

	bool result = m_configController->getFileBlockedById(schemaId, &data, &errorString);
	if (result == false)
	{
		return std::shared_ptr<VFrame30::Schema>();
	}

	std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(data);

	return schema;
}

void MonitorSchemaManager::slot_configurationArrived(ConfigSettings configuration)
{
	clear();

	setGlobalScript(configuration.globalScript);
	setOnConfigurationArrivedScript(configuration.onConfigurationArrivedScript);

	return;
}

MonitorConfigController* MonitorSchemaManager::monitorConfigController()
{
	if (m_configController == nullptr)
	{
		Q_ASSERT(m_configController);
		return nullptr;
	}

	return m_configController;
}

const MonitorConfigController* MonitorSchemaManager::monitorConfigController() const
{
	if (m_configController == nullptr)
	{
		Q_ASSERT(m_configController);
		return nullptr;
	}

	return m_configController;
}

QString MonitorSchemaManager::onConfigurationArrivedScript() const
{
	return m_onConfigurationArrivedScript;
}

void MonitorSchemaManager::setOnConfigurationArrivedScript(QString value)
{
	m_onConfigurationArrivedScript = value;
}
