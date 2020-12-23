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

		virtual bool generateConfiguration() override;

	private:
		bool writeSettings();
		bool writeBatFile();
		bool writeShFile();

		bool buildClientsList(CfgServiceSettingsGetter *settings);

	private:
		CfgServiceSettingsGetter m_settings;
	};

}
