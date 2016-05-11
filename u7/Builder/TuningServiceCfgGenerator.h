#pragma once

#include "SoftwareCfgGenerator.h"
#include "TuningDataStorage.h"
#include "../TuningService/TuningDataSource.h"

namespace Builder
{

	class TuningServiceCfgGenerator : public SoftwareCfgGenerator
	{
	private:
		TuningDataStorage* m_tuningDataStorage = nullptr;

		HashedVector<QString, Hardware::DeviceModule*> m_tuningLMs;
		QVector<Signal*> m_tuningSignals;

		bool writeSettings();
		bool writeTuningLMs();

	public:
		TuningServiceCfgGenerator(	DbController* db,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									TuningDataStorage* tuningDataStorage,
									BuildResultWriter* buildResultWriter);

		~TuningServiceCfgGenerator();

		virtual bool generateConfiguration() override;
	};

}
