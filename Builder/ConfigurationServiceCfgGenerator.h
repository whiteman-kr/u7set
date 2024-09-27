#pragma once

#include "SoftwareCfgGenerator.h"

namespace Builder
{

	class ConfigurationServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		ConfigurationServiceCfgGenerator(Context* context, Hardware::Software* software);

		~ConfigurationServiceCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	private:
		bool writeRunScriptFile(const QString& profile, const CfgServiceSettings& settings, E::OS os);

		QString getCommandLine(const QString& profile, E::OS os, bool simulation) const;
	};

}
