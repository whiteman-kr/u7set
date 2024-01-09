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
		emit schemasWereReseted();
		return;
	}

	std::shared_ptr<VFrame30::Schema> SchemaManager::schema(QString schemaId, std::shared_ptr<Context> context)
	{
		Q_ASSERT(context);

		std::shared_ptr<VFrame30::Schema> schema = loadSchema(schemaId);

		if (schema != nullptr)
		{
			if (schema->schemaId() != schemaId)
			{
				qDebug() << "Requested schema is not loaded one, "
						 << " Requsted SchemaID: " << schemaId
						 << ", Loaded SchemaID: " << schema->schemaId();

				return {};
			}
		}
		else
		{
			qDebug() << "SchemaManager::schema: Can't load schema " << schemaId;

			// and there is no such scheme (((
			// Just create an empty one, so ww can display at least blank space
			//
			schema = std::make_shared<VFrame30::LogicSchema>();
			schema->setSchemaId("EMPTYSCHEMA");
			schema->setCaption("Empty Schema");
		}

		// Set context to the newly created schema, this should be the only place for setting context
		// for cleints.
		//
		schema->setContext(std::move(context));

		return schema;
	}

	int SchemaManager::schemaCount() const
	{
		Q_ASSERT(false);		// "Must be implemented in derived class";
		return 0;
	}

	std::shared_ptr<VFrame30::Schema> SchemaManager::schemaByIndex(int /*schemaIndex*/, std::shared_ptr<Context> /*context*/)
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

	bool SchemaManager::trendData(QUuid /*trendUuid*/,
								  const TrendLib::TrendSignalParam& /*trendSignal*/,
								  QDateTime /*from*/,
								  QDateTime /*to*/,
								  E::TimeType /*timeType*/,
								  E::TrendMode /*mode*/,
								  std::list<std::shared_ptr<TrendLib::OneHourData>>* /*outData*/) const
	{
		Q_ASSERT(false);
		return false;
	}

	TimeStamp SchemaManager::maxTimeStamp(QUuid /*trendUuid*/, E::TimeType /*timeType*/) const
	{
		Q_ASSERT(false);
		return {};
	}

	// Load schema, must be overriden to perform loading schema appropriate to client.
	//
	std::shared_ptr<VFrame30::Schema> SchemaManager::loadSchema(const QString& schemaId)
	{
		Q_UNUSED(schemaId);
		Q_ASSERT(false);
		return std::shared_ptr<VFrame30::Schema>();
	}

}
