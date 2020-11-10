#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/ServiceSettings.h"

namespace Builder
{

	class ArchivingServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		ArchivingServiceCfgGenerator(Context* context, Hardware::Software* software);

		~ArchivingServiceCfgGenerator();

		virtual bool generateConfiguration() override;
		virtual bool getSettingsXml(QXmlStreamWriter& xmlWriter) override;

	private:
		bool writeSettings();
		bool writeArchSignalsFile();

		bool writeBatFile();
		bool writeShFile();

	private:
		ArchivingServiceSettings m_settings;
	};

}
