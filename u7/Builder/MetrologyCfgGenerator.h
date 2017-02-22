#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"

namespace Builder
{
    class MetrologyCfgGenerator : public SoftwareCfgGenerator
	{
	private:
		QStringList m_associatedLMs;
		QHash<QString, bool> m_associatedAppSignals;
		Hardware::SubsystemStorage* m_subsystems = nullptr;

		bool getAssociatedLMs();

		bool writeSettings();
		bool writeAppSignalsXml();

		bool findAppDataSourceAssociatedSignals(DataSource& appDataSource);

	public:
        MetrologyCfgGenerator(	DbController* db,
									Hardware::SubsystemStorage* subsystems,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									BuildResultWriter* buildResultWriter);
        ~MetrologyCfgGenerator();

		virtual bool generateConfiguration() override;
	};
}
