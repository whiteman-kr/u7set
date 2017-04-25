#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"

class DataSource;

namespace Builder
{
	class AppDataServiceCfgGenerator : public SoftwareCfgGenerator
	{
	private:
		QStringList m_associatedLMs;
		QHash<QString, bool> m_associatedAppSignals;
		Hardware::SubsystemStorage* m_subsystems = nullptr;

		bool getAssociatedLMs();

		bool writeSettings();
		bool writeAppSignalsXml();
		bool writeAppSignalsProtofile();
		bool writeEquipmentXml();
		bool writeAppDataSourcesXml();

		bool findAppDataSourceAssociatedSignals(DataSource& appDataSource);

	public:
		AppDataServiceCfgGenerator(	DbController* db,
									Hardware::SubsystemStorage* subsystems,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									BuildResultWriter* buildResultWriter);
		~AppDataServiceCfgGenerator();

		virtual bool generateConfiguration() override;
	};
}
