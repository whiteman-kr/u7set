#pragma once

#include <QObject>
#include <QTranslator>
#include <QUuid>

#include "../Builder/ModuleLogicCompiler.h"

#include "Subsystem.h"


namespace Builder
{

	class ApplicationLogicCompiler : public QObject
	{
		Q_OBJECT

	private:
		Hardware::SubsystemStorage* m_subsystems = nullptr;
		Hardware::EquipmentSet* m_equipmentSet = nullptr;
		Hardware::DeviceObject* m_deviceRoot = nullptr;
		Hardware::OptoModuleStorage* m_optoModuleStorage = nullptr;
		SignalSet* m_signals = nullptr;
		Afb::AfbElementCollection* m_afbl = nullptr;
		AppLogicData* m_appLogicData = nullptr;
		TuningDataStorage* m_tuningDataStorage = nullptr;
		BuildResultWriter* m_resultWriter = nullptr;
		Hardware::ConnectionStorage* m_connections = nullptr;

		static IssueLogger* m_log;

		QVector<ModuleLogicCompiler*> m_moduleCompilers;

		QVector<Hardware::DeviceModule*> m_lm;

		QString msg;

	private:
		void findLMs();
		void findLM(Hardware::DeviceObject* startFromDevice);

		bool checkOptoConnections();
		bool compileModulesLogicsPass1();
		bool compileModulesLogicsPass2();
		bool disposeOptoModulesTxRxBuffers();

		bool writeBinCodeForLm(QString subsysStrID, QString lmCaption, int channel, int frameSize, int frameCount, const QByteArray& appLogicBinCode);

		void clear();

	public:
		ApplicationLogicCompiler(Hardware::SubsystemStorage *subsystems,
								 Hardware::EquipmentSet* equipmentSet,
								 Hardware::OptoModuleStorage* optoModuleStorage,
								 Hardware::ConnectionStorage* connections,
								 SignalSet* signalSet,
								 Afb::AfbElementCollection* afblSet,
								 AppLogicData* appLogicData,
								 TuningDataStorage* tuningDataStorage,
								 BuildResultWriter* buildResultWriter,
								 IssueLogger* log);

		~ApplicationLogicCompiler();

		bool run();

		friend class ModuleLogicCompiler;
	};
}


