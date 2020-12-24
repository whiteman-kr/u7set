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
		bool writeBatFile();
		bool writeShFile();
	};

}
