#pragma once

#include "SoftwareCfgGenerator.h"
#include "../OnlineLib/SoftwareSettings.h"

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
		bool writeAppDataSourcesXml();
		bool writeAppSignalsXml();
		bool writeAcquiredAppSignalsFile();

		bool writeRunScriptFile(const QString& profile, const AppDataServiceSettings& settings, E::OS os);

		bool findAppDataSourceAcquiredSignals(DataSource& appDataSource);

	private:
		std::set<QString> m_acquiredAppSignals;
	};
}
