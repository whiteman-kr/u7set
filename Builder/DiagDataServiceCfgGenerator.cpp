#include "DiagDataServiceCfgGenerator.h"
#include "../lib/SoftwareSettings.h"


namespace Builder
{
	DiagDataServiceCfgGenerator::DiagDataServiceCfgGenerator(Context* context, Hardware::Software* software) :
		SoftwareCfgGenerator(context, software)
	{
	}

	DiagDataServiceCfgGenerator::~DiagDataServiceCfgGenerator()
	{
	}

	bool DiagDataServiceCfgGenerator::createSettingsProfile(const QString& profile)
	{
		DiagDataServiceSettingsGetter settingsGetter;

		if (settingsGetter.readFromDevice(m_context, m_software) == false)
		{
			return false;
		}

		return m_settingsSet.addProfile<DiagDataServiceSettings>(profile, settingsGetter);
	}

	bool DiagDataServiceCfgGenerator::generateConfiguration()
	{
		return true;
	}
}
