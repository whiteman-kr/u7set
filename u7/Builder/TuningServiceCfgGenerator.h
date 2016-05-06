#pragma once

#include "SoftwareCfgGenerator.h"

namespace Builder
{

	class TuningServiceCfgGenerator : public SoftwareCfgGenerator
	{
	private:
		HashedVector<QString, Hardware::DeviceModule*> m_tuningLMs;
		QVector<Signal*> m_tuningSignals;

		bool findTuningSignals();			// builds m_tuningSignals

		bool findAssociatedTuningSignals(DataSource& tuningSource);

		bool writeSettings();
		bool writeTuningSignals();
		bool writeTuningLMs();

	public:
		TuningServiceCfgGenerator(	DbController* db,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									BuildResultWriter* buildResultWriter);

		~TuningServiceCfgGenerator();

		virtual bool generateConfiguration() override;
	};

}
