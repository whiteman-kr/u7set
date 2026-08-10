#include "SimVduModuleImpl.h"

#include "SimulatorPrivate.h"
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

		// if (m_lmDescription.flashMemory().m_tuningWriteBitstream == true)
		//{
		//	bool loadEepromOk = loadEeprom(firmware, m_lmDescription.flashMemory().m_tuningUartId, &m_tuningEeprom);
		//	if (loadEepromOk == false)
		//	{
		//		m_log.writeError(tr("LogicModule: Loading Tuning EEPROM error"));
		//		ok = false;
		//	}
		// }

		if (m_lmDescription.flashMemory().m_configWriteBitstream == true)
		{
			bool loadEepromOk = loadEeprom(firmware, m_lmDescription.flashMemory().m_configUartId, &m_confEeprom);
			if (loadEepromOk == false)
			{
				m_log.writeError(tr("LogicModule: Loading Configuration EEPROM error"));
				ok = false;
			}
		}

		// if (m_lmDescription.flashMemory().m_appLogicWriteBitstream == true)
		//{
		//	bool loadEepromOk = loadEeprom(firmware, m_lmDescription.flashMemory().m_appLogicUartId, &m_appLogicEeprom);
		//	if (loadEepromOk == false)
		//	{
		//		m_log.writeError(tr("LogicModule: Loading Application Logic EEPROM error"));
		//		ok = false;
		//	}
		// }

		if (ok == false)
		{
			return false;
		}

		// Init DeviceEmulator
		//
		// DeviceError de = m_device.init(m_logicModuleInfo,
		//							   m_lmDescription,
		//							   m_tuningEeprom,
		//							   m_confEeprom,
		//							   m_appLogicEeprom,
		//							   connections,
		//							   logicModulesExtraInfo);

		// if (de == DeviceError::Ok || de == DeviceError::NoCommandProcessor)
		//{
		//	// DeviceError::NoCommandProcessor is ok, as there are a lot of LM's which are not simulated
		//	//
		// }
		// else
		//{
		//	ok = false;
		// }

		return ok;
	}

	void VduModuleImpl::clear()
	{
		m_log.writeDebug("Clear");

		m_lmDescription.clear();

		// m_tuningEeprom.clear();
		m_confEeprom.clear();
		// m_appLogicEeprom.clear();

		// m_device.clear();

		return;
	}

	QFuture<bool> VduModuleImpl::asyncRunCycle(std::chrono::microseconds currentTime,
											   const QDateTime& currentDateTime,
											   qint64 workcycle,
											   bool reset,
											   std::condition_variable& cvFinished)
	{
		// if (reset == true)
		//{
		//	m_device.reset();
		// }

		// auto f = [this, &cvFinished](std::chrono::microseconds currentTime, QDateTime currentDateTime, qint64 workcycle) -> bool
		//{
		//	bool result = this->m_device.runWorkcycle(currentTime, currentDateTime, workcycle);
		//	cvFinished.notify_one();
		//	return result;
		// };

		// return QtConcurrent::run(f, currentTime, currentDateTime, workcycle);

		return {};
	}

	bool VduModuleImpl::receiveConnectionsData(std::chrono::microseconds currentTime)
	{
		// TODO!!!!
		assert(false);
		return {};
		// return m_device.receiveConnectionsData(currentTime);
	}

	const ::LogicModuleInfo& VduModuleImpl::logicModuleExtraInfo() const
	{
		// TODO!!!!
		assert(false);
		return {};
		// return m_device.logicModuleExtraInfo();
	}

	void VduModuleImpl::setLogicModuleExtraInfo(const ::LogicModuleInfo& value)
	{
		// TODO!!!!
		// m_device.setLogicModuleExtraInfo(value);
	}

	std::chrono::microseconds VduModuleImpl::cycleDuration() const
	{
		return std::chrono::microseconds{m_lmDescription.logicUnit().m_cycleDuration};
	}

	// const Eeprom& VduModuleImpl::tuningEeprom() const
	//{
	//	return m_tuningEeprom;
	// }

	const Eeprom& VduModuleImpl::confEeprom() const
	{
		return m_confEeprom;
	}

	// const Eeprom& VduModuleImpl::appLogicEeprom() const
	//{
	//	return m_appLogicEeprom;
	// }

	// const std::vector<DeviceCommand>& VduModuleImpl::appCommands() const
	//{
	//	return m_commands;
	// }

	// std::unordered_map<int, size_t> VduModuleImpl::offsetToCommand() const
	//{
	//	return m_offsetToCommand;
	// }

	// const DeviceCommand& VduModuleImpl::offsetToCommand(int offset) const
	//{
	//	auto it = m_offsetToCommand.find(offset);

	//	if (it == m_offsetToCommand.end())
	//	{
	//		static DeviceCommand fakeCommand;
	//		return fakeCommand;
	//	}

	//	size_t index = it->second;
	//	return m_commands[index];
	//}

	// const Ram& VduModuleImpl::ram() const
	//{
	//	return m_device.ram();
	// }

	// Ram& VduModuleImpl::mutableRam()
	//{
	//	return m_device.mutableRam();
	// }

	RuntimeMode VduModuleImpl::runtimeMode() const
	{
		// TODO!!!!
		// return m_device.runtimeMode();
		assert(false);
		return {};
	}

	DeviceState VduModuleImpl::deviceState() const
	{
		// TODO!!!!
		// return m_device.deviceState();
		assert(false);
		return {};
	}

	bool VduModuleImpl::isPowerOff() const
	{
		// TODO!!!!
		// return deviceState() == DeviceState::Off;
		assert(false);
		return {};
	}

	void VduModuleImpl::setPowerOff(bool value)
	{
		// TODO!!!!
		assert(false);

		// if (value == true)
		//{
		//	if (isPowerOff() == false)
		//	{
		//		m_device.powerOff();
		//	}
		// }
		// else
		//{
		//	m_device.reset();
		// }
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

	// bool VduModuleImpl::sorIsSet() const
	//{
	//	return m_device.sorIsSet();
	// }

	// bool VduModuleImpl::sorSetSwitch1() const
	//{
	//	return m_device.sorSetSwitch1();
	// }

	// void VduModuleImpl::setSorSetSwitch1(bool value)
	//{
	//	m_device.setSorSetSwitch1(value);
	// }

	// bool VduModuleImpl::sorSetSwitch2() const
	//{
	//	return m_device.sorSetSwitch2();
	// }

	// void VduModuleImpl::setSorSetSwitch2(bool value)
	//{
	//	m_device.setSorSetSwitch2(value);
	// }

	// bool VduModuleImpl::sorSetSwitch3() const
	//{
	//	return m_device.sorSetSwitch3();
	// }

	// void VduModuleImpl::setSorSetSwitch3(bool value)
	//{
	//	m_device.setSorSetSwitch3(value);
	// }

	// bool VduModuleImpl::testSorResetSwitch(bool newValue)
	//{
	//	return m_device.testSorResetSwitch(newValue);
	// }
} // namespace Sim
