#ifndef TUNINGCLIENTCFGGENERATOR_H
#define TUNINGCLIENTCFGGENERATOR_H

#include "SoftwareCfgGenerator.h"

#include "../lib/Tuning/TuningFilter.h"
#include "../OnlineLib/SoftwareSettings.h"

#include <ClientLib/TuningSignalManager.h>

namespace Builder
{

	class TuningClientCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		TuningClientCfgGenerator(Context* context, Hardware::Software* software);

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

		static bool createTuningSignals(const QStringList& equipmentList, const SignalSet* signalSet, ::Proto::AppSignalSet* tuningSet);

	private:
		SubsystemStorage* m_subsystems = nullptr;

		bool createEquipmentList(QStringList* equipmentList);
		bool createObjectFilters(const ClientLib::TuningSignalManager& tuningSignalManager, const QStringList& equipmentList);
		bool createEquipmentAndSchemaFilters(const QStringList& equipmentList,const ClientLib::TuningSignalManager& tuningSignalManager);
		void createCounterFiltersFromTemplates();

		bool writeTuningSignals();
		bool writeObjectFilters();
		bool writeTuningSchemas();
		bool writeGlobalScript();
		bool writeTuningClientBehavior();

	private:
		::Proto::AppSignalSet m_tuningSet;

		TuningFilterStorage m_tuningFilterStorage;
	};
}

#endif // TUNINGCLIENTCFGGENERATOR_H
