#pragma once

#include "SoftwareCfgGenerator.h"

namespace Builder
{

	class TuningServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		TuningServiceCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter);
		~TuningServiceCfgGenerator();

		virtual bool generateConfiguration() override;
	};

}
