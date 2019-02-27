#pragma once

#include "SoftwareCfgGenerator.h"

namespace Builder
{

	class ArchivingServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		ArchivingServiceCfgGenerator(Context* context, Hardware::Software* software);

		~ArchivingServiceCfgGenerator();

		virtual bool generateConfiguration() override;

	private:
		bool writeSettings();
		bool writeArchSignalsFile();

		bool writeBatFile();
		bool writeShFile();
	};

}
