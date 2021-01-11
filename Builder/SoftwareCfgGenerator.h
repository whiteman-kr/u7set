#pragma once

#include "../lib/DbController.h"
#include "../lib/Signal.h"
#include "../lib/DeviceObject.h"
#include "../lib/DataSource.h"
#include "../lib/Subsystem.h"
#include "../VFrame30/Schema.h"
#include "../VFrame30/SchemaItemFrame.h"
#include "../lib/SoftwareSettings.h"

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

		virtual bool createSettingsProfile(const QString& profile);
		virtual bool generateConfiguration() = 0;

		bool writeConfigurationXml();

		bool getSettingsXml(QXmlStreamWriter& xmlWriter);

		static bool generalSoftwareCfgGeneration(Context* context);
		static bool loadAllSchemas(Context* context);
		static void clearStaticData();

		static bool writeSchemaScriptProperties(VFrame30::Schema* schema, QString dir, BuildResultWriter* buildResultWriter);

		void writeSoftwareSection(QXmlStreamWriter& xmlWriter, bool finalizeSection);

		QString equipmentID() const;

	protected:
		static bool checkLmToSoftwareLinks(Context* context);

		static bool joinSchemas(Context* context, VFrame30::Schema* schema, const VFrame30::Schema* pannel, Qt::Edge edge);

		static bool loadFileFromDatabase(DbController* db, int parentId, const QString& fileName, QString *errorCode, QByteArray* data);

		QString getBuildInfoCommentsForBat();
		QString getBuildInfoCommentsForSh();
		bool getConfigIp(QString* cfgIP1, QString* cfgIP2);
		bool getServiceParameters(QString &parameters);

		QString softwareCfgSubdir() const { return m_software->equipmentIdTemplate(); }

	protected:
		Context* m_context = nullptr;
		Hardware::Software* m_software = nullptr;
		DbController* m_dbController = nullptr;
		SignalSet* m_signalSet = nullptr;
		Hardware::EquipmentSet* m_equipment = nullptr;
		BuildResultWriter* m_buildResultWriter = nullptr;
		IssueLogger* m_log = nullptr;

		ConfigurationXmlFile* m_cfgXml = nullptr;

		SoftwareSettingsSet m_settingsSet;

		static std::multimap<QString, std::shared_ptr<SchemaFile>> m_schemaTagToFile;
	};
}
