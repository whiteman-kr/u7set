#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/DeviceHelper.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../UtilsLib/XmlHelper.h"

namespace Builder
{
	class DiagDataServiceCfgGenerator : public SoftwareCfgGenerator
	{
	private:

	public:
		DiagDataServiceCfgGenerator(Context* context, Hardware::Software* software);
		~DiagDataServiceCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	private:
		bool writeRunScriptFile(const QString& profile, const DiagDataServiceSettings& settings, E::OS os);
	};
}
