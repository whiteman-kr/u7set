#ifndef BUILDER_H
#define BUILDER_H

#include "../include/Signal.h"
#include "Subsystem.h"
#include "../Builder/BuildResultWriter.h"

// Forware declarations
//

class QThread;
class OutputLog;
class DbController;


namespace Builder
{
	class SignalSetObject;
	class ApplicationLogicData;
}

namespace Hardware
{
	class DeviceObject;
	class DeviceRoot;
	class McFirmwareOld;
}

namespace VFrame30
{
	class LogicScheme;
	class SchemeLayer;
}

namespace Afbl
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

		// Load Application Logic signals
		//
		bool loadSignals(DbController *db, SignalSet* signalSet);

		// Load Application Functional Block Library
		//
		bool loadAfbl(DbController *db, Afbl::AfbElementCollection* afbCollection);

		// Generate Modules Configurations Firmwares
		//
		bool modulesConfiguration(DbController* db, Hardware::DeviceRoot *deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage *subsystems, int changesetId, BuildResultWriter* buildWriter);

		// Build Application Logic
		//
		bool buildApplicationLogic(DbController* db, ApplicationLogicData* appLogicData, Afbl::AfbElementCollection* afbCollection, int changesetId);

		// Compile Application Logic
		//
		bool compileApplicationLogic(Hardware::DeviceObject* equipment, SignalSet* signalSet, Afbl::AfbElementCollection* afbCollection, ApplicationLogicData* appLogicData, BuildResultWriter* buildResultWriter);

		// Compile Data Aquisition Service configuration
		//
		bool compileDataAquisitionServiceConfiguration(Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, UnitList& unitInfo, BuildResultWriter* buildResultWriter);

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

		void setOutputLog(OutputLog* value);

		QString projectUserName() const;
		void setProjectUserName(const QString& value);

		QString projectUserPassword() const;
		void setProjectUserPassword(const QString& value);

		bool debug() const;
		void setDebug(bool value);

		bool release() const;

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

		bool m_debug = false;				// if true then don't get workcopy of checked out files, use unly checked in copy

		OutputLog* m_log = nullptr;					// Probably it's better to make it as shared_ptr
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
		Builder(OutputLog* log);
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
		void buildStarted();
		void buildFinished();

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
		OutputLog* m_log = nullptr;
	};

}

#endif // BUILDER_H
