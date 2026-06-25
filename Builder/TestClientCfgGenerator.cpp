#include "TestClientCfgGenerator.h"
#include "SoftwareSettingsGetter.h"

#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/XmlHelper.h"

namespace Builder
{
	TestClientCfgGenerator::TestClientCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
	}

	bool TestClientCfgGenerator::createSettingsProfile(const QString& profile)
	{
		TestClientSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		bool result = m_settingsSet.addProfile<TestClientSettings>(profile, settingsGetter);

		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Windows);
		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Linux);

		return result;
	}

	bool TestClientCfgGenerator::generateConfigurationStep1()
	{
		bool result = false;

		do
		{
			if (linkAppSignalsFile() == false) break;

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

			if (sw->softwareType() == E::SoftwareType::AppDataService)
			{
				appDataServicesIDs.append(sw->equipmentIdTemplate().trimmed());
			}
		}

		//

		XmlWriteHelper xml(m_cfgXml->xmlWriter());

		xml.writeStartElement(XmlElement::APP_DATA_SERVICES);
		xml.writeInt32Attribute(XmlAttribute::COUNT, static_cast<int>(appDataServicesIDs.count()));

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

	bool TestClientCfgGenerator::writeRunScriptFile(const QString& profile,
													const TestClientSettings& settings,
													E::OS os)
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoComments(os);

		QString cmdLine = getCommonCmdLine(settings.cfgService1_clientRequestIP,
											settings.cfgService1_clientRequestIP,
											os, true);

		if (cmdLine.isEmpty() == true)
		{
			return false;
		}

		content += cmdLine;

		BuildFile* buildFile = m_buildResultWriter->addFile(getRunScriptDirectory(os),
															getRunScriptName(profile, os),
															content);

		TEST_PTR_RETURN_FALSE(buildFile);

		return true;
	}
}
