#pragma once

#include "../SimConnectionsImpl.h"
#include "../SimEeprom.h"
#include "../SimScopedLog.h"
// #include "../SimLans.h"
#include "SimDeviceEmulatorCommon.h"

#include <SimulatorLib/SimDeviceState.h>
#include <SimulatorLib/SimVduSharedData.h>

#include <HardwareLib/LmDescription.h>
#include <HardwareLib/LogicModulesInfo.h>
#include <HardwareLib/ModuleFirmware.h>


namespace Sim
{
	class SimulatorPrivate;

	//
	// VduDeviceEmulator
	//
	class VduDeviceEmulator : public QObject
	{
		Q_OBJECT

		friend ::Sim::Snapshot;

	public:
		explicit VduDeviceEmulator(SimulatorPrivate* simulator);
		virtual ~VduDeviceEmulator();

	public:
		bool clear();
		DeviceError init(const Hardware::LogicModuleInfo& logicModuleInfo, // Run from UI thread
						 const LmDescription& lmDescription,
						 const Eeprom& confEeprom,
						 QString subsystemPath,
						 const ConnectionsImpl& connections,
						 const LogicModulesInfo& logicModulesExtraInfo);

		bool powerOff();
		bool reset();
		bool runWorkcycle(std::chrono::microseconds currentTime, QDateTime currentDateTime, qint64 workcycle);

		//	Public methods to access from simulation commands
		//
	public:
		//[[nodiscard]] Sim::CyclePhase phase() const;
		// void setPhase(Sim::CyclePhase value);

	private:
		bool initEeprom();

	public:
		void fault(QString reasone, QString func);

	private:
		bool processOffMode();
		bool processStartMode();
		bool processFaultMode();

		bool processOperate(std::chrono::microseconds currentTime, const QDateTime& currentDateTime, qint64 workcycle);

	public:
		bool receiveConnectionsData(std::chrono::microseconds currentTime); // This one is public to be called from Sim::Control
		bool sendConnectionsData(std::chrono::microseconds currentTime);    // Actually this one is private

	private:
		// Props
		//
	public:
		ScopedLog& log();

		const QString& equipmentId() const;
		Hash equipmentIdHash() const;

		int buildNo() const;

		Hardware::LogicModuleInfo logicModuleInfo() const;
		void setLogicModuleInfo(const Hardware::LogicModuleInfo& lmInfo);

		const ::LogicModuleInfo& logicModuleExtraInfo() const;
		void setLogicModuleExtraInfo(const ::LogicModuleInfo& value);

		const LmDescription& lmDescription() const;

		[[nodiscard]] QString sharedMemoryKey() const;

		// const Lans& lans() const;

		[[nodiscard]] RuntimeMode runtimeMode() const;
		void setRuntimeMode(RuntimeMode value);

		[[nodiscard]] DeviceState deviceState() const;

	private:
		void setDeviceState(DeviceState value);

		// External discrete inputs and outputs
		// class VduDeviceEmulator provides inputs with validities,
		// Vdu Sim App provides outputs.
		//
	public:
		[[nodiscard]] bool discreteInputState(size_t index) const;
		void setDiscreteInputState(size_t index, bool value);

		[[nodiscard]] bool discreteInputValidity(size_t index) const;
		void setDiscreteInputValidity(size_t index, bool value);

		static constexpr int INPUTS_COUNT = SimVduSharedData::DiscreteInputCount;
		static constexpr int OUTPUTS_COUNT = SimVduSharedData::DiscreteOutputCount;

		[[nodiscard]] std::optional<bool> discreteOutputState(size_t index) const;
		[[nodiscard]] std::array<std::optional<bool>, VduDeviceEmulator::OUTPUTS_COUNT> discreteOutputStates() const;

		void setDiscreteOutputState(size_t index, std::optional<bool> value);
		void setDiscreteOutputStates(const std::array<std::optional<bool>, VduDeviceEmulator::OUTPUTS_COUNT>& values);

		// Data
		//
	private:
		class SimulatorPrivate* m_simulator = nullptr;
		mutable ScopedLog m_log;

		Hardware::LogicModuleInfo m_logicModuleInfo;
		Hash m_logicModuleIdHash = UNDEFINED_HASH; // hash from m_logicModuleInfo.equipmentId.

		LmDescription m_lmDescription;
		::LogicModuleInfo m_logicModuleExtraInfo;  // Require mostly for LANs

		Eeprom m_confEeprom = Eeprom(UartId::Configuration);

		// Current state
		//
		std::atomic<RuntimeMode> m_runtimeMode = RuntimeMode::PoweredOffMode;
		std::atomic<DeviceState> m_deviceState2 = DeviceState::Off; // The only place where it can be accessed in concurrent mode is
																	// powerOff/reset powerOff can be called while simulation is running
																	// reset can be called to restart module after powerOff
		std::vector<ConnectionImplPtr> m_connections;

		// Lans m_lans{this, m_simulator};     // Device LAN Interfaces, based on m_logicModuleExtraInfo

		mutable std::mutex m_insOutsMutex; // Protects m_discreteInputStates, m_discreteInputValidities, m_discreteOutputStates
		std::array<bool, INPUTS_COUNT> m_discreteInputStates;                  // External keys
		std::array<bool, INPUTS_COUNT> m_discreteInputValidities;              // External keys
		std::array<std::optional<bool>, OUTPUTS_COUNT> m_discreteOutputStates; // VDU outputs, received from VDU Simulator App.

		// Cached state
		//
		mutable QMutex m_cacheMutex;

		Hardware::LogicModuleInfo m_cachedLogicModuleInfo;

		// --
		//
		struct
		{
			QString memoryKey;
			std::optional<QSharedMemory> clientSharedMemory;

			SimVduSharedData::SimulatorState simulatorState{};

			uint64_t lastOptoFromVduCounter = 0; // Last update counter from VDU, to detect new data
		} m_shared;
	};
} // namespace Sim
