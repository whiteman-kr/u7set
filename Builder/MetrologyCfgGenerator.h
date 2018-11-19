#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"

namespace Builder
{
	class MetrologyCfgGenerator : public SoftwareCfgGenerator
	{
	private:
		Hardware::SubsystemStorage* m_subsystems = nullptr;

		bool writeSettings();
		bool writeMetrologySignalsXml();

	public:
		MetrologyCfgGenerator(		DbController* db,
									Hardware::SubsystemStorage* subsystems,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									BuildResultWriter* buildResultWriter);
		virtual ~MetrologyCfgGenerator();

		virtual bool generateConfiguration() override;
	};
}
