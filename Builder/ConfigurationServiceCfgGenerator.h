#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/SoftwareSettings.h"

namespace Builder
{

	class ConfigurationServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		ConfigurationServiceCfgGenerator(Context* context, Hardware::Software* software);

		~ConfigurationServiceCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	private:
		bool writeBatFile();
		bool writeShFile();
	};

}
