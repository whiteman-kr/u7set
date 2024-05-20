#pragma once

#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/AcquiredDiagSignal.h"
#include "SoftwareCfgGenerator.h"

namespace OnlineLib
{
	class DataSource;
}

namespace Builder
{
	class DiagDataServiceCfgGenerator : public SoftwareCfgGenerator
	{
	private:

	public:
		DiagDataServiceCfgGenerator(Context* context, Hardware::Software* software);
		~DiagDataServiceCfgGenerator();

		virtual bool createSettingsProfile(const QString& profile) override;
		virtual bool generateConfigurationStep1() override;

	private:
		bool writeRunScriptFile(const QString& profile, const DiagDataServiceSettings& settings, E::OS os);

		bool writeDiagSignalTypesXml();
		bool writeDiagDataSourcesXml();
		bool writeAcquiredDiagSignalsFile();

		bool appendAquiredDiagSignalsToDataSource(const Hardware::DeviceModule* lm, OnlineLib::DataSource* ds);
		bool findAcquiredDiagSignals(const Hardware::DeviceModule* lm);
		bool findAcquiredParentObjects();

	private:
		std::map<QString, std::vector<DiagSignalConstShared>> m_lmAcquiredDiagSignals;	// LM equipmetID => diagSignals array
		//std::map<Hash, int> m_moduleDiagDataOffset;									// calcHash(Module equipmetID) => module diag data offset in FODIP
		std::map<QString, int> m_moduleDiagDataOffset;									// calcHash(Module equipmetID) => module diag data offset in FODIP

		Network::AcquiredDiagSignalsAndObjects m_protoAcquiredDiagSignalsAndObjects;
	};
}
