#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/SoftwareSettings.h"
#include "../TuningService/TuningDataStorage.h"
#include "../TuningService/TuningSource.h"
#include "Builder.h"

namespace Builder
{
	class Context;

	class TuningServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		TuningServiceCfgGenerator(Context* context,
								  Hardware::Software* software);

		~TuningServiceCfgGenerator();

		virtual bool generateConfiguration() override;

	private:
		bool writeTuningSources();

		bool writeBatFile();
		bool writeShFile();

	private:
		Tuning::TuningDataStorage* m_tuningDataStorage = nullptr;

		QVector<Signal*> m_tuningSignals;
	};

}
