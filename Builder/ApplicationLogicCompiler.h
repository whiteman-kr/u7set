#pragma once

#include "../lib/Subsystem.h"
#include "../VFrame30/Bus.h"
#include "ModuleLogicCompiler.h"
#include "../lib/ComparatorSet.h"


namespace Builder
{
	class ApplicationLogicCompiler;
	class LmDescriptionSet;

	typedef bool (ApplicationLogicCompiler::*ApplicationLogicCompilerProc)(void);

	class ApplicationLogicCompiler : public QObject
	{
		Q_OBJECT

	public:
		ApplicationLogicCompiler(Context* context);

		~ApplicationLogicCompiler();

		bool run();

		// context getters
		//
		Context* context();
		IssueLogger* log();
		Hardware::SubsystemStorage* subsystems();
		Hardware::EquipmentSet* equipmentSet();
		SignalSet* signalSet();
		LmDescriptionSet* lmDescriptions();
		AppLogicData* appLogicData();
		Tuning::TuningDataStorage* tuningDataStorage();
		ComparatorSet* comparatorSet();
		BuildResultWriter* buildResultWriter();
		Hardware::ConnectionStorage* connectionStorage();
		const VFrame30::BusSet* busSet();
		Hardware::OptoModuleStorage* opticModuleStorage();
		std::vector<Hardware::DeviceModule*>& lmModules();
		std::vector<Hardware::DeviceModule*>& lmAndBvbModules();

		//

	private:
		bool isBuildCancelled();

		bool prepareOptoConnectionsProcessing();
		bool checkLmIpAddresses();
		bool compileModulesLogicsPass1();
		bool compileModulesLogicsPass2();

		bool writeResourcesUsageReport();

		bool writeSerialDataXml();
		bool writeOptoConnectionsReport();
		bool writeOptoVhdFiles();
		bool writeOptoPortToPortVhdFile(const QString& connectionID, Hardware::OptoPortShared outPort, Hardware::OptoPortShared inPort);
		bool writeOptoSinglePortVhdFile(const QString& connectionID, Hardware::OptoPortShared outPort);
		bool writeOptoModulesReport();

		bool writeAppSignalSetFile();
		bool writeComparatorSetFile();
		bool writeSubsystemsXml();

		void clear();

	private:
		Context* m_context = nullptr;

		QVector<ModuleLogicCompiler*> m_moduleCompilers;
	};
}


