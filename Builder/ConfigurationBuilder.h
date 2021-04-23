#ifndef CONFIGURATIONBUILDER_H
#define CONFIGURATIONBUILDER_H

#include "../lib/AppSignal.h"
#include "BuildWorkerThread.h"

// Forware delcarations
//
class QThread;
class IssueLogger;
class DbController;

namespace Hardware
{
	class DeviceObject;
	class DeviceRoot;
	class McFirmwareOld;
}


namespace Builder
{

	// ------------------------------------------------------------------------
	//
	//		JsSignalSet
	//
	// ------------------------------------------------------------------------

	class JsSignalSet : public QObject
	{
		Q_OBJECT

	private:
		SignalSet* m_signalSet = nullptr;

	public:
		JsSignalSet(SignalSet* signalSet);
		Q_INVOKABLE QObject* getSignalByEquipmentID(const QString& equpmentID);
	};


	class ConfigurationBuilder : QObject
	{
		Q_OBJECT
	public:
		ConfigurationBuilder() = delete;
		ConfigurationBuilder(BuildWorkerThread* buildWorkerThread, Context* context);
		virtual ~ConfigurationBuilder();

		bool build();
		bool writeDataFiles();

		Q_INVOKABLE bool jsIsInterruptRequested();

	protected:


	private:
		DbController* db();
		IssueLogger* log() const;

		bool runConfigurationScriptFile(const std::vector<Hardware::DeviceModule *> &subsystemModules, LmDescription *lmDescription);

		bool writeExtraDataFiles();
		bool writeDeviceObjectToJson(const Hardware::DeviceObject* object, QJsonObject& jParent);


	private:
		Builder::BuildResultWriter* m_buildResultWriter = nullptr;

		BuildWorkerThread* m_buildWorkerThread = nullptr;
		DbController* m_db = nullptr;
		Hardware::DeviceRoot* m_deviceRoot = nullptr;
		std::vector<Hardware::DeviceModule*> m_fscModules;
        LmDescriptionSet *m_lmDescriptions = nullptr;
        SignalSet* m_signalSet = nullptr;
		SubsystemStorage* m_subsystems = nullptr;
		Hardware::OptoModuleStorage *m_opticModuleStorage = nullptr;
		mutable IssueLogger* m_log = nullptr;
		bool m_generateExtraDebugInfo = false;
	};

}

#endif // CONFIGURATIONBUILDER_H
