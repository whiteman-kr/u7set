#include "AdsBridgeCfgGenerator.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "Context.h"
#include "SoftwareSettingsGetter.h"

namespace Builder
{
	AdsBridgeCfgGenerator::AdsBridgeCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator{context, software}
	{
	}

	bool AdsBridgeCfgGenerator::createSettingsProfile(const QString& profile)
	{
		AdsBridgeSettingsGetter settingsGetter;

		if (settingsGetter.readSoftwareSettings(m_context, m_software) == false)
		{
			return false;
		}

		return m_settingsSet.addProfile<AdsBridgeSettings>(profile, settingsGetter);
	}

	bool AdsBridgeCfgGenerator::generateConfigurationStep1()
	{
		if (m_software == nullptr || m_software->softwareType() != E::SoftwareType::AdsBridge || m_equipment == nullptr ||
			m_cfgXml == nullptr || m_buildResultWriter == nullptr)
		{
			Q_ASSERT(m_software && m_software->softwareType() == E::SoftwareType::AdsBridge);
			Q_ASSERT(m_equipment);
			Q_ASSERT(m_cfgXml);
			Q_ASSERT(m_buildResultWriter);
			return false;
		}

		// Add links to schema files (previously written) via m_cfgXml->addLinkToFile(...)
		//
		auto settings = m_settingsSet.getSettingsDefaultProfile<AdsBridgeSettings>();

		TEST_PTR_LOG_RETURN_FALSE(settings, m_log);

		return true;
	}

	bool AdsBridgeCfgGenerator::generateConfigurationStep2()
	{
		if (bool ok = SoftwareCfgGenerator::generateConfigurationStep2(); ok == false)
		{
			return false;
		}

		return true;
	}
} // namespace Builder
