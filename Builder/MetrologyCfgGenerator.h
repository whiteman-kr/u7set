#pragma once

#include "SoftwareCfgGenerator.h"

#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"
#include "../lib/SoftwareSettings.h"

namespace Builder
{

	class MetrologyCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		MetrologyCfgGenerator(Context* context, Hardware::Software* software);
		virtual ~MetrologyCfgGenerator() override;

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	private:
		bool writeDatabaseInfo();
		bool writeMetrologyItemsXml();
		bool writeMetrologySignalSet();

		bool testElectricLimit(const Signal& signal, double lowLimit, double highLimit);
		bool testEngineeringLimit(const Signal& signal, double lowLimit, double highLimit);

		bool testElectricLimit_Input_mA(const Signal& signal);
		bool testElectricLimit_Input_mV(const Signal& signal);
		bool testElectricLimit_Input_Ohm(const Signal& signal);
		bool testElectricLimit_Input_V(const Signal& signal);
		bool testElectricLimit_Input_uA(const Signal& signal);
		bool testElectricLimit_Input_Hz(const Signal& signal);

	private:
		Hardware::SubsystemStorage* m_subsystems = nullptr;
		std::unordered_set<QString> m_analogSignalsOnSchemas;
	};
}
