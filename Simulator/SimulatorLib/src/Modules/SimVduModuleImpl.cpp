#include "SimVduModuleImpl.h"

#include "../SimulatorPrivate.h"
#include <HardwareLib/ModuleFirmware.h>


namespace Sim
{
	//
	// class VduModuleImpl
	//
	VduModuleImpl::VduModuleImpl(SimulatorPrivate* simulator) :
		BaseModuleImpl{simulator}
	{
		m_log.setOutputScope("VduModule");
	}

	VduModuleImpl::~VduModuleImpl() = default;

	bool VduModuleImpl::load(const Hardware::LogicModuleInfo& lmInfo,
							 const LmDescription& lmDescription,
							 const Hardware::ModuleFirmware& firmware,
							 const ConnectionsImpl& connections,
							 const LogicModulesInfo& logicModulesExtraInfo)
	{
		m_log.setOutputScope(QString("VDU %1").arg(lmInfo.equipmentId));

		clear();

		m_logicModuleInfo = lmInfo;
		m_lmDescription = lmDescription;

		bool ok = true;

		if (m_lmDescription.flashMemory().m_configWriteBitstream == true)
		{
			bool loadEepromOk = loadEeprom(firmware, m_lmDescription.flashMemory().m_configUartId, &m_confEeprom);
			if (loadEepromOk == false)
			{
				m_log.writeError(tr("LogicModule: Loading Configuration EEPROM error"));
				ok = false;
			}
		}

		if (ok == false)
		{
			return false;
		}

		// Init DeviceEmulator
		//
		QString subsystemPath = m_simulator->buildPath() + "Subsytems/" + m_logicModuleInfo.subsystemId;
		DeviceError de = m_device.init(m_logicModuleInfo, m_lmDescription, m_confEeprom, subsystemPath, connections, logicModulesExtraInfo);

		if (de != DeviceError::Ok)
		{
			ok = false;
		}

		return ok;
	}

	void VduModuleImpl::clear()
	{
		m_log.writeDebug("Clear");

		m_lmDescription.clear();
		m_confEeprom.clear();
		m_device.clear();

		return;
	}

	QFuture<bool> VduModuleImpl::asyncRunCycle(std::chrono::microseconds currentTime,
											   const QDateTime& currentDateTime,
											   qint64 workcycle,
											   bool reset,
											   std::condition_variable& cvFinished)
	{
		if (reset == true)
		{
			m_device.reset();
		}

		auto f = [this, &cvFinished](std::chrono::microseconds currentTime, QDateTime currentDateTime, qint64 workcycle) -> bool
		{
			bool result = this->m_device.runWorkcycle(currentTime, currentDateTime, workcycle);
			cvFinished.notify_one();
			return result;
		};

		return QtConcurrent::run(f, currentTime, currentDateTime, workcycle);
	}

	bool VduModuleImpl::receiveConnectionsData(std::chrono::microseconds currentTime)
	{
		return m_device.receiveConnectionsData(currentTime);
	}

	const ::LogicModuleInfo& VduModuleImpl::logicModuleExtraInfo() const
	{
		return m_device.logicModuleExtraInfo();
	}

	void VduModuleImpl::setLogicModuleExtraInfo(const ::LogicModuleInfo& value)
	{
		m_device.setLogicModuleExtraInfo(value);
	}

	std::chrono::microseconds VduModuleImpl::cycleDuration() const
	{
		return std::chrono::microseconds{m_lmDescription.logicUnit().m_cycleDuration};
	}

	const Eeprom& VduModuleImpl::confEeprom() const
	{
		return m_confEeprom;
	}

	RuntimeMode VduModuleImpl::runtimeMode() const
	{
		return m_device.runtimeMode();
	}

	DeviceState VduModuleImpl::deviceState() const
	{
		return m_device.deviceState();
	}

	bool VduModuleImpl::isPowerOff() const
	{
		return deviceState() == DeviceState::Off;
	}

	void VduModuleImpl::setPowerOff(bool value)
	{
		if (value == true)
		{
			if (isPowerOff() == false)
			{
				m_device.powerOff();
			}
		}
		else
		{
			m_device.reset();
		}
	}

	// bool VduModuleImpl::armingKey() const
	//{
	//	return m_device.armingKey();
	// }

	// void VduModuleImpl::setArmingKey(bool value)
	//{
	//	return m_device.setArmingKey(value);
	// }

	// bool VduModuleImpl::tuningKey() const
	//{
	//	return m_device.tuningKey();
	// }

	// void VduModuleImpl::setTuningKey(bool value)
	//{
	//	return m_device.setTuningKey(value);
	// }
} // namespace Sim
