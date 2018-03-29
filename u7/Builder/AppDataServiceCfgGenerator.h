#pragma once

#include "SoftwareCfgGenerator.h"
#include "../lib/DeviceHelper.h"
#include "../lib/XmlHelper.h"

class DataSource;

namespace Builder
{
	class AppDataServiceCfgGenerator : public SoftwareCfgGenerator
	{
	public:
		AppDataServiceCfgGenerator(	DbController* db,
									Hardware::SubsystemStorage* subsystems,
									Hardware::Software* software,
									SignalSet* signalSet,
									Hardware::EquipmentSet* equipment,
									const QHash<QString, quint64>& lmUniqueIdMap,
									BuildResultWriter* buildResultWriter);
		~AppDataServiceCfgGenerator();

		virtual bool generateConfiguration() override;

	private:
		const QHash<QString, quint64>& m_lmUniqueIdMap;

		//

		QStringList m_associatedLMs;
		QHash<QString, bool> m_associatedAppSignals;
		Hardware::SubsystemStorage* m_subsystems = nullptr;

		bool getAssociatedLMs();

		bool writeSettings();
		bool writeAppDataSourcesXml();
		bool writeAppSignalsXml();
		bool addLinkToAppSignalsFile();

		bool writeBatFile();
		bool writeShFile();


		bool findAppDataSourceAssociatedSignals(DataSource& appDataSource);
	};
}
