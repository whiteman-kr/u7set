#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/ServiceSettings.h"
#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"

namespace Builder
{
	class DiagDataServiceCfgGenerator : public SoftwareCfgGenerator
	{
	private:

	public:
		DiagDataServiceCfgGenerator(Context* context, Hardware::Software* software);
		~DiagDataServiceCfgGenerator();

		virtual bool generateConfiguration() override;

	private:
		bool writeSettings();

	private:
		DiagDataServiceSettings m_settings;
	};
}
