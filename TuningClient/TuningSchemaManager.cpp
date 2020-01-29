#include "TuningSchemaManager.h"

TuningSchemaManager::TuningSchemaManager(ConfigController* configController, QObject* parent) :
	VFrame30::SchemaManager(parent),
	m_configController(configController)
{
	assert(m_configController);

	connect(m_configController, &ConfigController::globalScriptArrived, this, &TuningSchemaManager::globalScriptArrived);
	return;
}

int TuningSchemaManager::schemaCount() const
{
	return m_configController->schemaCount();
}

std::shared_ptr<VFrame30::Schema> TuningSchemaManager::schemaByIndex(int schemaIndex)
{
	if (schemaIndex < 0)
	{
		return {};
	}

	QString schemaId = schemaIdByIndex(schemaIndex);
	if (schemaId.isEmpty() == true)
	{
		return {};
	}

	return schema(schemaId);
}

QString TuningSchemaManager::schemaCaptionById(const QString& schemaId) const
{
	return m_configController->schemaCaptionById(schemaId);
}

QString TuningSchemaManager::schemaCaptionByIndex(int schemaIndex) const
{
	return m_configController->schemaCaptionByIndex(schemaIndex);
}

QString TuningSchemaManager::schemaIdByIndex(int schemaIndex) const
{
	return m_configController->schemaIdByIndex(schemaIndex);
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

