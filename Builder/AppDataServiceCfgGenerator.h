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

		static bool writeAppSignalsExtXml(const AppSignalSet* signalSet,
										  const std::set<Hash>* limitedSet,
										  BuildResultWriter* resultWriter,
										  const QString& subDir);
	private:
		bool writeAppDataSourcesXml();
		bool writeAppSignalsXml();
		bool writeAppSignalsExtXml();
		bool writeAcquiredAppSignalsFile();

		bool writeRunScriptFile(const QString& profile, const AppDataServiceSettings& settings, E::OS os);

		bool findAppDataSourceAcquiredSignals(DataSource& appDataSource);

	private:
		std::set<Hash> m_acquiredAppSignals;		// set of calcHash(signal.appSignalID)
	};
}
