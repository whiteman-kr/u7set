#pragma once

#include "SoftwareCfgGenerator.h"
#include "../include/DeviceHelper.h"
#include "../include/XmlHelper.h"

namespace Builder
{
	class DASCfgGenerator : public SoftwareCfgGenerator
	{
	private:
		bool writeSettings();
		bool writeAppSignalsXml();
		bool writeEquipmentXml();
		bool writeDataSourcesXml();

	public:
		DASCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter);
		~DASCfgGenerator();

		virtual bool generateConfiguration() override;
	};
}
