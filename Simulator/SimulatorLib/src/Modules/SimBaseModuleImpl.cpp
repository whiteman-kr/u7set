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
