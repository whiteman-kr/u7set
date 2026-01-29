#pragma once

#include "SoftwareCfgGenerator.h"
#include "GatewayDescriptionParser.h"

#include "../OnlineLib/SoftwareSettings.h"

namespace Builder
{
	class GatewayServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		GatewayServiceCfgGenerator(Context* context, Hardware::Software* software);

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;
		virtual bool generateConfigurationStep2() override;

	private:
		bool writeRunScriptFile(const QString& profile,
								const GatewayServiceSettings& settings,
								E::OS os);

		bool doGatewaySpecificProcessing(const Gateway::GatewayShared& gw);
		bool adsGatewayProcessing(const Gateway::GatewayShared& gw);

	private:
		Gateway::GatewaysShared m_gateways;
		Gateway::ParserShared m_parser;
	};
}
