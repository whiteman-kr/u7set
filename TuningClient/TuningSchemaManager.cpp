#include "TuningSchemaManager.h"

TuningSchemaManager::TuningSchemaManager(ConfigController* configController, QObject* parent) :
	VFrame30::SchemaManager(parent),
	m_configController(configController)
{
	assert(m_configController);

	connect(m_configController, &ConfigController::globalScriptArrived, this, &TuningSchemaManager::globalScriptArrived);
	return;
}

std::shared_ptr<VFrame30::Schema> TuningSchemaManager::loadSchema(QString schemaId)
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

void TuningSchemaManager::globalScriptArrived(QByteArray data)
{
	setGlobalScript(QString(data));
	return;
}

