#pragma once

#include "../include/DbController.h"
#include "../include/Signal.h"
#include "../include/DeviceObject.h"
#include "BuildResultWriter.h"
#include "IssueLogger.h"
#include "Subsystem.h"

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

		static HashedVector<QString, Hardware::DeviceModule*> m_lmList;

		static HashedVector<QString, Hardware::Software*> m_softwareList;

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
		static bool buildSoftwareList(Hardware::EquipmentSet *equipment, IssueLogger* log);
		static bool checkLmToSoftwareLinks(IssueLogger* log);


		static const int LM_ETHERNET_ADAPTER1 = 1;
		static const int LM_ETHERNET_ADAPTER2 = 2;
		static const int LM_ETHERNET_ADAPTER3 = 3;

		struct LmEthernetAdapterNetworkProperties
		{
			int adapterNo;		// LM_ETHERNET_ADAPTER* values
			QString adapterID;

			// only for adapterNo == LM_ETHERNET_ADAPTER1
			//
			bool tuningEnable = true;
			QString tuningIP;
			int tuningPort = 0;
			QString tuningServiceID;

			// only for adapterNo == LM_ETHERNET_ADAPTER2 or adapterNo == LM_ETHERNET_ADAPTER3
			//
			bool appDataEnable = true;
			QString appDataIP;
			int appDataPort = 0;
			QString appDataServiceID;

			bool diagDataEnable = true;
			QString diagDataIP;
			int diagDataPort = 0;
			QString diagDataServiceID;

			bool getLmEthernetAdapterNetworkProperties(Hardware::DeviceModule* lm, int adapterNo, IssueLogger* log);
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

		virtual bool generateConfiguration() = 0;
	};
}
