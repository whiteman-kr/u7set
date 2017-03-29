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
							 Hardware::OptoModuleStorage *opticModuleStorage, IssueLogger* log, int buildNo, int changesetId, bool debug,
                             QString projectName, QString userName);
		virtual ~ConfigurationBuilder();

		bool build(BuildResultWriter &buildResultWriter);

		bool writeBinaryFiles(BuildResultWriter &buildResultWriter);

		Q_INVOKABLE int jsBuildNo();
		Q_INVOKABLE bool jsIsInterruptRequested();

		quint64 getFirmwareUniqueId(const QString &subsystemID, int lmNumber);

		void setGenericUniqueId(const QString& subsystemID, int lmNumber, quint64 genericUniqueId);

	protected:


	private:
		DbController* db();
		IssueLogger* log() const;
		int buildNo() const;
		int changesetId() const;
		bool debug() const;
		bool release() const;

        bool runConfigurationScriptFile(const QString& subsystemID, const QString& configurationScriptFile);


	private:
		Hardware::ModuleFirmwareCollection m_confCollection;

		BuildWorkerThread* m_buildWorkerThread = nullptr;
		DbController* m_db = nullptr;
		Hardware::DeviceRoot* m_deviceRoot = nullptr;
        std::vector<Hardware::DeviceModule*> m_lmModules;
        LmDescriptionSet *m_lmDescriptions = nullptr;
        SignalSet* m_signalSet = nullptr;
		Hardware::SubsystemStorage* m_subsystems = nullptr;
		Hardware::OptoModuleStorage *m_opticModuleStorage = nullptr;
		mutable IssueLogger* m_log = nullptr;

		int m_buildNo = 0;
		int m_changesetId = 0;
		int m_debug = false;
		QString m_projectName;
		QString m_userName;

	};

}

#endif // CONFIGURATIONBUILDER_H
