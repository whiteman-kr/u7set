#include "Configuration.h"
#include "Schema.h"

namespace VFrame30
{
	Configuration::Configuration()
	{
	}

	Configuration::~Configuration()
	{
	}

	// Serialization
	//

	bool Configuration::SaveData(Proto::Envelope* message) const
	{
		const std::string& className = this->metaObject()->className();
		quint32 classnamehash = CUtils::GetClassHashCode(className);

		message->set_classnamehash(classnamehash);

		// --
		//
		Proto::Configuration* pMutableConfiguration = message->mutable_configuration();

		Proto::Write(pMutableConfiguration->mutable_uuid(), m_guid);
		Proto::Write(pMutableConfiguration->mutable_strid(), m_strID);
		Proto::Write(pMutableConfiguration->mutable_caption(), m_caption);
		Proto::Write(pMutableConfiguration->mutable_variables(), m_variables);
		Proto::Write(pMutableConfiguration->mutable_globals(), m_globals);
		 
		for (auto vf = m_schemaIds.begin(); vf != m_schemaIds.end(); ++vf)
		{
			Proto::Uuid* pGuid = pMutableConfiguration->add_schemasids();
			Proto::Write(pGuid, *vf);
		}

		bool saveFrameResult = true;

		for (auto vf = m_schemas.begin(); vf != m_schemas.end(); ++vf)
		{
			Proto::Envelope* schema = pMutableConfiguration->add_schemas();
			saveFrameResult &= vf->get()->Save(schema);
		}
				
		return saveFrameResult;
	}

	bool Configuration::LoadData(const Proto::Envelope& message)
	{
		if (message.has_configuration() == false)
		{
			assert(message.has_configuration());
			return false;
		}

		const Proto::Configuration& configuration = message.configuration();

		m_guid = Proto::Read(configuration.uuid());
		Proto::Read(configuration.strid(), &m_strID);
		Proto::Read(configuration.caption(), &m_caption);
		Proto::Read(configuration.variables(), &m_variables);
		Proto::Read(configuration.globals(), &m_globals);

		m_schemaIds.clear();
		for (int i = 0; i < configuration.schemasids().size(); i++)
		{
			const QUuid& schemaGuid = Proto::Read(configuration.schemasids(i));
			m_schemaIds.push_back(schemaGuid);

			assert(schemaGuid.isNull() == false);
		}

		m_schemas.clear();
		for (int i = 0; i < configuration.schemas().size(); i++)
		{
			std::shared_ptr<Schema> schema = Schema::Create(configuration.schemas(i));

			if (schema == nullptr)
			{
				assert(schema != nullptr);
				continue;
			}

			m_schemas.push_back(schema);
		}

		return true;
	}

	std::shared_ptr<Configuration> Configuration::CreateObject(const Proto::Envelope& message)
	{
		// Ёта функци€ может создавать только один экземпл€р
		//
		if (message.has_configuration() == false)
		{
			assert(message.has_configuration());
			return nullptr;
		}

		//quint32 classNameHash = message.classnamehash();

		std::shared_ptr<Configuration> configuration = std::make_shared<Configuration>();

		bool result = configuration->LoadData(message);

		if (result == false)
		{
			return std::shared_ptr<Configuration>();
		}

		return configuration;
	}


	// Properties and Datas
	//
	const QUuid& Configuration::guid() const
	{
		return m_guid;
	}

	void Configuration::setGuid(const QUuid& guid)
	{
		m_guid = guid;
		return;
	}

	const QString& Configuration::strID() const
	{
		return m_strID;
	}

	void Configuration::setStrID(const QString& strID)
	{
		m_strID = strID;
	}

	const QString& Configuration::caption() const
	{
		return m_caption;
	}

	void Configuration::setCaption(const QString& caption)
	{
		m_caption = caption;
	}

	const QString& Configuration::variables() const
	{
		return m_variables;
	}

	void Configuration::setVariables(const QString& variables)
	{
		m_variables = variables;
	}

	const QString& Configuration::globals() const
	{
		return m_globals;
	}

	void Configuration::setGlobals(const QString& globals)
	{
		m_globals = globals;
	}

	const std::vector<QUuid>& Configuration::schemasIDs() const
	{
		return m_schemaIds;
	}

	std::vector<QUuid>* Configuration::mutableSchemasIDs()
	{
		return &m_schemaIds;
	}

	const std::vector<std::shared_ptr<Schema>>& Configuration::schemas() const
	{
		return m_schemas;
	}

	std::vector<std::shared_ptr<Schema>>* Configuration::mutableSchemas()
	{
		return &m_schemas;
	}

}
