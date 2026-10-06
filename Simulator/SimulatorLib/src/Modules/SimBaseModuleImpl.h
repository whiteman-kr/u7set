#pragma once

#include "../SimEeprom.h"
#include "../SimScopedLog.h"

#include <SimulatorLib/SimDeviceState.h>
#include <SimulatorLib/SimModuleJumpers.h>

#include <HardwareLib/LmDescription.h>
#include <HardwareLib/ModuleFirmware.h>


namespace Sim
{
	class SimulatorPrivate;

	//
	// class BaseModuleImpl
	//
	class BaseModuleImpl : public QObject
	{
		Q_OBJECT

	public:
		explicit BaseModuleImpl(SimulatorPrivate* simulator);
		virtual ~BaseModuleImpl();

	public:
		// Running LM/VDU/ACM..., start running one cycle.
		//
		virtual QFuture<bool> asyncRunCycle(std::chrono::microseconds currentTime,
											const QDateTime& currentDateTime,
											qint64 workcycle,
											bool reset,
											std::condition_variable& cvFinished);

		virtual bool receiveConnectionsData(std::chrono::microseconds currentTime);

		virtual std::chrono::microseconds cycleDuration() const;

		[[nodiscard]] virtual bool isPowerOff() const;
		virtual void setPowerOff(bool value);

		[[nodiscard]] virtual DeviceState deviceState() const;

	protected:
		bool loadEeprom(const Hardware::ModuleFirmware& firmware, int uartId, Eeprom* eeprom);

	public:
		const QString& equipmentId() const;

		QString subsystemId() const;
		int lmNumber() const;
		E::Channel channel() const;

		ModuleJumpers moduleJumpers() const;
		void setModuleJumpers(ModuleJumpers jumpers);

		const Hardware::LogicModuleInfo& logicModuleInfo() const;

		LmDescription& lmDescription();
		const LmDescription& lmDescription() const;

	protected:
		SimulatorPrivate* m_simulator = nullptr;
		mutable ScopedLog m_log;

		Hardware::LogicModuleInfo m_logicModuleInfo;
		LmDescription m_lmDescription;

		ModuleJumpers m_moduleJumpers;
	};
} // namespace Sim
