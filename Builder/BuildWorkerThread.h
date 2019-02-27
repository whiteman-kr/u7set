#ifndef BUILDWORKERTHREAD_H
#define BUILDWORKERTHREAD_H

#include <QJSEngine>
#include "../lib/Subsystem.h"
#include "../lib/CommonTypes.h"
#include "../TuningService/TuningDataStorage.h"
#include "Context.h"
#include "TuningBuilder.h"
#include "OptoModule.h"
#include "RunOrder.h"


namespace Builder
{

	//
	//		BuildWorkerThread
	//
	class BuildWorkerThread : public QThread
	{
		Q_OBJECT

	public:
		BuildWorkerThread(QObject* parent = nullptr);

	private:
		virtual void run() override;

		// Get SuppressWarning list
		//
		bool getProjectProperties();

		// Get Equipment from the project database
		//
		bool getEquipment();
		bool getEquipment(Hardware::DeviceObject* parent);

		void findFSCConfigurationModules(Hardware::DeviceObject* object, std::vector<Hardware::DeviceModule*>* out) const;
		void findModulesByFamily(Hardware::DeviceObject* object, std::vector<Hardware::DeviceModule*>* out, Hardware::DeviceModule::FamilyType family) const;

		// Expand Devices StrId
		//
		bool expandDeviceStrId(Hardware::DeviceObject* device);

		// Load subsystems
		//
		bool loadSubsystems();

		// Check same Uuids and same StrIds
		//
		bool checkUuidAndStrId(Hardware::DeviceObject* root);
		bool checkUuidAndStrIdWorker(Hardware::DeviceObject* device,
										 std::map<QUuid, Hardware::DeviceObject*>& uuidMap,
										 std::map<QString, Hardware::DeviceObject*>& strIdMap);

		bool checkChildRestrictions(Hardware::DeviceObject* root);
		bool checkChildRestrictionsWorker(Hardware::DeviceObject* device);

		// Load Application Logic signals
		//
		bool loadSignals(SignalSet* signalSet, Hardware::EquipmentSet* equipment);

		// Load BusTypes (VFrame30::BusSet)
		//
		bool loadBusses();

		// Load Application Functional Block Library
		//
		bool loadLmDescriptions();
		bool loadLogicModuleDescription(Hardware::DeviceModule* logicModule, LmDescriptionSet* lmDescriptions);

		// Load Connections
		//
		bool loadConnections();

		// Check that all files (and from that theirs SchemaIds) in $root$/Schema are unique
		//
		bool checkRootSchemasUniquesIds(DbController* db);

		// Build Application Logic
		//
		bool parseApplicationLogic();

		// Save Logic Modules Descriptions
		//
		bool saveLogicModuleDescriptions(const LmDescriptionSet& lmDescriptions,
										 std::shared_ptr<BuildResultWriter> buildResultWriter);

		// Compile Application Logic
		//
		bool compileApplicationLogic(Hardware::SubsystemStorage* subsystems,
										const std::vector<Hardware::DeviceModule*>& lmModules,
										Hardware::EquipmentSet*equipmentSet,
										Hardware::OptoModuleStorage* optoModuleStorage,
										Hardware::ConnectionStorage* connections,
										SignalSet* signalSet,
										LmDescriptionSet* lmDescriptions,
										AppLogicData* appLogicData,
										Tuning::TuningDataStorage* tuningDataStorage,
										ComparatorStorage* comparatorStorage, VFrame30::BusSet *busSet,
										std::shared_ptr<BuildResultWriter> buildResultWriter);

		// Generate MATS software configurations
		//
		bool generateSoftwareConfiguration(const LmsUniqueIdMap& lmsUniqueIdMap);

		bool writeBinaryFiles(BuildResultWriter& buildResultWriter);

		void generateModulesInformation(BuildResultWriter& buildWriter,
								   const std::vector<Hardware::DeviceModule *>& lmModules);

		void generateLmsUniqueID(BuildResultWriter& buildWriter,
								 const std::vector<Hardware::DeviceModule *>& lmModules,
								 LmsUniqueIdMap& lmsUniqueIdMap);

	signals:
		void runOrderReady(RunOrder runOrder);

		// Properties
		//
	public:
		QString projectName() const;
		void setProjectName(QString value);

		QString serverIpAddress() const;
		void setServerIpAddress(QString value);

		int serverPort() const;
		void setServerPort(int value);

		QString serverUsername() const;
		void setServerUsername(QString value);

		QString serverPassword() const;
		void setServerPassword(QString value);

		DbProjectProperties projectProperties() const;

		void setIssueLog(IssueLogger* value);

		QString projectUserName() const;
		void setProjectUserName(QString value);

		QString projectUserPassword() const;
		void setProjectUserPassword(QString value);

		QString buildOutputPath() const;
		void setBuildOutputPath(QString value);

		bool debug() const;
		void setDebug(bool value);

		bool release() const;

		bool expertMode() const;
		void setExpertMode(bool value);

		bool isInterruptRequested();

		// Data
		//
	private:
		QString m_projectName;

		QString m_serverIpAddress;
		int m_serverPort = 0;
		QString m_serverUsername;
		QString m_serverPassword;

		QString m_projectUserName;
		QString m_projectUserPassword;

		DbProjectProperties m_projectProperties;

		QString m_buildOutputPath;

		bool m_debug = false;							// if true then don't get workcopy of checked out files, use unly checked in copy
		bool m_expertMode = false;

		IssueLogger* m_log = nullptr;					// Probably it's better to make it as shared_ptr

		std::unique_ptr<Context> m_context;
	};

}

#endif // BUILDWORKERTHREAD_H
