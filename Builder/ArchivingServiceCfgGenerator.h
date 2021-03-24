#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/SoftwareSettings.h"

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

		bool writeBatFile();
		bool writeShFile();
	};

}
