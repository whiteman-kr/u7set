#include "SchemaManager.h"
#include "LogicSchema.h"

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

			// and there is no such scheme (((
			// Just create an empty one, so wi can display at least blank space
			//
			schema = std::make_shared<VFrame30::LogicSchema>();
			schema->setSchemaId("EMPTYSCHEMA");
			schema->setCaption("Empty Schema");
		}

		return schema;
	}

	int SchemaManager::schemaCount() const
	{
		Q_ASSERT(false);		// "Must be implemented in derived class";
		return 0;
	}

	std::shared_ptr<VFrame30::Schema> SchemaManager::schemaByIndex(int /*schemaIndex*/)
	{
		Q_ASSERT(false);		// "Must be implemented in derived class";
		return {};
	}

	QString SchemaManager::schemaCaptionById(const QString& /*schemaId*/) const
	{
		Q_ASSERT(false);		// "Must be implemented in derived class";
		return {};
	}

	QString SchemaManager::schemaCaptionByIndex(int /*schemaIndex*/) const
	{
		Q_ASSERT(false);		// "Must be implemented in derived class";
		return {};
	}

	QString SchemaManager::schemaIdByIndex(int /*schemaIndex*/) const
	{
		Q_ASSERT(false);		// "Must be implemented in derived class";
		return {};
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
		m_globalScript = value + QChar::LineFeed;
	}

}
