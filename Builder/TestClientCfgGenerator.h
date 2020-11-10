#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/ServiceSettings.h"

namespace Builder
{
	class TestClientCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		TestClientCfgGenerator(Context* context, Hardware::Software* software);

		virtual bool generateConfiguration() override;
		virtual bool getSettingsXml(QXmlStreamWriter& xmlWriter) override;

	private:
		bool writeSettings();
		bool linkAppSignalsFile();
		bool writeBatFile();
		bool writeShFile();

	private:
		TestClientSettings m_settings;
	};
}
