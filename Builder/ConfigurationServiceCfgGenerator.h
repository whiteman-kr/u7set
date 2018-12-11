#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/ServiceSettings.h"

namespace Builder
{

	class ConfigurationServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		ConfigurationServiceCfgGenerator(DbController* db,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									BuildResultWriter* buildResultWriter);

		~ConfigurationServiceCfgGenerator();

		virtual bool generateConfiguration() override;

	private:
		bool writeSettings();
		bool writeBatFile();
		bool writeShFile();

		bool buildClientsList(CfgServiceSettings* settings);
	};

}
