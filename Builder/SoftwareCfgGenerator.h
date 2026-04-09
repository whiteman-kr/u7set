#pragma once

#include "../AppSignalLib/ISignalManager.h"
#include "../OnlineLib/SoftwareSettings.h"
#include <HardwareLib/Software.h>
#include <VFrame30/SchemaDetails.h>

#include "BuildResultWriter.h"
#include "Context.h"
#include "IssueLogger.h"
#include "SignalSet.h"

namespace VFrame30
{
	class Schema;
	class VduSchema;
}

namespace Hardware
{
	class EquipmentSet;
}

namespace AppSignalLists
{
	class AppSignalList;
}

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

		virtual bool generateConfigurationStep1() = 0;		// In first will be executed for each software item in uncertain order
															//
															// Should be implemented in derived class

		virtual bool generateConfigurationStep2();			// Will be executed in uncertain order after
															// execution of generateConfigurationStep1 for ALL software items
															//
															// Required for link results of generateConfigurationStep1
															//
															// Implementation in derived class is not obligatory
		bool createConfigurationXml();

		bool getSettingsXml(QXmlStreamWriter& xmlWriter);

		static bool generalSoftwareCfgGeneration(Context* context);
		static bool loadAllSchemas(Context* context);
		static bool generateVduFonts(Context& context);
		static bool generateVduSchemas(const std::vector<VFrame30::VduSchema*>& schemas, Context& context);
		static void clearStaticData();

		static bool writeSchemaScriptProperties(VFrame30::Schema* schema, QString dir, BuildResultWriter* buildResultWriter);

		void writeSoftwareSection(QXmlStreamWriter& xmlWriter, bool finalizeSection);

		QString equipmentID() const;

	protected:
		static bool checkLmToSoftwareLinks(Context* context);

		static const Hardware::Software *getConnectedSoftware(const Context* context,
													   const QString& equipmentID,
													   bool checkConnectionToControllers);

		static bool joinSchemas(Context* context, VFrame30::Schema* schema, const VFrame30::Schema* pannel, Qt::Edge edge);

		static bool loadFileFromDatabase(DbController* db, int parentId, const QString& fileName, QString *errorCode, QByteArray* data);

		QString getBuildInfoComments(E::OS os) const;
		QString getCommonCmdLine(const HostAddressPort& cfgSrvIp1,
								  const HostAddressPort& cfgSrvIp2,
								  E::OS os,
								  bool runAsConsoleApp);

		QString getCommentStart(E::OS os) const;
		QString getRunScriptDirectory(E::OS os) const;
		QString getRunScriptName(const QString& profile, E::OS os) const;

		QString softwareCfgSubdir() const { return m_software->equipmentIdTemplate(); }

		static std::vector<AppSignal*> createAppSignalList(const QStringList& equipmentList, const SignalSet& signalSet);
		static std::vector<AppSignal*> createTuningSignalList(const QStringList& equipmentList, const SignalSet& signalSet);

		bool writeTuningSignals(const std::vector<AppSignal*>& tuningSignals);
		bool writeAppSignalLists(const ISignalManager& signalManager,
								 const QStringList& appSignalListIds,
								 const QStringList& appSignalListMasks,
								 const QStringList& appSignalListTags,
								 std::vector<std::shared_ptr<AppSignalLists::AppSignalList>>& appSignalLists);
		bool writeMatsUsers(const QString& propertyName, const QStringList& tuningUserAccounts);

		template <typename TYPE>
		TYPE getObjectProperty(QString strId, QString property, bool* ok)
		{
			if (ok == nullptr)
			{
				assert(false);
				return TYPE();
			}

			*ok = true;

			Hardware::DeviceObject* object = m_equipment->deviceObject(strId).get();
			if (object == nullptr)
			{
				m_log->errCFG3021(m_software->equipmentId(), property, strId);

				QString errorStr = tr("Object %1 is not found")
								   .arg(strId);

				m_cfgXml->xmlWriter().writeTextElement("Error", errorStr);

				*ok = false;
				return TYPE();
			}

			bool exists = object->propertyExists(property);
			if (exists == false)
			{
				QString errorStr = tr("Object %1 does not have property %2").arg(strId).arg(property);

				m_log->writeError(errorStr);
				m_cfgXml->xmlWriter().writeTextElement("Error", errorStr);

				*ok = false;
				return TYPE();
			}

			QVariant v = object->propertyValue(property);
			if (v.isValid() == false)
			{
				QString errorStr = tr("Object %1, property %2 is invalid").arg(strId).arg(property);

				m_log->writeError(errorStr);
				m_cfgXml->xmlWriter().writeTextElement("Error", errorStr);

				*ok = false;
				return TYPE();
			}

			if (v.canConvert<TYPE>() == false)
			{
				QString errorStr = tr("Object %1, property %2 has wrong type").arg(strId).arg(property);

				m_log->writeError(errorStr);
				m_cfgXml->xmlWriter().writeTextElement("Error", errorStr);

				*ok = false;
				return TYPE();
			}

			TYPE t = v.value<TYPE>();

			return t;
		}

		bool saveScriptProperties(QString scriptProperty, QString fileName);

	protected:
		Context* m_context = nullptr;
		Hardware::Software* m_software = nullptr;
		QStringList m_softwareControllersIDs;
		DbController* m_dbController = nullptr;
		SignalSet* m_signalSet = nullptr;
		Hardware::EquipmentSet* m_equipment = nullptr;
		BuildResultWriter* m_buildResultWriter = nullptr;
		IssueLogger* m_log = nullptr;

		ConfigurationXmlFile* m_cfgXml = nullptr;

		SoftwareSettingsSet m_settingsSet;

		static std::multimap<QString, std::shared_ptr<SchemaFile>> m_schemaTagToFile;

		// Profile_STR(TuningService.TuningSimIP:Port) => TuningService.EquipmentID
		//
		static std::map<QString, QString> m_tuningSimIpPorts;
	};
}
