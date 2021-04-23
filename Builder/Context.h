#pragma once

#include <memory>
#include <QJSEngine>
#include <unordered_set>

#include "../DbLib/DbController.h"
#include "../HardwareLib/Connection.h"
#include "../VFrame30/Bus.h"
#include "../VFrame30/LogicSchema.h"
#include "../TuningService/TuningDataStorage.h"
#include "SignalSet.h"
#include "ConnectionStorage.h"
#include "SubsystemStorage.h"
#include "IssueLogger.h"
#include "BuildResultWriter.h"
#include "LmDescriptionSet.h"
#include "OptoModule.h"
#include "../lib/ComparatorSet.h"
#include "../Simulator/SimProfiles.h"

namespace Builder
{
	class AppLogicData;


	class Context
	{
	public:
		Context(IssueLogger* log, QString buildOutputPath, bool expertMode);
		Context(const Context&) = delete;
		Context(Context&&) = delete;
		Context& operator=(const Context&) = delete;
		Context& operator=(Context&&) = delete;

		bool generateAppSignalsXml() const;
		bool generateExtraDebugInfo()const;

	public:
		mutable IssueLogger* m_log = nullptr;

		DbController m_db;
		DbProjectProperties m_projectProperties;

		QString m_buildOutputPath;

		bool m_expertMode = false;

		int m_lastChangesetId = 0;

		// --
		//
		std::shared_ptr<BuildResultWriter> m_buildResultWriter;

		std::shared_ptr<Hardware::EquipmentSet> m_equipmentSet;
		std::shared_ptr<VFrame30::BusSet> m_busSet;

		std::shared_ptr<SignalSet> m_signalSet;

		std::vector<std::shared_ptr<VFrame30::LogicSchema>> m_appLogicSchemas;

		std::shared_ptr<LmDescriptionSet> m_lmDescriptions;
		std::shared_ptr<LmDescriptionSet> m_fscDescriptions;

		std::vector<Hardware::DeviceModule*> m_lmModules;
		std::vector<Hardware::DeviceModule*> m_lmAndBvbModules;
		std::vector<Hardware::DeviceModule*> m_fscModules;

		Sim::Profiles m_simProfiles;

		std::map<QString, Hardware::Software*> m_software;

		std::shared_ptr<SubsystemStorage> m_subsystems;
		std::map<QString, quint64> m_lmsUniqueIDs;

		std::shared_ptr<AppLogicData> m_appLogicData;

		std::shared_ptr<ConnectionStorage> m_connections;
		std::shared_ptr<Hardware::OptoModuleStorage> m_opticModuleStorage;

		std::shared_ptr<Tuning::TuningDataStorage> m_tuningDataStorage;
		std::shared_ptr<ComparatorSet> m_comparatorSet;

		std::unordered_set<QString> m_analogSignalsOnSchemas;

		//--
		//
		std::atomic_int m_progress;			// 0 - 100%
	};

}


