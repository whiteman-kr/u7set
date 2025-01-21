#pragma once

#include "SoftwareCfgGenerator.h"

#include "../OnlineLib/SoftwareSettings.h"

namespace Builder
{
	class GatewayServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		GatewayServiceCfgGenerator(Context* context, Hardware::Software* software);

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	private:
		bool writeRunScriptFile(const QString& profile,
								const GatewayServiceSettings& settings,
								E::OS os);
	};
}
