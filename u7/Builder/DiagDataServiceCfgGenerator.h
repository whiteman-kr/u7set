#pragma once

#include "SoftwareCfgGenerator.h"
#include "../include/DeviceHelper.h"
#include "../include/XmlHelper.h"

namespace Builder
{
	class DiagDataServiceCfgGenerator : public SoftwareCfgGenerator
	{
	private:

	public:
		DiagDataServiceCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter);
		~DiagDataServiceCfgGenerator();

		virtual bool generateConfiguration() override;
	};
}
