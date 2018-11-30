#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"
#include "../lib/DataSource.h"

namespace Builder
{
	class AppDataServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		AppDataServiceCfgGenerator(	DbController* db,
									const Hardware::SubsystemStorage* subsystems,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									const QHash<QString, quint64>& lmUniqueIdMap,
									BuildResultWriter* buildResultWriter);
		~AppDataServiceCfgGenerator();

		virtual bool generateConfiguration() override;

	private:
		bool getAssociatedLMs();

		bool writeSettings();
		bool writeAppDataSourcesXml();
		bool writeAppSignalsXml();
		bool addLinkToAppSignalsFile();

		bool writeBatFile();
		bool writeShFile();

		bool findAppDataSourceAssociatedSignals(DataSource& appDataSource);

	private:
		const QHash<QString, quint64>& m_lmUniqueIdMap;
		SubsystemKeyMap m_subsystemKeyMap;

		//

		QStringList m_associatedLMs;
		QHash<QString, bool> m_associatedAppSignals;
	};
}
