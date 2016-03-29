#include "Stable.h"
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
		 
		for (auto vf = m_schemeIds.begin(); vf != m_schemeIds.end(); ++vf)
		{
			Proto::Uuid* pGuid = pMutableConfiguration->add_schemesids();
			Proto::Write(pGuid, *vf);
		}

		bool saveFrameResult = true;

		for (auto vf = m_schemes.begin(); vf != m_schemes.end(); ++vf)
		{
			Proto::Envelope* scheme = pMutableConfiguration->add_schemes();
			saveFrameResult &= vf->get()->Save(scheme);
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

		m_schemeIds.clear();
		for (int i = 0; i < configuration.schemesids().size(); i++)
		{
			const QUuid& schemeGuid = Proto::Read(configuration.schemesids(i));
			m_schemeIds.push_back(schemeGuid);

			assert(schemeGuid.isNull() == false);
		}

		m_schemes.clear();
		for (int i = 0; i < configuration.schemes().size(); i++)
		{
			Schema* scheme = Schema::Create(configuration.schemes(i));

			if (scheme == nullptr)
			{
				assert(scheme != nullptr);
				continue;
			}

			m_schemes.push_back(std::shared_ptr<Schema>(scheme));
		}

		return true;
	}

	Configuration* Configuration::CreateObject(const Proto::Envelope& message)
	{
		// Ёта функци€ может создавать только один экземпл€р
		//
		if (message.has_configuration() == false)
		{
			assert(message.has_configuration());
			return nullptr;
		}

		//quint32 classNameHash = message.classnamehash();

		Configuration* pConfiguration = new Configuration();
		bool result = pConfiguration->LoadData(message);

		if (result == false)
		{
			delete pConfiguration;
			return nullptr;
		}

		return pConfiguration;
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

	const std::vector<QUuid>& Configuration::schemesIDs() const
	{
		return m_schemeIds;
	}

	std::vector<QUuid>* Configuration::mutableSchemesIDs()
	{
		return &m_schemeIds;
	}

	const std::vector<std::shared_ptr<Schema>>& Configuration::schemes() const
	{
		return m_schemes;
	}

	std::vector<std::shared_ptr<Schema>>* Configuration::mutableSchemes()
	{
		return &m_schemes;
	}

}
