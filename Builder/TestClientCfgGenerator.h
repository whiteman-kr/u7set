#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/SoftwareSettings.h"

namespace Builder
{
	class TestClientCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		TestClientCfgGenerator(Context* context, Hardware::Software* software);

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;
		virtual bool generateConfigurationStep2() override;

	private:
		bool linkAppSignalsFile();
		bool writeBatFile();
		bool writeShFile();
	};
}
