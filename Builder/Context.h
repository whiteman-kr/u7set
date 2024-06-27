#pragma once

#include <DbLib/DbController.h>
#include <Simulator/SimProfiles.h>

#include "../AppSignalLib/Bus.h"
#include "../AppSignalLib/ComparatorSet.h"
#include "../lib/TuningDataStorage.h"

#include "DiagSignalTypesStorage.h"

#include "BuildOptions.h"
#include "BuildResultWriter.h"
#include "ConnectionStorage.h"
#include "DbMatsUsers.h"
#include "IssueLogger.h"
#include "LmDescriptionSet.h"
#include "OptoModule.h"
#include "SignalSet.h"
#include "SubsystemStorage.h"
#include "Vdu/VduFontProvider.h"

namespace VFrame30
{
	class Schema;
	class VduSchema;
	class LogicSchema;
} // namespace VFrame30

namespace Builder
{
	class AppLogicData;

	struct PackedLogicSource
	{
		QString appSignalID;
		QString sourceItemLabelOut;
	};

	struct LmPackedLogicSources
	{
		QString lmID;
		std::list<PackedLogicSource> sources;
	};

	class Context
	{
	public:
		Context(IssueLogger* log, QString buildOutputPath, bool expertMode, BuildOptions buildOptions);
		Context(const Context&) = delete;
		Context(Context&&) = delete;
		Context& operator=(const Context&) = delete;
		Context& operator=(Context&&) = delete;

		~Context();

		bool generateAppSignalsXml() const;
		bool generateAppSignalsExtXml() const;
		bool generateExtraDebugInfo() const;

	public:
		mutable IssueLogger* m_log = nullptr;

		DbController m_db;
		DbProjectProperties m_projectProperties;

		QString m_buildOutputPath;

		bool m_expertMode = false;
		BuildOptions m_buildOptions;

		int m_lastChangesetId = 0;

		// --
		//
		std::shared_ptr<BuildResultWriter> m_buildResultWriter;

		std::shared_ptr<Hardware::EquipmentSet> m_equipmentSet;
		std::shared_ptr<AppSignalLib::BusSet> m_busSet;

		std::shared_ptr<SignalSet> m_signalSet;

		std::vector<std::shared_ptr<VFrame30::LogicSchema>> m_appLogicSchemas;

		std::shared_ptr<LmDescriptionSet> m_lmDescriptions;

		std::vector<Hardware::DeviceModule*> m_lmModules;

		std::vector<Hardware::DeviceModule*> m_fscModules;   // includes LM and BVB modules
		std::vector<Hardware::DeviceModule*> m_vduModules;   // includes VDU modules

		std::map<QString, std::map<Hash, int>> m_vduSignals; // VDU EquipmentID => (Hash(appSignalID) => VDU signal index)

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

		std::map<QString, std::list<LmPackedLogicSources>>
			m_packedLogicSources; // Key is label of packed_*_out schema item => list of LmPacketLogicSources

		DbMatsUserStorage m_matsUsers;

		std::shared_ptr<DiagSignalTypesStorage> m_diagSignalTypes;

		VduFontProvider m_vduFontProvider;

		std::map<QString, const Hardware::DeviceModule*> m_actuators;

		// Generated VDU schemas
		//
		struct GeneratedVduSchema
		{
			std::shared_ptr<VFrame30::Schema> schema;
			uint64_t crc64 = 0;                        // Generated CRC64 of the schema in VDU format
		};

		using VduSchemaList = std::list<GeneratedVduSchema>;
		std::map<QString, VduSchemaList> m_vduSchemas; // Key is VduEquipmentID, value is VduSchemas assigned to this VDU.
	};

} // namespace Builder
