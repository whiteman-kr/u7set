#pragma once

#include "../include/DbController.h"
#include "../include/Signal.h"
#include "../include/DeviceObject.h"
#include "../Builder/BuildResultWriter.h"

namespace Builder
{
	class SoftwareCfgGenerator : public QObject
	{
		Q_OBJECT

	private:
		DbController* m_dbController = nullptr;
		Hardware::Software* m_software = nullptr;
		SignalSet* m_signalSet = nullptr;
		Hardware::EquipmentSet* m_equipment = nullptr;
		BuildResultWriter* m_buildResultWriter = nullptr;
		OutputLog* m_log = nullptr;
		ConfigurationXmlFile * m_cfgXml = nullptr;
		QString m_subDir;

		bool generateMonitorCfg();
		bool writeMonitorSettings();

		void writeErrorSection(QXmlStreamWriter& xmlWriter, QString error);

		bool writeAppSignalsXml();
		bool writeEquipmentXml();

		bool generateDataAcqisitionServiceCfg();

	public:
		SoftwareCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter);

		bool run();
	};
}
