#pragma once

#include "SoftwareCfgGenerator.h"

namespace Builder
{
	class DASCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		DASCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter);
		~DASCfgGenerator();

		virtual bool generateConfiguration();
	};
}
