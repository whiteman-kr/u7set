#include "GatewayServiceCfgGenerator.h"
#include "SoftwareSettingsGetter.h"
#include "../lib/GatewayDescription.h"
#include "../OnlineLib/SoftwareSettings.h"

namespace Builder
{

	GatewayServiceCfgGenerator::GatewayServiceCfgGenerator(Context* context, Hardware::Software* software)	:
		SoftwareCfgGenerator(context, software)
	{
	}

	bool GatewayServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		GatewayServiceSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		bool result = true;

		result &= m_settingsSet.addProfile<GatewayServiceSettings>(profile, settingsGetter);

		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Windows);
		result &= writeRunScriptFile(profile, settingsGetter, E::OS::Linux);

		return result;
	}

	bool GatewayServiceCfgGenerator::generateConfigurationStep1()
	{
		if (m_software == nullptr ||
			m_software->softwareType() != E::SoftwareType::GatewayService ||
			m_equipment == nullptr ||
			m_cfgXml == nullptr ||
			m_buildResultWriter == nullptr)
		{
			assert(m_software);
			assert(m_software->softwareType() == E::SoftwareType::GatewayService);
			assert(m_equipment);
			assert(m_cfgXml);
			assert(m_buildResultWriter);
			return false;
		}

		IssueLogger* log = m_buildResultWriter->log();

		if (log == nullptr)
		{
			assert(log);
			return false;
		}

		bool result = true;

		std::shared_ptr<const GatewayServiceSettings> settings = m_settingsSet.getSettingsDefaultProfile<GatewayServiceSettings>();

		GatewayDescriptionParser gdp;

		GwParserLog parserLog;

		result = gdp.parse(settings->gatewayDescription, &parserLog);

		return result;
	}

	bool GatewayServiceCfgGenerator::writeRunScriptFile(const QString& profile,
														const GatewayServiceSettings& settings,
														E::OS os)
	{
		TEST_PTR_RETURN_FALSE(m_software);

		QString content = getBuildInfoComments(os);

		QString cmdLine = getCommonCmdLine(settings.cfgService1.address,
										   settings.cfgService2.address, os, true);

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
