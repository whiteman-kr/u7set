#pragma once

#include <QObject>
#include <QTranslator>
#include <QUuid>

#include "../Builder/ModuleLogicCompiler.h"

#include "Subsystem.h"


namespace Builder
{
	class ApplicationLogicCompiler;
	struct LmDescriptionSet;

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
								 BuildResultWriter* buildResultWriter,
								 IssueLogger* log);

		~ApplicationLogicCompiler();

		bool run();

	private:
		bool isBuildCancelled();

		bool findLMs();
		void findLM(Hardware::DeviceObject* startFromDevice);

		bool checkAppSignals();
		bool checkOptoConnections();
		bool checkLmIpAddresses();
		bool compileModulesLogicsPass1();
		bool compileModulesLogicsPass2();
		bool disposeOptoModulesTxRxBuffers();

		bool writeBinCodeForLm(QString subsystemID, int subsystemKey, QString lmEquipmentID, QString lmCaption, int lmNumber, int frameSize, int frameCount, quint64 uniqueID, ApplicationLogicCode& appLogicCode);

		bool writeOptoConnectionsReport();
		bool writeOptoVhdFiles();
		bool writeOptoVhdFile(const QString& connectionID, Hardware::OptoPort* outPort, Hardware::OptoPort* inPort);
		bool writeOptoModulesReport();

		void writeOptoPortInfo(Hardware::OptoPort* port, QStringList& list);

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
		BuildResultWriter* m_resultWriter = nullptr;
		Hardware::ConnectionStorage* m_connections = nullptr;

		static IssueLogger* m_log;

		QVector<ModuleLogicCompiler*> m_moduleCompilers;

		QVector<Hardware::DeviceModule*> m_lm;

		QString msg;


		friend class ModuleLogicCompiler;
	};
}


