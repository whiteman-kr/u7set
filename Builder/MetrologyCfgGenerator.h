#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"

namespace Builder
{
	class MetrologyCfgGenerator : public SoftwareCfgGenerator
	{
	private:
		Hardware::SubsystemStorage* m_subsystems = nullptr;

		bool writeSettings();
		bool writeMetrologySignalsXml();

	public:
		MetrologyCfgGenerator(Context* context, Hardware::Software* software);
		virtual ~MetrologyCfgGenerator();

		virtual bool generateConfiguration() override;
	};
}
