#pragma once

#include <QObject>
#include <QTranslator>
#include <QUuid>

#include "ModuleLogicCompiler.h"
#include "ComparatorStorage.h"

#include "Subsystem.h"

#include "../VFrame30/Bus.h"


namespace Builder
{
	class ApplicationLogicCompiler;
	class LmDescriptionSet;

	typedef bool (ApplicationLogicCompiler::*ApplicationLogicCompilerProc)(void);

	class ApplicationLogicCompiler : public QObject
	{
		Q_OBJECT

	public:
		ApplicationLogicCompiler(Hardware::SubsystemStorage *subsystems,
								 const std::vector<Hardware::DeviceModule*>& lmModules,
								 Hardware::EquipmentSet* equipmentSet,
								 Hardware::OptoModuleStorage* optoModuleStorage,
								 Hardware::ConnectionStorage* connections,
								 SignalSet* signalSet,
								 LmDescriptionSet* lmDescriptions,
								 AppLogicData* appLogicData,
								 Tuning::TuningDataStorage* tuningDataStorage,
								 ComparatorStorage* comparatorStorage,
								 VFrame30::BusSet* busSet,
								 BuildResultWriter* buildResultWriter,
								 IssueLogger* log);

		~ApplicationLogicCompiler();

		bool run();

	private:
		bool isBuildCancelled();

		bool prepareOptoConnectionsProcessing();
		bool checkLmIpAddresses();
		bool compileModulesLogicsPass1();
		bool processBvbModules();
		bool compileModulesLogicsPass2();

		bool writeResourcesUsageReport();

		bool writeBinCodeForLm(QString subsystemID, int subsystemKey, int appLogicUartId, QString lmEquipmentID, QString lmCaption, int lmNumber, int frameSize, int frameCount, int lmDescriptionNumber, quint64 uniqueID, ApplicationLogicCode& appLogicCode);

		bool writeSerialDataXml();
		bool writeOptoConnectionsReport();
		bool writeOptoVhdFiles();
		bool writeOptoPortToPortVhdFile(const QString& connectionID, Hardware::OptoPortShared outPort, Hardware::OptoPortShared inPort);
		bool writeOptoSinglePortVhdFile(const QString& connectionID, Hardware::OptoPortShared outPort);
		bool writeOptoModulesReport();

		bool writeAppSignalSetFile();
		bool writeSubsystemsXml();

		const LmDescriptionSet& lmDescriptionSet() const;

		void clear();

	private:
		Hardware::SubsystemStorage* m_subsystems = nullptr;
		const std::vector<Hardware::DeviceModule*> m_lmModules;
		Hardware::EquipmentSet* m_equipmentSet = nullptr;
		Hardware::DeviceObject* m_deviceRoot = nullptr;
		Hardware::OptoModuleStorage* m_optoModuleStorage = nullptr;
		SignalSet* m_signals = nullptr;
		LmDescriptionSet* m_lmDescriptions = nullptr;
		AppLogicData* m_appLogicData = nullptr;
		Tuning::TuningDataStorage* m_tuningDataStorage = nullptr;
		ComparatorStorage* m_cmpStorage = nullptr;
		const VFrame30::BusSet* m_busSet = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		Hardware::ConnectionStorage* m_connections = nullptr;

		static IssueLogger* m_log;

		QVector<ModuleLogicCompiler*> m_moduleCompilers;

		QHash<int, QString> m_busSignals;
		int m_maxSignalID = -1;

		friend class ModuleLogicCompiler;
	};
}


