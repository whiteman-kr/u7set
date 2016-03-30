#ifndef CONFIGURATIONBUILDER_H
#define CONFIGURATIONBUILDER_H

#include "Builder.h"

// Forware delcarations
//
class QThread;
class OutputLog;
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
		Q_INVOKABLE QObject* getSignalByDeviceStrID(const QString& deviceStrID);
	};


	class ConfigurationBuilder : QObject
	{
		Q_OBJECT
	public:
		ConfigurationBuilder() = delete;
        ConfigurationBuilder(DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage* subsystems, Hardware::OptoModuleStorage *opticModuleStorage, OutputLog* log, int changesetId, bool debug, QString projectName, QString userName, BuildResultWriter* buildWriter);
		virtual ~ConfigurationBuilder();

		bool build();

	protected:


	private:
		void findLmModules(Hardware::DeviceObject* object, std::vector<Hardware::DeviceModule*>& modules);
		DbController* db();
		OutputLog* log() const;
		int changesetId() const;
		bool debug() const;
		bool release() const;

	private:
		DbController* m_db = nullptr;
		Hardware::DeviceRoot* m_deviceRoot = nullptr;
		SignalSet* m_signalSet = nullptr;
		Hardware::SubsystemStorage* m_subsystems = nullptr;
        Hardware::OptoModuleStorage *m_opticModuleStorage = nullptr;
		mutable OutputLog* m_log = nullptr;
		BuildResultWriter* m_buildWriter = nullptr;

		int m_changesetId = 0;
		int m_debug = false;
		QString m_projectName;
		QString m_userName;

	};

}

#endif // CONFIGURATIONBUILDER_H
