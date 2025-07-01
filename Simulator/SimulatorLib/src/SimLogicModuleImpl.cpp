#include "SimLogicModuleImpl.h"
#include "SimulatorPrivate.h"
#include <HardwareLib/ModuleFirmware.h>


namespace Sim
{

	LogicModuleImpl::LogicModuleImpl(SimulatorPrivate* simulator) :
		m_simulator(simulator),
		m_log(simulator->log(), "LogicModule")
	{
	}

	LogicModuleImpl::~LogicModuleImpl()
	{
		return;
	}

	bool LogicModuleImpl::load(const Hardware::LogicModuleInfo& lmInfo,
							   const LmDescription& lmDescription,
							   const Hardware::ModuleFirmware& firmware,
							   const ConnectionsImpl& connections,
							   const LogicModulesInfo& logicModulesExtraInfo)
	{
		m_log.setOutputScope(QString("LM %1").arg(lmInfo.equipmentId));

		clear();

		m_logicModuleInfo = lmInfo;
		m_lmDescription = lmDescription;

		bool ok = true;

		if (m_lmDescription.flashMemory().m_tuningWriteBitstream == true)
		{
			ok &= loadEeprom(firmware, m_lmDescription.flashMemory().m_tuningUartId, &m_tuningEeprom);
		}

		if (m_lmDescription.flashMemory().m_configWriteBitstream == true)
		{
			ok &= loadEeprom(firmware, m_lmDescription.flashMemory().m_configUartId, &m_confEeprom);
		}

		if (m_lmDescription.flashMemory().m_appLogicWriteBitstream == true)
		{
			ok &= loadEeprom(firmware, m_lmDescription.flashMemory().m_appLogicUartId, &m_appLogicEeprom);
		}

		// Init DeviceEmulator
		//
		DeviceError de = m_device.init(m_logicModuleInfo,
									   m_lmDescription,
									   m_tuningEeprom,
									   m_confEeprom,
									   m_appLogicEeprom,
									   connections,
									   logicModulesExtraInfo);

		if (de == DeviceError::Ok || de == DeviceError::NoCommandProcessor)
		{
			// DeviceError::NoCommandProcessor is ok, as there are a lot of LM's which are not simulated
			//
		}
		else
		{
			ok = false;
		}

		setAppCommands(ok);

		return ok;
	}

	void LogicModuleImpl::clear()
	{
		m_log.writeDebug("Clear");

		m_lmDescription.clear();

		m_tuningEeprom.clear();
		m_confEeprom.clear();
		m_appLogicEeprom.clear();

		m_device.clear();

		return;
	}

	QFuture<bool> LogicModuleImpl::asyncRunCycle(std::chrono::microseconds currentTime,
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

	bool LogicModuleImpl::receiveConnectionsData(std::chrono::microseconds currentTime)
	{
		return m_device.receiveConnectionsData(currentTime);
	}

	bool LogicModuleImpl::loadEeprom(const Hardware::ModuleFirmware& firmware, int uartId, Eeprom* eeprom)
	{
		if (eeprom == nullptr)
		{
			assert(eeprom);
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

	void LogicModuleImpl::setAppCommands(bool set)
	{
		if (set == true)
		{
			m_commands = m_device.commands();
			m_offsetToCommand = m_device.offsetToCommands();
		}
		else
		{
			m_commands.clear();
			m_offsetToCommand.clear();
		}

		return;
	}

	const QString& LogicModuleImpl::equipmentId() const
	{
		return m_logicModuleInfo.equipmentId;
	}

	int LogicModuleImpl::lmNumber() const
	{
		return m_logicModuleInfo.lmNumber;
	}

	E::Channel LogicModuleImpl::channel() const
	{
		return m_logicModuleInfo.channel;
	}

	const Hardware::LogicModuleInfo& LogicModuleImpl::logicModuleInfo() const
	{
		return m_logicModuleInfo;
	}

	LmDescription& LogicModuleImpl::lmDescription()
	{
		return m_lmDescription;
	}

	const LmDescription& LogicModuleImpl::lmDescription() const
	{
		return m_lmDescription;
	}

	const ::LogicModuleInfo& LogicModuleImpl::logicModuleExtraInfo() const
	{
		return m_device.logicModuleExtraInfo();
	}

	void LogicModuleImpl::setLogicModuleExtraInfo(const ::LogicModuleInfo& value)
	{
		m_device.setLogicModuleExtraInfo(value);
	}

	std::chrono::microseconds LogicModuleImpl::cycleDuration() const
	{
		return std::chrono::microseconds{m_lmDescription.logicUnit().m_cycleDuration};
	}

	const Eeprom& LogicModuleImpl::tuningEeprom() const
	{
		return m_tuningEeprom;
	}

	const Eeprom& LogicModuleImpl::confEeprom() const
	{
		return m_confEeprom;
	}

	const Eeprom& LogicModuleImpl::appLogicEeprom() const
	{
		return m_appLogicEeprom;
	}

	const std::vector<DeviceCommand>& LogicModuleImpl::appCommands() const
	{
		return m_commands;
	}

	std::unordered_map<int, size_t> LogicModuleImpl::offsetToCommand() const
	{
		return m_offsetToCommand;
	}

	const DeviceCommand& LogicModuleImpl::offsetToCommand(int offset) const
	{
		auto it = m_offsetToCommand.find(offset);

		if (it == m_offsetToCommand.end())
		{
			static DeviceCommand fakeCommand;
			return fakeCommand;
		}

		size_t index = it->second;
		return m_commands[index];
	}

	const Ram& LogicModuleImpl::ram() const
	{
		return m_device.ram();
	}

	Ram& LogicModuleImpl::mutableRam()
	{
		return m_device.mutableRam();
	}

	RuntimeMode LogicModuleImpl::runtimeMode() const
	{
		return m_device.runtimeMode();
	}

	DeviceState LogicModuleImpl::deviceState() const
	{
		return m_device.deviceState();
	}

	bool LogicModuleImpl::isPowerOff() const
	{
		return deviceState() == DeviceState::Off;
	}

	void LogicModuleImpl::setPowerOff(bool value)
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
			// if (isPowerOff() == true)
			// {
			// 	m_device.reset();
			// }
		}
	}

	bool LogicModuleImpl::armingKey() const
	{
		return m_device.armingKey();
	}

	void LogicModuleImpl::setArmingKey(bool value)
	{
		return m_device.setArmingKey(value);
	}

	bool LogicModuleImpl::tuningKey() const
	{
		return m_device.tuningKey();
	}

	void LogicModuleImpl::setTuningKey(bool value)
	{
		return m_device.setTuningKey(value);
	}

	bool LogicModuleImpl::sorIsSet() const
	{
		return m_device.sorIsSet();
	}

	bool LogicModuleImpl::sorSetSwitch1() const
	{
		return m_device.sorSetSwitch1();
	}

	void LogicModuleImpl::setSorSetSwitch1(bool value)
	{
		m_device.setSorSetSwitch1(value);
	}

	bool LogicModuleImpl::sorSetSwitch2() const
	{
		return m_device.sorSetSwitch2();
	}

	void LogicModuleImpl::setSorSetSwitch2(bool value)
	{
		m_device.setSorSetSwitch2(value);
	}

	bool LogicModuleImpl::sorSetSwitch3() const
	{
		return m_device.sorSetSwitch3();
	}

	void LogicModuleImpl::setSorSetSwitch3(bool value)
	{
		m_device.setSorSetSwitch3(value);
	}

	bool LogicModuleImpl::testSorResetSwitch(bool newValue)
	{
		return m_device.testSorResetSwitch(newValue);
	}
} // namespace Sim
