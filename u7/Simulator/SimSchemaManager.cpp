#include "SimSchemaManager.h"

SimSchemaManager::SimSchemaManager(/*MonitorConfigController* configController, */QObject* parent /*= nullptr*/) :
	VFrame30::SchemaManager(parent)
	//m_configController(configController)
{
//	assert(m_configController);
//	connect(m_configController, &MonitorConfigController::configurationArrived, this, &MonitorSchemaManager::slot_configurationArrived);

	return;
}

std::shared_ptr<VFrame30::Schema> SimSchemaManager::loadSchema(QString schemaId)
{
	assert(false);
	bool to_do_SimSchemaManager_loadSchema;
	return std::shared_ptr<VFrame30::Schema>();
//	QByteArray data;
//	QString errorString;

//	bool result = m_configController->getFileBlockedById(schemaId, &data, &errorString);
//	if (result == false)
//	{
//		return std::shared_ptr<VFrame30::Schema>();
//	}

//	std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(data);

//	return schema;
}

//void MonitorSchemaManager::slot_configurationArrived(ConfigSettings configuration)
//{
//	clear();
//	setGlobalScript(configuration.globalScript);
//	return;
//}

