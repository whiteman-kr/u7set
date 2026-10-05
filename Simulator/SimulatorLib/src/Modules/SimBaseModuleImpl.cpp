#include "SimBaseModuleImpl.h"

#include "../SimulatorPrivate.h"

#include <HardwareLib/ModuleFirmware.h>


namespace Sim
{
	//
	// class BaseModuleImpl
	//
	BaseModuleImpl::BaseModuleImpl(SimulatorPrivate* simulator) :
		m_simulator(simulator),
		m_log(simulator->log(), "BaseModuleImpl")
	{
	}

	BaseModuleImpl::~BaseModuleImpl() = default;

	QFuture<bool> BaseModuleImpl::asyncRunCycle(std::chrono::microseconds currentTime,
												const QDateTime& currentDateTime,
												qint64 workcycle,
												bool reset,
												std::condition_variable& cvFinished)
	{
		Q_UNUSED(currentTime);
		Q_UNUSED(currentDateTime);
		Q_UNUSED(workcycle);
		Q_UNUSED(reset);
		Q_UNUSED(cvFinished);

		assert(false); // This method should be overridden in derived classes
		return {};
	}

	bool BaseModuleImpl::receiveConnectionsData(std::chrono::microseconds currentTime)
	{
		Q_UNUSED(currentTime);

		assert(false); // This method should be overridden in derived classes
		return {};
	}

	std::chrono::microseconds BaseModuleImpl::cycleDuration() const
	{
		assert(false); // This method should be overridden in derived classes
		return {};
	}

	bool BaseModuleImpl::isPowerOff() const
	{
		assert(false); // This method should be overridden in derived classes
		return {};
	}

	void BaseModuleImpl::setPowerOff(bool value)
	{
		Q_UNUSED(value);

		assert(false); // This method should be overridden in derived classes
		return;
	}

	DeviceState BaseModuleImpl::deviceState() const
	{
		assert(false); // This method should be overridden in derived classes
		return {};
	}

	bool BaseModuleImpl::loadEeprom(const Hardware::ModuleFirmware& firmware, int uartId, Eeprom* eeprom)
	{
		if (eeprom == nullptr)
		{
			assert(eeprom);
			return false;
		}

		if (subsystemId() != firmware.subsysId())
		{
			m_log.writeError(tr("Loading eeprom data error, SubsystemID mismatch, SubsystemID = %1, Firmware SubsystemID = %2")
								 .arg(subsystemId())
								 .arg(firmware.subsysId()));
			return false;
		}

		bool ok = true;

		const Hardware::ModuleFirmwareData& data = firmware.firmwareData(uartId, &ok);
		if (ok == false)
		{
			m_log.writeError(tr("Loading eeprom data error, UartID = %1").arg(uartId));
			return false;
		}

		ok = eeprom->init(data);
		if (ok == false)
		{
			m_log.writeError(tr("LogicModule: Loading EEPROM error"));
			return false;
		}

		return true;
	}

	const QString& BaseModuleImpl::equipmentId() const
	{
		return m_logicModuleInfo.equipmentId;
	}

	QString BaseModuleImpl::subsystemId() const
	{
		return m_logicModuleInfo.subsystemId;
	}

	int BaseModuleImpl::lmNumber() const
	{
		return m_logicModuleInfo.lmNumber;
	}

	E::Channel BaseModuleImpl::channel() const
	{
		return m_logicModuleInfo.channel;
	}

	ModuleJumpers BaseModuleImpl::moduleJumpers() const
	{
		return m_moduleJumpers;
	}

	void BaseModuleImpl::setModuleJumpers(ModuleJumpers jumpers)
	{
		m_moduleJumpers = std::move(jumpers);
	}

	const Hardware::LogicModuleInfo& BaseModuleImpl::logicModuleInfo() const
	{
		return m_logicModuleInfo;
	}

	LmDescription& BaseModuleImpl::lmDescription()
	{
		return m_lmDescription;
	}

	const LmDescription& BaseModuleImpl::lmDescription() const
	{
		return m_lmDescription;
	}
} // namespace Sim
