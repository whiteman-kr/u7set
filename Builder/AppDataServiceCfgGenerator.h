#pragma once

#include "SoftwareCfgGenerator.h"

namespace OnlineLib
{
	class AppDataServiceSettings;
	class DataSource;
}

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

		static bool writeAppSignalsExtXml(	const Context* context,
											const AppSignalSet* signalSet,
											const std::set<Hash>* limitedSet,
											const QString& subDir);
	private:
		bool writeAppDataSourcesXml();
		bool writeAppSignalsXml();
		bool writeAppSignalsExtXml();
		bool writeAcquiredAppSignalsFile();

		bool writeRunScriptFile(const QString& profile, const AppDataServiceSettings& settings, E::OS os);

		bool findAppDataSourceAcquiredSignals(OnlineLib::DataSource& appDataSource);

	private:
		std::set<Hash> m_acquiredAppSignals;		// set of calcHash(signal.appSignalID)
	};
}
