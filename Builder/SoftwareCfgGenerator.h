#pragma once

#include "../lib/DbController.h"
#include "../lib/Signal.h"
#include "../lib/DeviceObject.h"
#include "../lib/DataSource.h"
#include "../lib/Subsystem.h"
#include "../VFrame30/Schema.h"

#include "BuildResultWriter.h"
#include "IssueLogger.h"
#include "SignalSet.h"
#include "Context.h"

namespace Builder
{

	class SoftwareCfgGenerator : public QObject
	{
		Q_OBJECT

	public:
		struct SchemaFile
		{
			SchemaFile(const QString& _schemaId, const QString& _fileName, const QString& _subDir, const QString& _group, const QString& _details) :
				schemaId(_schemaId),
				fileName(_fileName),
				subDir(_subDir),
				group(_group),
				details(_details)
			{
			}

			QString schemaId;
			QString fileName;
			QString subDir;
			QString group;
			VFrame30::SchemaDetails details;
		};

	public:
		SoftwareCfgGenerator(Context* context, Hardware::Software* software);
		virtual ~SoftwareCfgGenerator();

		bool run();

		static bool generalSoftwareCfgGeneration(Context* context);
		static bool loadAllSchemas(Context* context);
		static void clearStaticData();

		virtual bool generateConfiguration() = 0;

		void initSubsystemKeyMap(SubsystemKeyMap* subsystemKeyMap, const Hardware::SubsystemStorage* subsystems);

		QString equipmentID() const;

	protected:
		static bool buildLmList(Hardware::EquipmentSet *equipment, IssueLogger* log);
		static bool buildSoftwareList(Hardware::EquipmentSet *equipment, IssueLogger* log);
		static bool checkLmToSoftwareLinks(IssueLogger* log);

	protected:
		Context* m_context = nullptr;
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

		static std::multimap<QString, std::shared_ptr<SchemaFile>> m_schemaTagToFile;

		Hardware::DeviceRoot* m_deviceRoot = nullptr;

		QString getBuildInfoCommentsForBat();
		QString getBuildInfoCommentsForSh();
		bool getConfigIp(QString* cfgIP1, QString* cfgIP2);
		bool getServiceParameters(QString &parameters);
	};
}
