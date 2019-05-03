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

	public:
		MetrologyCfgGenerator(Context* context, Hardware::Software* software);
		virtual ~MetrologyCfgGenerator();

		virtual bool generateConfiguration() override;

	private:

		bool writeSettings();
		bool writeMetrologySignalsXml();

		bool testElectricRange_ThermoCouple(const Signal& signal);
		bool testElectricRange_ThermoResistor(const Signal& signal);

	};
}
