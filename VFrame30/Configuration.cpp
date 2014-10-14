#include "Stable.h"
#include "Configuration.h"
#include "Scheme.h"

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
		 
		for (auto vf = m_videoFramesIDs.begin(); vf != m_videoFramesIDs.end(); ++vf)
		{
			Proto::Uuid* pGuid = pMutableConfiguration->add_videoframesids();
			Proto::Write(pGuid, *vf);
		}

		bool saveFrameResult = true;

		for (auto vf = m_videoFrames.begin(); vf != m_videoFrames.end(); ++vf)
		{
			Proto::Envelope* pVideoFrame = pMutableConfiguration->add_videoframes();
			saveFrameResult &= vf->get()->Save(pVideoFrame);
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
		m_strID = Proto::Read(configuration.strid());
		m_caption = Proto::Read(configuration.caption());
		m_variables = Proto::Read(configuration.variables());
		m_globals = Proto::Read(configuration.globals());

		m_videoFramesIDs.clear();
		for (int i = 0; i < configuration.videoframesids().size(); i++)
		{
			const QUuid& videoFrameGuid = Proto::Read(configuration.videoframesids(i));
			m_videoFramesIDs.push_back(videoFrameGuid);

			assert(videoFrameGuid.isNull() == false);
		}

		m_videoFrames.clear();
		for (int i = 0; i < configuration.videoframes().size(); i++)
		{
			Scheme* pVideoFrame = Scheme::Create(configuration.videoframes(i));

			if (pVideoFrame == nullptr)
			{
				assert(pVideoFrame != nullptr);
				continue;
			}

			m_videoFrames.push_back(std::shared_ptr<Scheme>(pVideoFrame));
		}

		return true;
	}

	Configuration* Configuration::CreateObject(const Proto::Envelope& message)
	{
		// ��� ������� ����� ��������� ������ ���� ���������
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

	const std::vector<QUuid>& Configuration::videoFramesIDs() const
	{
		return m_videoFramesIDs;
	}

	std::vector<QUuid>* Configuration::mutableVideoFramesIDs()
	{
		return &m_videoFramesIDs;
	}

	const std::vector<std::shared_ptr<Scheme>>& Configuration::videoFrames() const
	{
		return m_videoFrames;
	}

	std::vector<std::shared_ptr<Scheme>>* Configuration::mutableVideoFrames()
	{
		return &m_videoFrames;
	}

}
