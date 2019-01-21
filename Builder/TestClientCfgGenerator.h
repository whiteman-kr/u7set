#pragma once

#include "SoftwareCfgGenerator.h"

namespace Builder
{
	class TestClientCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		TestClientCfgGenerator(DbController* db,
							   Hardware::Software* software,
							   SignalSet* signalSet,
							   Hardware::EquipmentSet* equipment,
							   BuildResultWriter* buildResultWriter);

		bool generateConfiguration() override;

	private:
		bool writeSettings();
		bool writeBatFile();
		bool writeShFile();
	};
}
