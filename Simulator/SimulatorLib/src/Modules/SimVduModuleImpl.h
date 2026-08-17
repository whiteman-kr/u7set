#pragma once

#include "../SimEeprom.h"
#include "SimBaseModuleImpl.h"
#include "SimVduDeviceEmulator.h"

#include <HardwareLib/LmDescription.h>
#include <HardwareLib/LogicModulesInfo.h>
#include <HardwareLib/ModuleFirmware.h>


namespace Sim
{
	class SimulatorPrivate;
	class ConnectionsImpl;

	class VduModuleImpl : public BaseModuleImpl
	{
		Q_OBJECT

		friend class Snapshot;

	public:
		explicit VduModuleImpl(SimulatorPrivate* simulator);
		virtual ~VduModuleImpl();

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

	public:
		const ::LogicModuleInfo& logicModuleExtraInfo() const;
		void setLogicModuleExtraInfo(const ::LogicModuleInfo& value);

		virtual std::chrono::microseconds cycleDuration() const override;

		const Eeprom& confEeprom() const;

		[[nodiscard]] RuntimeMode runtimeMode() const;
		[[nodiscard]] virtual DeviceState deviceState() const override;

		[[nodiscard]] virtual bool isPowerOff() const override;
		virtual void setPowerOff(bool value) override;

		//[[nodiscard]] bool armingKey() const;
		// void setArmingKey(bool value);

		//[[nodiscard]] bool tuningKey() const;
		// void setTuningKey(bool value);

	private:
		// Loaded LM data
		//
		Eeprom m_confEeprom = Eeprom(UartId::Configuration);

		// Running Emulation
		//
		VduDeviceEmulator m_device{m_simulator};
	};
} // namespace Sim
