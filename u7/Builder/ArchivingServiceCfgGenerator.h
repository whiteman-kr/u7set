#pragma once

#include "SoftwareCfgGenerator.h"

namespace Builder
{

	class ArchivingServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		ArchivingServiceCfgGenerator(DbController* db,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									BuildResultWriter* buildResultWriter);

		~ArchivingServiceCfgGenerator();

		virtual bool generateConfiguration() override;

	private:
		bool writeSettings();
		bool writeArchSignalsFile();

		bool writeBatFile();
		bool writeShFile();
	};

}
