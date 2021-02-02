#include "TestClientCfgGenerator.h"

namespace Builder
{
	TestClientCfgGenerator::TestClientCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
	}

	bool TestClientCfgGenerator::createSettingsProfile(const QString& profile)
	{
		TestClientSettingsGetter settingsGetter;

		if (settingsGetter.readFromDevice(m_context, m_software) == false)
		{
			return false;
		}

		return m_settingsSet.addProfile<TestClientSettings>(profile, settingsGetter);
	}

	bool TestClientCfgGenerator::generateConfigurationStep1()
	{
		bool result = false;

		do
		{
			if (linkAppSignalsFile() == false) break;
			if (writeBatFile() == false) break;
			if (writeShFile() == false) break;

			result = true;
		}
		while(false);

		return result;
	}

	bool TestClientCfgGenerator::generateConfigurationStep2()
	{
		bool result = true;

		QStringList appDataServicesIDs;

		for(auto p : m_context->m_software)
		{
			const Hardware::Software* sw = p.second;

			TEST_PTR_CONTINUE(sw);

			if (sw->type() == E::SoftwareType::AppDataService)
			{
				appDataServicesIDs.append(sw->equipmentIdTemplate().trimmed());
			}
		}

		//

		XmlWriteHelper xml(m_cfgXml->xmlWriter());

		xml.writeStartElement(XmlElement::APP_DATA_SERVICES);
		xml.writeIntAttribute(XmlAttribute::COUNT, appDataServicesIDs.count());

		QString ids = appDataServicesIDs.join(Separator::SEMICOLON);

		xml.writeStringAttribute(XmlAttribute::ID, ids);

		xml.writeEndElement();

		// adding links to files AppDataSources.xml for each AppDataService;

		for(const QString& id : appDataServicesIDs)
		{
			result &= m_cfgXml->addLinkToFile(id, File::APP_DATA_SOURCES_XML);
		}

		return result;
	}

	bool TestClientCfgGenerator::linkAppSignalsFile()
	{
		bool res = m_cfgXml->addLinkToFile(Directory::COMMON, File::APP_SIGNALS_ASGS);

		if (res == false)
		{
			// Can't link build file %1 into /%2/configuration.xml.
			//
			m_log->errCMN0018(QString("%1\\%2").arg(Directory::COMMON).arg(File::APP_SIGNALS_ASGS), equipmentID());
			return false;
		}

		return true;
	}

	bool TestClientCfgGenerator::writeBatFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForBat();

		content += "TestAppDataSrv.exe";

		QString parameters;
		if (getServiceParameters(parameters) == false)
		{
			return false;
		}
		content += parameters.mid(3);	// Skip -e parameter

		BuildFile* buildFile = m_buildResultWriter->addFile(Directory::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".bat", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}

	bool TestClientCfgGenerator::writeShFile()
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoCommentsForSh();

		content += "./TestAppDataSrv";

		QString parameters;

		if (getServiceParameters(parameters) == false)
		{
			return false;
		}

		content += parameters.mid(3);	// Skip -e parameter

		BuildFile* buildFile = m_buildResultWriter->addFile(Directory::RUN_SERVICE_SCRIPTS, m_software->equipmentIdTemplate().toLower() + ".sh", content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}
}
