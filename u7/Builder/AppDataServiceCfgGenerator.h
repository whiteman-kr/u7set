#pragma once

#include "SoftwareCfgGenerator.h"
#include "../include/DeviceHelper.h"
#include "../include/XmlHelper.h"

namespace Builder
{
	class AppDataServiceCfgGenerator : public SoftwareCfgGenerator
	{
	private:
		QStringList m_associatedLMs;
		QHash<QString, bool> m_associatedAppSignals;
		//QHash<QString, int>

		bool getAssociatedLMs();

		bool writeSettings();
		bool writeAppSignalsXml();
		bool writeAppSignalsProtofile();
		bool writeEquipmentXml();
		bool writeAppDataSourcesXml();

		bool findAppDataSourceAssociatedSignals(DataSource& appDataSource);

	public:
		AppDataServiceCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter);
		~AppDataServiceCfgGenerator();

		virtual bool generateConfiguration() override;
	};
}
