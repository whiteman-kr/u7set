#pragma once

#include "../SimConnectionsImpl.h"
#include "../SimEeprom.h"
#include "../SimScopedLog.h"
// #include "../SimLans.h"
#include "SimDeviceEmulatorCommon.h"

#include <SimulatorLib/SimDeviceState.h>

#include <HardwareLib/LmDescription.h>
#include <HardwareLib/LogicModulesInfo.h>
#include <HardwareLib/ModuleFirmware.h>

namespace SimVduSharedData
{
	constexpr size_t OptoBufferMaxSize = 2560 * 2;
	constexpr size_t OptoPortCount = 8;

	struct SimulatorState
	{
		uint64_t updateCounter; // The update counter, incremented by simulator on each update
		int32_t deviceState;    // enum class DeviceState (0 - Off, Start, Faut, Operate)
		int32_t reserved0;      // Reserved for future use (padding to 8 bytes)
		uint64_t plantTime;     // The plant time in microseconds
	};

	struct OptoBuffer
	{
		uint64_t timeout;       //
		uint64_t occupied;      // The number of bytes occupied in the buffer
		uint8_t data[OptoBufferMaxSize];
	};

	struct OptoPorts
	{
		uint64_t updateCounter;
		OptoBuffer portData[OptoPortCount]; // Data from simulator to VDU
	};

	struct SharedData
	{
		uint64_t size;                      // The size of this structure, must be filled first

		SimulatorState simulatorState;      // State of the simulator

		OptoPorts toVdu;                    // Data from simulator to VDU
		OptoPorts fromVdu;                  // Data from VDU to simulator
	};

	static_assert(std::is_standard_layout_v<SharedData> && std::is_trivial_v<SharedData>, "SharedData must be POD type");
} // namespace SimVduSharedData


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

		// const Lans& lans() const;

		[[nodiscard]] RuntimeMode runtimeMode() const;
		void setRuntimeMode(RuntimeMode value);

		[[nodiscard]] DeviceState deviceState() const;

	private:
		void setDeviceState(DeviceState value);

	public:
#if 0
		[[nodiscard]] bool armingKey() const;
		void setArmingKey(bool value);

		[[nodiscard]] bool tuningKey() const;
		void setTuningKey(bool value);
#endif

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

		// std::atomic<bool> m_armingKey{false}; // External Key
		// std::atomic<bool> m_tuningKey{false}; // Extrenal Key

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
