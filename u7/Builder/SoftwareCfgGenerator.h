#pragma once

#include "../lib/DbController.h"
#include "../lib/Signal.h"
#include "../lib/DeviceObject.h"
#include "../lib/DataSource.h"

#include "BuildResultWriter.h"
#include "IssueLogger.h"
#include "Subsystem.h"
#include "SignalSet.h"

namespace Builder
{

	class SoftwareCfgGenerator : public QObject
	{
		Q_OBJECT

	public:
		struct SchemaFile
		{
			QString id;
			QString subDir;
			QString fileName;
			QString group;
			QString details;
		};

	public:
		SoftwareCfgGenerator(	DbController* db,
								Hardware::Software* software,
								SignalSet* signalSet,
								Hardware::EquipmentSet* equipment,
								BuildResultWriter* buildResultWriter);

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

		static bool writeAppLogicSchemasDetails(const QList<SchemaFile>& schemaFiles, BuildResultWriter* buildResultWriter, QString dir, IssueLogger* log);

		virtual bool generateConfiguration() = 0;

		void initSubsystemKeyMap(SubsystemKeyMap* subsystemKeyMap, const Hardware::SubsystemStorage* subsystems);

	protected:
		static bool buildLmList(Hardware::EquipmentSet *equipment, IssueLogger* log);
		static bool buildSoftwareList(Hardware::EquipmentSet *equipment, IssueLogger* log);
		static bool checkLmToSoftwareLinks(IssueLogger* log);

	protected:
		DbController* m_dbController = nullptr;
		Hardware::Software* m_software = nullptr;
		SignalSet* m_signalSet = nullptr;
		Hardware::EquipmentSet* m_equipment = nullptr;
		BuildResultWriter* m_buildResultWriter = nullptr;
		IssueLogger* m_log = nullptr;
		ConfigurationXmlFile* m_cfgXml = nullptr;
		QString m_subDir;

		static HashedVector<QString, Hardware::DeviceModule*> m_lmList;

		static HashedVector<QString, Hardware::Software*> m_softwareList;

		Hardware::DeviceRoot* m_deviceRoot = nullptr;

		static QList<SchemaFile> m_schemaFileList;


		QString getBuildInfoCommentsForBat();
		QString getBuildInfoCommentsForSh();
		bool getConfigIp(QString* cfgIP1, QString* cfgIP2);
		bool getServiceParameters(QString &parameters);
	};
}
