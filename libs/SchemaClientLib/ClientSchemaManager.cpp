#include "SchemaClientLib/ClientSchemaManager.h"
#include "SchemaClientLib/SchemaClientConfigController.h"

namespace SchemaClientLib
{
	ClientSchemaManager::ClientSchemaManager(SchemaClientConfigController& configController, QObject* parent /*= nullptr*/) :
		VFrame30::SchemaManager(parent),
		m_configController(configController)
	{
		return;
	}

	std::shared_ptr<VFrame30::Schema> ClientSchemaManager::loadSchema(const QString& schemaId)
	{
		std::shared_ptr<VFrame30::Schema> result;

		QByteArray data;
		QString errorString;

		bool ok = m_configController.getFileBlockedById(schemaId, &data, &errorString);
		if (ok == false)
		{
			qWarning() << "ClientSchemaManager::loadSchema: " << schemaId << ", error: " << errorString;
		}
		else
		{
			result = VFrame30::Schema::Create(data);
		}
		
		return result;
	}

	bool ClientSchemaManager::hasSchema(const QString& schemaId) const
	{
		return m_configController.hasFileId(schemaId);
	}

	int ClientSchemaManager::schemaCount() const
	{
		return m_configController.schemaCount();
	}

	std::shared_ptr<VFrame30::Schema> ClientSchemaManager::schemaByIndex(int schemaIndex,
																		  std::shared_ptr<VFrame30::Context> context)
	{
		if (schemaIndex < 0 ||
			context == nullptr)
		{
			Q_ASSERT(context);
			return {};
		}

		QString schemaId = schemaIdByIndex(schemaIndex);
		if (schemaId.isEmpty() == true)
		{
			return {};
		}

		return schema(schemaId, std::move(context));
	}

	QString ClientSchemaManager::schemaCaptionById(const QString& schemaId) const
	{
		return m_configController.schemaCaptionById(schemaId);
	}

	QString ClientSchemaManager::schemaCaptionByIndex(int schemaIndex) const
	{
		return m_configController.schemaCaptionByIndex(schemaIndex);
	}

	QString ClientSchemaManager::schemaIdByIndex(int schemaIndex) const
	{
		return m_configController.schemaIdByIndex(schemaIndex);
	}

	SchemaClientConfigController& ClientSchemaManager::configController()
	{
		return m_configController;
	}

	const SchemaClientConfigController& ClientSchemaManager::configController() const
	{
		return m_configController;
	}

}