#pragma once

#include "../SimEeprom.h"
#include "SimBaseModuleImpl.h"
#include "SimLmDeviceEmulator.h"

#include <HardwareLib/LmDescription.h>
#include <HardwareLib/LogicModulesInfo.h>
#include <HardwareLib/ModuleFirmware.h>


namespace Sim
{
	class SimulatorPrivate;
	class ConnectionsImpl;

	class LogicModuleImpl : public BaseModuleImpl
	{
		Q_OBJECT

		friend class Snapshot;

	public:
		explicit LogicModuleImpl(SimulatorPrivate* simulator);
		virtual ~LogicModuleImpl();

	public:
		bool load(const Hardware::LogicModuleInfo& lmInfo,
				  const LmDescription& lmDescription,
				  const Hardware::ModuleFirmware& firmware,
				  const ConnectionsImpl& connections,
				  const LogicModulesInfo& logicModulesExtraInfo);

		void clear();

		// Running LM, start running one cycle.
		//
		virtual QFuture<bool> asyncRunCycle(std::chrono::microseconds currentTime,
											const QDateTime& currentDateTime,
											qint64 workcycle,
											bool reset,
											std::condition_variable& cvFinished) override;

		virtual bool receiveConnectionsData(std::chrono::microseconds currentTime) override;

	protected slots:
		void setAppCommands(bool ok);

	public:
		const ::LogicModuleInfo& logicModuleExtraInfo() const;
		void setLogicModuleExtraInfo(const ::LogicModuleInfo& value);

		virtual std::chrono::microseconds cycleDuration() const override;

		const Eeprom& tuningEeprom() const;
		const Eeprom& confEeprom() const;
		const Eeprom& appLogicEeprom() const;

		const std::vector<DeviceCommand>& appCommands() const;

		std::unordered_map<int, size_t> offsetToCommand() const;
		const DeviceCommand& offsetToCommand(int offset) const;

		[[nodiscard]] const Ram& ram() const; // This RAM access is not protected by any mutex, use it only when no concurrent
											  // thread is accessing it!
		[[nodiscard]] Ram& mutableRam();      // This RAM access is not protected by any mutex, use it only when no concurrent thread is
											  // accessing it!

		[[nodiscard]] RuntimeMode runtimeMode() const;
		[[nodiscard]] virtual DeviceState deviceState() const override;

		[[nodiscard]] virtual bool isPowerOff() const override;
		virtual void setPowerOff(bool value) override;

		[[nodiscard]] bool armingKey() const;
		void setArmingKey(bool value);

		[[nodiscard]] bool tuningKey() const;
		void setTuningKey(bool value);

		[[nodiscard]] bool sorIsSet() const;

		[[nodiscard]] bool sorSetSwitch1() const;
		void setSorSetSwitch1(bool value);

		[[nodiscard]] bool sorSetSwitch2() const;
		void setSorSetSwitch2(bool value);

		[[nodiscard]] bool sorSetSwitch3() const;
		void setSorSetSwitch3(bool value);

		bool testSorResetSwitch(bool newValue);

	private:
		// Loaded LM data
		//
		Eeprom m_tuningEeprom = Eeprom(UartId::Tuning);
		Eeprom m_confEeprom = Eeprom(UartId::Configuration);
		Eeprom m_appLogicEeprom = Eeprom(UartId::ApplicationLogic);

		// Running Emulation
		//
		DeviceEmulator m_device{m_simulator};

		// --
		//
		std::vector<DeviceCommand> m_commands;

		std::unordered_map<int, size_t> m_offsetToCommand; // key: command offset, value: index in m_commands
	};

} // namespace Sim
