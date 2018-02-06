#pragma once

#include "SoftwareCfgGenerator.h"

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
		bool writeBatFile();
		bool writeShFile();

	};

}
