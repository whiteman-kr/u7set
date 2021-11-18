#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/SoftwareSettings.h"

namespace Builder
{
	class AppDataServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		AppDataServiceCfgGenerator(Context* context,
								   Hardware::Software* software);
		~AppDataServiceCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	private:
		bool getAssociatedLMs();

		bool writeAppDataSourcesXml();
		bool writeAppSignalsXml();
		bool addLinkToAppSignalsFile();

		bool writeRunScriptFile(const QString& profile, const AppDataServiceSettings& settings, E::OS os);

		bool findAppDataSourceAssociatedSignals(DataSource& appDataSource);

	private:
		std::set<QString> m_associatedAppSignals;
	};
}
