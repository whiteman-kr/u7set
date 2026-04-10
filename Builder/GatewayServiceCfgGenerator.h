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
		bool ivsImpulseGatewayProcessing(const Gateway::GatewayShared& gw);
		bool modbusSlaveGatewayProcessing(const Gateway::GatewayShared& gw);
		bool adsGatewayProcessing(const Gateway::GatewayShared& gw);
		bool tuningGatewayProcessing(const Gateway::GatewayShared& gw);

		bool checkConnection(const Gateway::GatewayShared& gw, E::SoftwareType swType);

		bool checkStrLen(const QString& appSignalID, const QString& str, size_t len, const QString& propName);

	private:
		Gateway::GatewaysShared m_gateways;
		Gateway::ParserShared m_parser;
	};
}
