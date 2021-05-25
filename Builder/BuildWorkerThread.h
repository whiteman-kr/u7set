#pragma once

#include <QJSEngine>
#include "../TuningService/TuningDataStorage.h"
#include "Context.h"
#include "TuningBuilder.h"
#include "OptoModule.h"
#include "RunOrder.h"


namespace Builder
{

	struct BuildTask;

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

		// Build tasks:
		//		void taskXxx(std::atomic<double> progress);
		//		progress: ref to double value form 0 to 1.0 indicates progress of the current task
		//
	private:
		struct BuildTask;

		bool preBuild();
		bool postBuild();

		bool preRunTask(const BuildTask& task);
		bool postRunTask(const BuildTask& task);

		bool taskOpenProject();
		bool taskGetProjectProperties();
		bool taskStartBuildResultWriter();
		bool taskGetEquipment();
		bool taskLoadBusTypes();					// Load BusTypes (VFrame30::BusSet)
		bool taskLoadAppSignals();					// Load Builder::SignalSet
		bool taskLoadLmDescriptions();				// Load LmDescription files
		bool taskLoadSubsystems();					// Load subsystems
		bool taskLoadConnections();					// Load connections
		bool taskCheckSchemaIds();					// Check all schemas ids for uniqueness
		bool taskParseApplicationLogic();			// Parse application logic schemas
		bool taskSaveLogicModuleDescriptions();		// Save LogicModule Descriptions
		bool taskCompileApplicationLogic();			// Compile application logic
		bool taskProcessTuningParameters();			// Tuning Parameters
		bool taskGenerationModulesConfiguration();	// Generate Modules Configuration
		bool taskGenerationBitstreamFile();			// Generate Bitstream File
		bool taskGenerationSoftwareConfiguration();// Generate Software Configuration
		bool taskRunSimTests();					// Run Simulator-based tests

		struct BuildTask
		{
			using BuildTaskFunc = bool (BuildWorkerThread::*)();

			const BuildTaskFunc func;
			const QString name;					// Task name
			const bool breakOnFailed = false;	// if thrue and task build failed, then stop build

			struct BuildTaskResult
			{
				bool result;			// keeps return value of bool func(...) or no value if task was not run
				qint64 durationMs;		// time spent to perform task
			};
			std::optional<BuildTaskResult> result;
		};

		std::vector<BuildTask> m_buildTasks =
			{
				{
					.func = &BuildWorkerThread::taskOpenProject,
					.name = "Open Project",
					.breakOnFailed = true
				},
				{
					.func = &BuildWorkerThread::taskGetProjectProperties,
					.name = "Getting Project Properties",
					.breakOnFailed = true
				},
				{
					.func = &BuildWorkerThread::taskStartBuildResultWriter,
					.name = "Start BuildResultWriter",
					.breakOnFailed = true
				},
				{
					.func = &BuildWorkerThread::taskGetEquipment,
					.name = "Getting Equipment",
					.breakOnFailed = true
				},
				{
					.func = &BuildWorkerThread::taskLoadBusTypes,
					.name = "Loading BusType(s)",
					.breakOnFailed = false
				},
				{
					.func = &BuildWorkerThread::taskLoadAppSignals,
					.name = "Loading AppSignals",
					.breakOnFailed = false
				},
				{
					.func = &BuildWorkerThread::taskLoadLmDescriptions,
					.name = "Loading LogicModule Descriptions",
					.breakOnFailed = true
				},
				{
					.func = &BuildWorkerThread::taskLoadSubsystems,
					.name = "Loading SubSystems",
					.breakOnFailed = true
				},
				{
					.func = &BuildWorkerThread::taskLoadConnections,
					.name = "Loading Connections",
					.breakOnFailed = false
				},
				{
					.func = &BuildWorkerThread::taskCheckSchemaIds,
					.name = "Checking SchemaIDs Uniqueness",
					.breakOnFailed = false
				},
				{
					.func = &BuildWorkerThread::taskParseApplicationLogic,
					.name = "Application Logic Parsing",
					.breakOnFailed = true
				},
				{
					.func = &BuildWorkerThread::taskSaveLogicModuleDescriptions,
					.name = "Saving LogicModule Descriptions",
					.breakOnFailed = false
				},
				{
					.func = &BuildWorkerThread::taskCompileApplicationLogic,
					.name = "Application Logic Compilation",
					.breakOnFailed = true
				},
				{
					.func = &BuildWorkerThread::taskProcessTuningParameters,
					.name = "Tuning Parameters Processing",
					.breakOnFailed = true
				},
				{
					.func = &BuildWorkerThread::taskGenerationModulesConfiguration,
					.name = "Modules Configuration Generation",
					.breakOnFailed = false
				},
				{
					.func = &BuildWorkerThread::taskGenerationBitstreamFile,
					.name = "Bitstream File Generation",
					.breakOnFailed = false
				},
				{
					.func = &BuildWorkerThread::taskGenerationSoftwareConfiguration,
					.name = "Software Configuration Generation",
					.breakOnFailed = false
				},
				{
					.func = &BuildWorkerThread::taskRunSimTests,
					.name = "Simulator-based Tests",
					.breakOnFailed = false
				},
			};

	private:
		// Get Equipment from the project database
		//
		bool getEquipment(Hardware::DeviceObject* parent);

		void findFSCConfigurationModules(Hardware::DeviceObject* object, std::vector<Hardware::DeviceModule*>* out) const;
		void findModulesByFamily(Hardware::DeviceObject* object, std::vector<Hardware::DeviceModule*>* out, Hardware::DeviceModule::FamilyType family) const;

		// Expand Devices StrId
		//
		bool expandDeviceStrId(Hardware::DeviceObject* device);

		// Check same Uuids and same StrIds
		//
		bool checkUuidAndStrId(Hardware::DeviceObject* root);
		bool checkUuidAndStrIdWorker(Hardware::DeviceObject* device,
										 std::map<QUuid, Hardware::DeviceObject*>& uuidMap,
										 std::map<QString, Hardware::DeviceObject*>& strIdMap);

		bool checkChildRestrictions(std::shared_ptr<Hardware::DeviceObject> root);
		bool checkChildRestrictionsWorker(std::shared_ptr<Hardware::DeviceObject> device);

		// Load Application Logic signals
		//
		bool loadSignals(SignalSet* signalSet, Hardware::EquipmentSet* equipment);

		// Load Application Functional Block Library
		//
		bool loadLogicModuleDescription(Hardware::DeviceModule* logicModule, LmDescriptionSet* lmDescriptions);

		// Load Sim Profiles
		//
		bool loadSimProfiles();

		// Generate MATS software configurations
		//
		bool generateSoftwareConfiguration();

		bool checkProfiles();

		bool writeFirmwareStatistics(BuildResultWriter& buildResultWriter);

		bool generateModulesInformation(BuildResultWriter& buildWriter,
								   const std::vector<Hardware::DeviceModule *>& lmModules);

		bool generateLmsUniqueIDs(Context* context);

		bool writeLogicModulesInfoXml(Context* context);

		bool buildSoftwareList(Context* context);

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

		QElapsedTimer m_buildTimer;
		std::atomic<double> m_totalProgress;	// 0 - 100%

		int m_finalizedErrorCount = 0;
		int m_finalizedWarningCount = 0;
	};

}

