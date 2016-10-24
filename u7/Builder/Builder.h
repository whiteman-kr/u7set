#ifndef BUILDER_H
#define BUILDER_H

#include "../lib/Signal.h"
#include "Subsystem.h"
#include "Connection.h"
#include "../Builder/BuildResultWriter.h"
#include "../TuningService/TuningDataStorage.h"
#include "../Builder/IssueLogger.h"

// Forware declarations
//

class QThread;
class DbController;


namespace Builder
{
	class SignalSetObject;
	class AppLogicData;
}

namespace Hardware
{
	class DeviceObject;
	class DeviceRoot;
	class McFirmwareOld;
}

namespace VFrame30
{
	class LogicSchema;
	class SchemaLayer;
}

namespace Afb
{
	class AfbElementCollection;
}

namespace Builder
{
	// ------------------------------------------------------------------------
	//
	//		BuildWorkerThread
	//
	// ------------------------------------------------------------------------

	class BuildWorkerThread : public QThread
	{
		Q_OBJECT

	private:
		virtual void run() override;

		// Get Equipment from the project database
		//
		bool getEquipment(DbController* db, Hardware::DeviceObject* parent);

		// Expand Devices StrId
		//
		bool expandDeviceStrId(Hardware::DeviceObject* device);

		// Check same Uuids and same StrIds
		//
		bool checkUuidAndStrId(Hardware::DeviceObject* root);
		bool checkUuidAndStrIdWorker(Hardware::DeviceObject* device,
										 std::map<QUuid, Hardware::DeviceObject*>& uuidMap,
										 std::map<QString, Hardware::DeviceObject*>& strIdMap);

		// Load Application Logic signals
		//
		bool loadSignals(DbController *db, SignalSet* signalSet, Hardware::EquipmentSet &equipment);

		bool checkAppSignals(SignalSet& signalSet, Hardware::EquipmentSet& equipment);

		// Load Application Functional Block Library
		//
		bool loadAfbl(DbController *db, Afb::AfbElementCollection* afbCollection);

		// Generate Modules Configurations Firmwares
		//
		bool modulesConfiguration(BuildWorkerThread* buildWorkerThread, DbController* db, Hardware::DeviceRoot *deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage *subsystems, Hardware::OptoModuleStorage *opticModuleStorage, int changesetId, BuildResultWriter* buildWriter);

		// Generate Tuning configuration
		//
		bool tuningParameters(DbController* db, Hardware::DeviceRoot *deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage *subsystems, Tuning::TuningDataStorage *tuningDataStorage, int changesetId, BuildResultWriter* buildWriter);

		// Build Application Logic
		//
		bool parseApplicationLogic(DbController* db, AppLogicData* appLogicData, Afb::AfbElementCollection* afbCollection, Hardware::EquipmentSet* equipment, SignalSet* signalSet, int changesetId);

		// Compile Application Logic
		//
		bool compileApplicationLogic(	Hardware::SubsystemStorage* subsystems,
										Hardware::EquipmentSet*equipmentSet,
										Hardware::OptoModuleStorage* optoModuleStorage,
										Hardware::ConnectionStorage* connections,
										SignalSet* signalSet,
										Afb::AfbElementCollection* afbCollection,
										AppLogicData* appLogicData,
										Tuning::TuningDataStorage* tuningDataStorage,
										BuildResultWriter* buildResultWriter);

		// Generate SCADA software configurations
		//
		bool generateSoftwareConfiguration(	DbController* db,
											Hardware::SubsystemStorage* subsystems,
											Hardware::EquipmentSet* equipment,
											SignalSet* signalSet,
											Tuning::TuningDataStorage* tuningDataStorage,
											BuildResultWriter* buildResultWriter);

		// What's the next compilation task?
		//

	signals:
		void resultReady(const QString &s);

		// Properties
		//
	public:
		QString projectName() const;
		void setProjectName(const QString& value);

		QString serverIpAddress() const;
		void setServerIpAddress(const QString& value);

		int serverPort() const;
		void setServerPort(int value);

		QString serverUsername() const;
		void setServerUsername(const QString& value);

		QString serverPassword() const;
		void setServerPassword(const QString& value);

		void setIssueLog(IssueLogger* value);

		QString projectUserName() const;
		void setProjectUserName(const QString& value);

		QString projectUserPassword() const;
		void setProjectUserPassword(const QString& value);

		bool debug() const;
		void setDebug(bool value);

		bool release() const;

		bool isInterruptRequested();

		// Data
		//
	private:
		mutable QMutex m_mutex;

		QString m_projectName;

		QString m_serverIpAddress;
		int m_serverPort = 0;
		QString m_serverUsername;
		QString m_serverPassword;

		QString m_projectUserName;
		QString m_projectUserPassword;

		bool m_debug = false;							// if true then don't get workcopy of checked out files, use unly checked in copy

		IssueLogger* m_log = nullptr;					// Probably it's better to make it as shared_ptr
	};


	// ------------------------------------------------------------------------
	//
	//		Builder
	//
	// ------------------------------------------------------------------------

	class Builder : public QObject
	{
		Q_OBJECT

	public:
		Builder(IssueLogger* log);
		virtual ~Builder();

		// Public methods
		//
	public:
		bool start(QString projectName,
				   QString serverIp,
				   int serverPort,
				   QString serverUserName,
				   QString serverPassword,
				   QString projectUserName,
				   QString projectUserPassword,
				   bool debug);

		void stop();

		bool isRunning() const;

		// Signlas
		//
	signals:

		// Slots
		//
	protected slots:
		void handleResults(QString result);

		// Properties
		//
	private:
		bool debug() const;

		// Data
		//
	private:
		BuildWorkerThread* m_thread = nullptr;
		IssueLogger* m_log = nullptr;
	};

}

#endif // BUILDER_H
