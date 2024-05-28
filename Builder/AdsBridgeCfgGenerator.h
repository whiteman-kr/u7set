#pragma once

#include "SoftwareCfgGenerator.h"

namespace Builder
{
	class Context;

	class AdsBridgeCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		AdsBridgeCfgGenerator(Context* context, Hardware::Software* software);

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;
		virtual bool generateConfigurationStep2() override;
	};
} // namespace Builder