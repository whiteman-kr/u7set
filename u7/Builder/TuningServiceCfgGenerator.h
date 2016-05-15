#pragma once

#include "SoftwareCfgGenerator.h"
#include "../TuningService/TuningDataStorage.h"
#include "../TuningService/TuningDataSource.h"

namespace Builder
{

	class TuningServiceCfgGenerator : public SoftwareCfgGenerator
	{
	private:
		Tuning::TuningDataStorage* m_tuningDataStorage = nullptr;
		Hardware::SubsystemStorage* m_subsystems = nullptr;

		HashedVector<QString, Hardware::DeviceModule*> m_tuningLMs;
		QVector<Signal*> m_tuningSignals;

		bool writeSettings();
		bool writeTuningLMs();

	public:
		TuningServiceCfgGenerator(	DbController* db,
									Hardware::SubsystemStorage* subsystems,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									Tuning::TuningDataStorage* tuningDataStorage,
									BuildResultWriter* buildResultWriter);

		~TuningServiceCfgGenerator();

		virtual bool generateConfiguration() override;
	};

}
