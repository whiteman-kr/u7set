#include "GatewayServiceCfgGenerator.h"
#include "SoftwareSettingsGetter.h"
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

		return m_settingsSet.addProfile<GatewayServiceSettings>(profile, settingsGetter);
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
			assert(m_software->softwareType() == E::SoftwareType::Monitor);
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

		std::shared_ptr<const GatewayServiceSettings> settings = m_settingsSet.getSettingsDefaultProfile<GatewayServiceSettings>();

		bool result = true;

		return result;
	}


}
