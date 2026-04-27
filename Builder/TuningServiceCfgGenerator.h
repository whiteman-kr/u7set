#pragma once

#include "SoftwareCfgGenerator.h"
#include "../OnlineLib/SoftwareSettings.h"
#include "../AppSignalLib/TuningDataStorage.h"

namespace Builder
{
	class Context;

	class TuningServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		TuningServiceCfgGenerator(Context* context,
								  Hardware::Software* software);

		~TuningServiceCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	private:
		bool writeTuningSourcesXml();

		bool writeRunScriptFile(const QString& profile, const TuningServiceSettings& settings, E::OS os);

	private:
		Tuning::TuningDataStorage* m_tuningDataStorage = nullptr;
	};

}
