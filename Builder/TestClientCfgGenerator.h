#pragma once

#include "SoftwareCfgGenerator.h"

namespace Builder
{
	class TestClientCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		TestClientCfgGenerator(Context* context, Hardware::Software* software);

		bool generateConfiguration() override;

	private:
		bool writeSettings();
		bool writeBatFile();
		bool writeShFile();
	};
}
