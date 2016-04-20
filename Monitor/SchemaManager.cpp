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
	auto found = m_schemas.find(schemaId);

	if (found == m_schemas.end())
	{
		// Schema is not read yet
		//
		QByteArray data;
		QString errorString;

		bool result = m_configController->getFileBlockedById(schemaId, &data, &errorString);
		if (result == true)
		{
			VFrame30::Schema* rawSchema = VFrame30::Schema::Create(data);
			std::shared_ptr<VFrame30::Schema> schema(rawSchema);

			if (rawSchema != nullptr)
			{
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
			}

			return schema;
		}
		else
		{
			return std::shared_ptr<VFrame30::Schema>();
		}
	}
	else
	{
		return found->second;
	}
}

void SchemaManager::slot_configurationArrived(ConfigSettings configuration)
{
	m_schemas.clear();

	// DEBUGGGGG!!!!!!!!!!!!

	std::shared_ptr<VFrame30::Schema> s = schema(configuration.startSchemaId);
	if (s != nullptr)
	{
		qDebug() << s->caption();
	}

	// END OF DEBUGGGGG!!!!!!!!!!!!

	emit resetSchema(configuration.startSchemaId);
	return;
}
