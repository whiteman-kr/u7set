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

		bool findLMs();
		void findLM(Hardware::DeviceObject* startFromDevice);

		bool checkAppSignals();
		bool expandBusSignals();
		bool prepareOptoConnectionsProcessing();
		bool checkLmIpAddresses();
		bool compileModulesLogicsPass1();
		bool processBvbModules();
		bool compileModulesLogicsPass2();

		bool writeBinCodeForLm(QString subsystemID, int subsystemKey, QString lmEquipmentID, QString lmCaption, int lmNumber, int frameSize, int frameCount, quint64 uniqueID, ApplicationLogicCode& appLogicCode);

		bool writeSerialDataXml();
		bool writeOptoConnectionsReport();
		bool writeOptoVhdFiles();
		bool writeOptoVhdFile(const QString& connectionID, Hardware::OptoPortShared outPort, Hardware::OptoPortShared inPort);
		bool writeOptoModulesReport();

		bool writeAppSignalSetFile();

		const LmDescriptionSet& lmDescriptionSet() const;

		void clear();

	private:
		Hardware::SubsystemStorage* m_subsystems = nullptr;
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

		QVector<Hardware::DeviceModule*> m_lm;

		QHash<QString, int> m_busSignals;

		friend class ModuleLogicCompiler;
	};
}


