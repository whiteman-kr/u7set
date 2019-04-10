#include "SchemaManager.h"

namespace VFrame30
{

	SchemaManager::SchemaManager(QObject* parent) :
		QObject(parent)
	{
		qDebug() << "SchemaManager::SchemaManager";
	}

	SchemaManager::~SchemaManager()
	{
		qDebug() << "SchemaManager::~SchemaManager";
	}

	void SchemaManager::clear()
	{
		m_globalScript.clear();
		emit schemasWereReseted();
		return;
	}

	std::shared_ptr<VFrame30::Schema> SchemaManager::schema(QString schemaId)
	{
		std::shared_ptr<VFrame30::Schema> schema = loadSchema(schemaId);

		if (schema != nullptr)
		{
			// Add schema to map
			//
			if (schema->schemaId() != schemaId)
			{
				qDebug() << "Requested schema is not loaded one, "
						 << " Requsted SchemaID: " << schemaId
						 << ", Loaded SchemaID: " << schema->schemaId();

				return std::shared_ptr<VFrame30::Schema>();
			}
		}
		else
		{
			qDebug() << "SchemaManager::schema: Can't load schema " << schemaId;
		}

		return schema;
	}

	// Load schema, must be overriden to perform loading schema appropriate to client.
	//
	std::shared_ptr<VFrame30::Schema> SchemaManager::loadSchema(QString schemaId)
	{
		Q_UNUSED(schemaId);
		Q_ASSERT(false);
		return std::shared_ptr<VFrame30::Schema>();
	}

	const QString& SchemaManager::globalScript() const
	{
		return m_globalScript;
	}

	void SchemaManager::setGlobalScript(const QString& value)
	{
		m_globalScript = value;
	}

}
