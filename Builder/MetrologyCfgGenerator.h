#pragma once

#include "SoftwareCfgGenerator.h"

#include "../lib/DeviceHelper.h"
#include "../UtilsLib/XmlHelper.h"
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

		bool testElectricLimit(const AppSignal& signal, double lowLimit, double highLimit);
		bool testEngineeringLimit(const AppSignal& signal, double lowLimit, double highLimit);

		bool testElectricLimit_Input_mA(const AppSignal& signal);
		bool testElectricLimit_Input_mV(const AppSignal& signal);
		bool testElectricLimit_Input_Ohm(const AppSignal& signal);
		bool testElectricLimit_Input_V(const AppSignal& signal);
		bool testElectricLimit_Input_uA(const AppSignal& signal);
		bool testElectricLimit_Input_Hz(const AppSignal& signal);

	private:
		Hardware::SubsystemStorage* m_subsystems = nullptr;
		std::unordered_set<QString> m_analogSignalsOnSchemas;
	};
}
