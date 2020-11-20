#include "SimLogicModule.h"
#include "../lib/ModuleFirmware.h"
#include "Simulator.h"

namespace Sim
{

	LogicModule::LogicModule(Simulator* simulator) :
		m_simulator(simulator),
		m_log(simulator->log(), "LogicModule")
	{
	}

	LogicModule::~LogicModule()
	{
		return;
	}

	bool LogicModule::load(const Hardware::LogicModuleInfo& lmInfo,
						   const LmDescription& lmDescription,
						   const Hardware::ModuleFirmware& firmware,
						   const Connections& connections,
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

	void LogicModule::clear()
	{
		m_log.writeDebug("Clear");

		m_lmDescription.clear();

		m_tuningEeprom.clear();
		m_confEeprom.clear();
		m_appLogicEeprom.clear();

		m_device.clear();

		return;
	}

	QFuture<bool> LogicModule::asyncRunCycle(std::chrono::microseconds currentTime, const QDateTime& currentDateTime, qint64 workcycle, bool reset)
	{
		if (reset == true)
		{
			m_device.reset();
		}

		return QtConcurrent::run<bool>(&m_device, &DeviceEmulator::runWorkcycle, currentTime, currentDateTime, workcycle);
	}

	bool LogicModule::receiveConnectionsData(std::chrono::microseconds currentTime)
	{
		return m_device.receiveConnectionsData(currentTime);
	}

	bool LogicModule::loadEeprom(const Hardware::ModuleFirmware& firmware, int uartId, Eeprom* eeprom)
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

	void LogicModule::setAppCommands(bool set)
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

	const QString& LogicModule::equipmentId() const
	{
		return m_logicModuleInfo.equipmentId;
	}

	int LogicModule::lmNumber() const
	{
		return m_logicModuleInfo.lmNumber;
	}

	E::Channel LogicModule::channel() const
	{
		return m_logicModuleInfo.channel;
	}

	const Hardware::LogicModuleInfo& LogicModule::logicModuleInfo() const
	{
		return m_logicModuleInfo;
	}

	LmDescription& LogicModule::lmDescription()
	{
		return m_lmDescription;
	}

	const LmDescription& LogicModule::lmDescription() const
	{
		return m_lmDescription;
	}

	const ::LogicModuleInfo& LogicModule::logicModuleExtraInfo() const
	{
		return m_device.logicModuleExtraInfo();
	}

	void LogicModule::setLogicModuleExtraInfo(const ::LogicModuleInfo& value)
	{
		m_device.setLogicModuleExtraInfo(value);
	}

	std::chrono::microseconds LogicModule::cycleDuration() const
	{
		return std::chrono::microseconds{m_lmDescription.logicUnit().m_cycleDuration};
	}

	const std::vector<DeviceCommand>& LogicModule::appCommands() const
	{
		return m_commands;
	}

	std::unordered_map<int, size_t> LogicModule::offsetToCommand() const
	{
		return m_offsetToCommand;
	}

	const DeviceCommand& LogicModule::offsetToCommand(int offset) const
	{
		auto it = m_offsetToCommand.find(offset);

		if (it == m_offsetToCommand.end())
		{
			static LmCommand fakeLmCommand;
			static DeviceCommand fakeCommand(fakeLmCommand);
			return fakeCommand;
		}

		size_t index = it->second;
		return m_commands[index];
	}

	const Ram& LogicModule::ram() const
	{
		return m_device.ram();
	}

	Ram& LogicModule::mutableRam()
	{
		return m_device.mutableRam();
	}

	RuntimeMode LogicModule::runtimeMode() const
	{
		return m_device.runtimeMode();
	}

	DeviceState LogicModule::deviceState() const
	{
		return m_device.deviceState();
	}

	bool LogicModule::isPowerOff() const
	{
		return deviceState() == DeviceState::Off;
	}

	void LogicModule::setPowerOff(bool value)
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
//			if (isPowerOff() == true)
//			{
//				m_device.reset();
//			}
		}
	}

	bool LogicModule::armingKey() const
	{
		return m_device.armingKey();
	}

	void LogicModule::setArmingKey(bool value)
	{
		return m_device.setArmingKey(value);
	}

	bool LogicModule::tuningKey() const
	{
		return m_device.tuningKey();
	}

	void LogicModule::setTuningKey(bool value)
	{
		return m_device.setTuningKey(value);
	}

}
