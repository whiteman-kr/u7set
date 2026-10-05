#pragma once

#include "../AppSignalLib/ComparatorSet.h"
#include "../AppSignalLib/Bus.h"
#include "SubsystemStorage.h"
#include "ModuleLogicCompiler.h"


namespace Builder
{
	class LmDescriptionSet;

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
		SubsystemStorage* subsystems();
		Hardware::EquipmentSet* equipmentSet();
		SignalSet* signalSet();
		LmDescriptionSet* lmDescriptions();
		std::shared_ptr<AppLogicData> appLogicData();
		Tuning::TuningDataStorage* tuningDataStorage();
		ComparatorSet* comparatorSet();
		BuildResultWriter* buildResultWriter();
		Builder::ConnectionStorage* connectionStorage();
		const AppSignalLib::BusSet* busSet();
		Hardware::OptoModuleStorage* opticModuleStorage();
		std::vector<Hardware::DeviceModule*>& lmModules();
		OnlineLib::BuildInfo buildInfo();
		std::vector<Hardware::DeviceModule*>& fscModules();

		//

	private:
		bool isBuildCancelled();

		bool compileActuatorsLogicsPass1();
		bool compileActuatorsLogicsPass2();

		bool checkLmIpAddresses();

		bool compileModulesLogicsPass1();
		bool compileModulesLogicsPass2();

		bool checkSignalsIDsAndHashes();

		bool writeResourcesUsageReport();

		bool writeSerialDataXml();
		bool writeOptoConnectionsReport();
		bool writeOptoConnectionsXml();
		bool writeOptoVhdFiles();
		bool writeOptoPortToPortVhdFile(const QString& connectionID, Hardware::OptoPortShared outPort, Hardware::OptoPortShared inPort);
		bool writeOptoSinglePortVhdFile(const QString& connectionID, Hardware::OptoPortShared outPort);
		bool writeAppSignalSetFile();
		bool writeCommonAppSignalsExtXmlFile();
		bool writeComparatorSetFile();
		bool writeSubsystemsXml();
		bool writeAppSignalsListCsv();

		void clear();

	private:
		Context* m_context = nullptr;
		int m_vdusCount = 0;

//		QVector<ModuleLogicCompiler*> m_moduleCompilers;
	};
}


