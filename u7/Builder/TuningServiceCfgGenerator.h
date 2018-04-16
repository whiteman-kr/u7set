#pragma once

#include "SoftwareCfgGenerator.h"
#include "../TuningService/TuningDataStorage.h"
#include "../TuningService/TuningSource.h"
#include "Builder.h"

namespace Builder
{
	class TuningServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		TuningServiceCfgGenerator(DbController* db,
									const Hardware::SubsystemStorage* subsystems,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									Tuning::TuningDataStorage* tuningDataStorage,
									const LmsUniqueIdMap& lmsUniqueIdMap,
									BuildResultWriter* buildResultWriter);

		~TuningServiceCfgGenerator();

		virtual bool generateConfiguration() override;

	private:
		const LmsUniqueIdMap m_lmsUniqueIdMap;
		SubsystemKeyMap m_subsystemKeyMap;

		Tuning::TuningDataStorage* m_tuningDataStorage = nullptr;

		QVector<Signal*> m_tuningSignals;

		bool writeSettings();
		bool writeTuningSources();

		bool writeBatFile();
		bool writeShFile();
	};

}
