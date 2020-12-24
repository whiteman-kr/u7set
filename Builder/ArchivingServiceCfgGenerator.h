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

		virtual bool generateConfiguration() override;

	private:
		bool writeArchSignalsFile();

		bool writeBatFile();
		bool writeShFile();
	};

}
