#pragma once

#include "SoftwareCfgGenerator.h"
#include "../TuningService/TuningDataStorage.h"
#include "../TuningService/TuningSource.h"
#include "Builder.h"

namespace Builder
{
	class TuningServiceCfgGenerator : public SoftwareCfgGenerator
	{
	private:
		Tuning::TuningDataStorage* m_tuningDataStorage = nullptr;
		Hardware::SubsystemStorage* m_subsystems = nullptr;

		HashedVector<QString, Hardware::DeviceModule*> m_tuningLMs;
		QVector<Signal*> m_tuningSignals;

		LmsUniqueIdMap m_lmsUniqueIdMap;

		bool writeSettings();
		bool writeTuningLMs();
		bool writeBatFile();

	public:
		TuningServiceCfgGenerator(	DbController* db,
									Hardware::SubsystemStorage* subsystems,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									Tuning::TuningDataStorage* tuningDataStorage,
									const LmsUniqueIdMap& lmsUniqueIdMap,
									BuildResultWriter* buildResultWriter);

		~TuningServiceCfgGenerator();

		virtual bool generateConfiguration() override;
	};

}
