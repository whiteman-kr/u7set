#ifndef BUILDWORKERTHREAD_H
#define BUILDWORKERTHREAD_H

#include <QJSEngine>
#include "../lib/Subsystem.h"
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
		bool saveLogicModuleDescriptions();

		// Compile Application Logic
		//
		bool compileApplicationLogic();

		// Generate MATS software configurations
		//
		bool generateSoftwareConfiguration();

		bool writeFirmwareStatistics(BuildResultWriter& buildResultWriter);

		bool writeBinaryFiles(BuildResultWriter& buildResultWriter);

		void generateModulesInformation(BuildResultWriter& buildWriter,
								   const std::vector<Hardware::DeviceModule *>& lmModules);

		bool generateLmsUniqueIDs(Context* context);

		bool writeLogicModulesInfoXml(Context* context);

		bool buildSoftwareList(Context* context);

		bool createSchemasAlbums();

		// Run simulator-based script tests
		//
		bool runSimTests();

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

		void setIssueLog(IssueLogger* value);

		QString projectUserName() const;
		void setProjectUserName(QString value);

		QString projectUserPassword() const;
		void setProjectUserPassword(QString value);

		QString buildOutputPath() const;
		void setBuildOutputPath(QString value);

		bool expertMode() const;
		void setExpertMode(bool value);

		bool isInterruptRequested();

		int progress() const;

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

		QString m_buildOutputPath;

		bool m_expertMode = false;

		IssueLogger* m_log = nullptr;		// Probably it's better to make it as shared_ptr

		std::unique_ptr<Context> m_context;
	};

}

#endif // BUILDWORKERTHREAD_H
