#pragma once

#include "../lib/DbController.h"
#include "../lib/Signal.h"
#include "../lib/DeviceObject.h"
#include "../lib/DataSource.h"
#include "../lib/Subsystem.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaItemFrame.h"

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

		static bool writeSchemaScriptProperties(VFrame30::Schema* schema, QString dir, BuildResultWriter* buildResultWriter);

		virtual bool generateConfiguration() = 0;
		virtual bool getSettingsXml(QXmlStreamWriter& xmlWriter) = 0;

		void writeSoftwareSection(QXmlStreamWriter& xmlWriter, bool finalizeSection);

		void initSubsystemKeyMap(SubsystemKeyMap* subsystemKeyMap, const Hardware::SubsystemStorage* subsystems);

		QString equipmentID() const;

	protected:
		static bool buildLmList(Hardware::EquipmentSet *equipment, IssueLogger* log);
		static bool buildSoftwareList(Hardware::EquipmentSet *equipment, IssueLogger* log);
		static bool checkLmToSoftwareLinks(Context* context);

		static bool joinSchemas(Context* context, VFrame30::Schema* schema, const VFrame30::Schema* pannel, Qt::Edge edge);

		static bool loadFileFromDatabase(DbController* db, int parentId, const QString& fileName, QString *errorCode, QByteArray* data);

		bool getLmPropertiesFromDevice(const Hardware::DeviceModule* lm,
									   DataSource::DataType dataType,
									   int adapterNo,
									   E::LanControllerType adapterType,
									   const Hardware::EquipmentSet& equipmentSet,
									   const SubsystemKeyMap& subsystemKeyMap,
									   const QHash<QString, quint64>& lmUniqueIdMap,
									   DataSource* ds,
									   Builder::IssueLogger* log);

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
