#pragma once

#include "../include/DbController.h"
#include "../include/Signal.h"
#include "../include/DeviceObject.h"
#include "../Builder/BuildResultWriter.h"
#include "IssueLogger.h"

namespace Builder
{
	class SoftwareCfgGenerator : public QObject
	{
		Q_OBJECT

	protected:
		DbController* m_dbController = nullptr;
		Hardware::Software* m_software = nullptr;
		SignalSet* m_signalSet = nullptr;
		Hardware::EquipmentSet* m_equipment = nullptr;
		BuildResultWriter* m_buildResultWriter = nullptr;
		IssueLogger* m_log = nullptr;
		ConfigurationXmlFile* m_cfgXml = nullptr;
		QString m_subDir;

		static QList<Hardware::DeviceModule*> m_lmList;

		struct SchemaFile
		{
			QString id;
			QString subDir;
			QString fileName;
			QString group;
		};
		static QList<SchemaFile> m_schemaFileList;

		Hardware::DeviceRoot* m_deviceRoot = nullptr;

		static bool buildLmList(Hardware::EquipmentSet *equipment, IssueLogger* log);

	public:
		SoftwareCfgGenerator(DbController* db, Hardware::Software* software, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter);

		bool run();

		static bool generalSoftwareCfgGeneration(DbController* db, SignalSet* signalSet, Hardware::EquipmentSet* equipment, BuildResultWriter* buildResultWriter);
		static bool writeSchemas(DbController* db, BuildResultWriter* buildResultWriter, IssueLogger* log);
		static bool writeSchemasList(DbController* db,
									 BuildResultWriter* buildResultWriter,
									 int parentFileId,
									 QString fileExtension,
									 QString subDir,
									 QString group,
									 IssueLogger* log);

		virtual bool generateConfiguration() = 0;
	};
}
