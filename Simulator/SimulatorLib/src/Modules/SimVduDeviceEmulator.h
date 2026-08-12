#pragma once

#include "../SimConnectionsImpl.h"
#include "../SimEeprom.h"
#include "../SimScopedLog.h"
// #include "../SimLans.h"
#include "SimDeviceEmulatorCommon.h"

#include <SimulatorLib/SimDeviceState.h>
// #include <SimulatorLib/SimDeviceCommand.h>
// #include <SimulatorLib/SimRam.h>

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

#if 0
		// RAM access
		//
		bool movRamMem(quint32 src, quint32 dst, quint32 sizeW);
		bool movRamMem(Ram::Handle memoryAreaHandleSrc, quint32 src, Ram::Handle memoryAreaHandleDst, quint32 dst, quint32 sizeW);

		bool setRamMem(quint32 address, quint16 data, quint16 size);
		bool setRamMem(Ram::Handle memoryAreaHandle, quint32 address, quint16 data, quint16 size);

		bool writeRamBit(quint32 offsetW, quint16 bitNo, quint16 data);
		bool writeRamBit(Ram::Handle memoryAreaHandle, quint32 offsetW, quint16 bitNo, quint16 data);

		quint16 readRamBit(quint32 offsetW, quint16 bitNo);
		quint16 readRamBit(Ram::Handle memoryAreaHandle, quint32 offsetW, quint16 bitNo);

		bool writeRamBit(quint32 offsetW, quint16 bitNo, quint16 data, E::LogicModuleRamAccess access);
		quint16 readRamBit(quint32 offsetW, quint16 bitNo, E::LogicModuleRamAccess access);

		bool writeRamWord(quint32 offsetW, quint16 data);
		bool writeRamWord(Ram::Handle memoryAreaHandle, quint32 offsetW, quint16 data);

		quint16 readRamWord(quint32 offsetW);
		quint16 readRamWord(Ram::Handle memoryAreaHandle, quint32 offsetW);

		bool writeRamWord(quint32 offsetW, quint16 data, E::LogicModuleRamAccess access);
		quint16 readRamWord(quint32 offsetW, E::LogicModuleRamAccess access);

		bool writeRamDword(quint32 offsetW, quint32 data);
		bool writeRamDword(Ram::Handle memoryAreaHandle, quint32 offsetW, quint32 data);

		quint32 readRamDword(quint32 offsetW);
		quint32 readRamDword(Ram::Handle memoryAreaHandle, quint32 offsetW);

		bool writeRamDword(quint32 offsetW, quint32 data, E::LogicModuleRamAccess access);
		quint32 readRamDword(quint32 offsetW, E::LogicModuleRamAccess access);

		// Getting data from m_plainAppLogic
		//
		quint16 getWord(int wordOffset) const;
		quint32 getDword(int wordOffset) const;
#endif

	private:
		bool initMemory();
		// bool initValiditySignals(); // Sets validity for input signals to 1. Users do not want to set many validities on simulation
		// start.

		bool initEeprom();
		// bool parseAppLogicCode();
		// bool parseCommand(const LmCommand& command, int programCounter);

	public:
		void fault(QString reasone, QString func);

	private:
#if 0
		bool processOffMode();
		bool processStartMode();
		bool processFaultMode();

		bool processOperate(std::chrono::microseconds currentTime, const QDateTime& currentDateTime, qint64 workcycle);

		bool runCommand(EmulatorCommand& deviceCommand);
#endif

	public:
		bool receiveConnectionsData(std::chrono::microseconds currentTime); // This one is public to be called from Sim::Control
		bool sendConnectionsData(std::chrono::microseconds currentTime);    // Actually this one is private

	private:
		//// Getting data from m_plainAppLogic
		////
		// template<typename TYPE>
		// TYPE getData(int eepromOffset) const;

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

		// const Ram& ram() const;
		// Ram& mutableRam();

		// const Lans& lans() const;

		//[[nodiscard]] RuntimeMode runtimeMode() const;
		// void setRuntimeMode(RuntimeMode value);

		//[[nodiscard]] DeviceState deviceState() const;

	private:
		void setDeviceState(DeviceState value);

	public:
#if 0
		[[nodiscard]] bool armingKey() const;
		void setArmingKey(bool value);

		[[nodiscard]] bool tuningKey() const;
		void setTuningKey(bool value);

		[[nodiscard]] bool sorIsSet() const;
		void setSorIsSet(bool value);

		[[nodiscard]] bool sorSetSwitch1() const;
		void setSorSetSwitch1(bool value);

		[[nodiscard]] bool sorSetSwitch2() const;
		void setSorSetSwitch2(bool value);

		[[nodiscard]] bool sorSetSwitch3() const;
		void setSorSetSwitch3(bool value);

		[[nodiscard]] bool testSorResetSwitch(bool newValue);
#endif

		// Data
		//
	private:
		class SimulatorPrivate* m_simulator = nullptr;
		mutable ScopedLog m_log;

		Hardware::LogicModuleInfo m_logicModuleInfo;
		Hash m_logicModuleIdHash = UNDEFINED_HASH; // hash from m_logicModuleInfo.equipmentId.

		LmDescription m_lmDescription;
		::LogicModuleInfo m_logicModuleExtraInfo;

		Eeprom m_confEeprom = Eeprom(UartId::Configuration);

		// QByteArray m_plainAppLogic;   // Just AppLogic data for specific m_logicModuleNumber and cleaned CRCs

		// Current state
		//
		std::atomic<RuntimeMode> m_runtimeMode = RuntimeMode::PoweredOffMode;
		std::atomic<DeviceState> m_deviceState2 = DeviceState::Off; // The only place where it can be accessed in concurrent mode is
																	// powerOff/reset powerOff can be called while simulation is running
																	// reset can be called to restart module after powerOff

		// Ram m_ram;
		// LogicUnitData m_logicUnit;
		std::vector<ConnectionImplPtr> m_connections;

		// std::vector<EmulatorCommand> m_commands;

		// Lans m_lans{this, m_simulator};     // Device LAN Interfaces, based on m_logicModuleExtraInfo

		// Tuning
		//
		// std::atomic<bool> m_armingKey{false}; // External Key
		// std::atomic<bool> m_tuningKey{false}; // Extrenal Key

		// External SOR Set Keys
		//
		// std::atomic<bool> m_sorIsSet{false};       // Cached value of signal SOR is Set from module RAM

		// std::atomic<bool> m_sorSetSwitch1{false};  // External Key
		// std::atomic<bool> m_sorSetSwitch2{false};  // External Key
		// std::atomic<bool> m_sorSetSwitch3{false};  // External Key

		// std::atomic<bool> m_sorResetSwitch{false}; // External Key

		// Cached state
		//
		mutable QMutex m_cacheMutex;

		Hardware::LogicModuleInfo m_cachedLogicModuleInfo;
	};
} // namespace Sim
