#include "SchemaManager.h"

SchemaManager::SchemaManager(MonitorConfigController* configController) :
	QObject(nullptr),
	m_configController(configController)
{
	assert(m_configController);

	connect(m_configController, &MonitorConfigController::configurationArrived, this, &SchemaManager::slot_configurationArrived);
}

SchemaManager::~SchemaManager()
{
}

std::shared_ptr<VFrame30::Schema> SchemaManager::schema(QString schemaId)
{
	assert(m_configController);

	auto found = m_schemas.find(schemaId);

	if (found == m_schemas.end())
	{
		// Schema is not read yet
		//
		std::shared_ptr<VFrame30::Schema> schema = loadSchema(schemaId);
		return schema;
	}
	else
	{
		return found->second;
	}
}

std::shared_ptr<VFrame30::Schema>  SchemaManager::loadSchema(QString schemaId)
{
	QByteArray data;
	QString errorString;

	bool result = m_configController->getFileBlockedById(schemaId, &data, &errorString);

	if (result == false)
	{
		return std::shared_ptr<VFrame30::Schema>();
	}

	VFrame30::Schema* rawSchema = VFrame30::Schema::Create(data);
	std::shared_ptr<VFrame30::Schema> schema(rawSchema);

	if (rawSchema == nullptr)
	{
		return std::shared_ptr<VFrame30::Schema>();
	}

	if (rawSchema->isLogicSchema() == true)
	{
		qDebug() << "rawSchema->isLogicSchema()";
	}

	if (rawSchema->isMonitorSchema() == true)
	{
		qDebug() << "rawSchema->isMonitorSchema()";
	}

	// Add schema to map
	//
	if (rawSchema->schemaID() != schemaId)
	{
		// FileRecord ID attribute has one SchemaID and file contains other SchemaID, womething wrong!!!
		//
		qDebug() << "FileRecord ID attribute has one SchemaID and file contains other SchemaID, womething wrong!!!"
				 << " File ID: " << schemaId
				 << " SchemaFileID: " << rawSchema->schemaID();

		return std::shared_ptr<VFrame30::Schema>();
	}

	m_schemas[schema->schemaID()] = schema;

	return schema;
}

void SchemaManager::slot_configurationArrived(ConfigSettings configuration)
{
	m_schemas.clear();
	emit resetSchema(configuration.startSchemaId);
	return;
}
