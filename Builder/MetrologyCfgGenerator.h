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
		std::unordered_set<QString> m_analogSignalsOnSchemas;

	public:
		MetrologyCfgGenerator(Context* context, Hardware::Software* software);
		virtual ~MetrologyCfgGenerator();

		virtual bool generateConfiguration() override;

	private:

		bool writeSettings();
		bool writeMetrologyItemsXml();
		bool writeMetrologySignalSet();

		bool testElectricLimit(const Signal& signal, double lowLimit, double highLimit);
		bool testEngineeringLimit(const Signal& signal, double lowLimit, double highLimit);

		bool testElectricLimit_Input_V(const Signal& signal);
		bool testElectricLimit_Input_mA(const Signal& signal);
		bool testElectricLimit_Input_mV(const Signal& signal);
		bool testElectricLimit_Input_Ohm(const Signal& signal);

	};
}
