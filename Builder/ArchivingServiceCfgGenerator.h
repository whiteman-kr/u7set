#pragma once

#include "SoftwareCfgGenerator.h"

namespace Builder
{
	class ArchivingServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		ArchivingServiceCfgGenerator(Context* context, Hardware::Software* software);

		~ArchivingServiceCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	private:
		bool writeArchSignalsFile();

		bool writeRunScriptFile(const QString& profile, const ArchivingServiceSettings& settings, E::OS os);
	};
}
