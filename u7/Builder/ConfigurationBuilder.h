#ifndef CONFIGURATIONBUILDER_H
#define CONFIGURATIONBUILDER_H

#include "Builder.h"

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
		SignalSet *m_signalSet = nullptr;

	public:
		JsSignalSet(SignalSet* signalSet);
		Q_INVOKABLE QObject* getSignalByEquipmentID(const QString& equpmentID);
	};


	class ConfigurationBuilder : QObject
	{
		Q_OBJECT
	public:
		ConfigurationBuilder() = delete;
        ConfigurationBuilder(BuildWorkerThread* buildWorkerThread, DbController* db, Hardware::DeviceRoot* deviceRoot, const std::vector<Hardware::DeviceModule *> &lmModules, LmDescriptionSet* lmDescriptions, SignalSet* signalSet, Hardware::SubsystemStorage* subsystems,
							 Hardware::OptoModuleStorage *opticModuleStorage, Hardware::ModuleFirmwareCollection* firmwareCollection, IssueLogger* log);
		virtual ~ConfigurationBuilder();

		bool build(BuildResultWriter &buildResultWriter);

		bool writeDataFiles(BuildResultWriter &buildResultWriter);

		Q_INVOKABLE bool jsIsInterruptRequested();

	protected:


	private:
		DbController* db();
		IssueLogger* log() const;

		bool runConfigurationScriptFile(const std::vector<Hardware::DeviceModule *> &subsystemModules, LmDescription *lmDescription);


	private:
		Hardware::ModuleFirmwareCollection* m_firmwareCollection = nullptr;

		BuildWorkerThread* m_buildWorkerThread = nullptr;
		DbController* m_db = nullptr;
		Hardware::DeviceRoot* m_deviceRoot = nullptr;
        std::vector<Hardware::DeviceModule*> m_lmModules;
        LmDescriptionSet *m_lmDescriptions = nullptr;
        SignalSet* m_signalSet = nullptr;
		Hardware::SubsystemStorage* m_subsystems = nullptr;
		Hardware::OptoModuleStorage *m_opticModuleStorage = nullptr;
		mutable IssueLogger* m_log = nullptr;

		QJSEngine m_jsEngine;
	};

}

#endif // CONFIGURATIONBUILDER_H
