#pragma once

#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/AcquiredDiagSignal.h"
#include "../UtilsLib/XmlHelper.h"
#include "SoftwareCfgGenerator.h"
#include "DeviceHelper.h"

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

		bool appendAquiredDiagSignalsToDataSource(const Hardware::DeviceModule* lm, DataSource* ds);
		bool findAcquiredDiagSignals(const Hardware::DeviceModule* lm);
		bool findAcquiredParentObjects();

	private:
		std::map<Hash, std::vector<DiagSignalConstShared>> m_lmAcquiredDiagSignals;	// calcHash(LM equipmetID) => diagSignals array
		std::map<Hash, int> m_moduleDiagDataOffset;									// calcHash(Module equipmetID) => module diag data offset in FODIP

		Network::AcquiredDiagSignals m_protoAcquiredDiagSignals;
	};
}
