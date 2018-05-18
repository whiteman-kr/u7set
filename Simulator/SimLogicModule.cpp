#include "SimLogicModule.h"
#include "../lib/ModuleFirmware.h"

namespace Sim
{

	LogicModule::LogicModule() :
		Output("LogicModule")
	{
		connect(&m_device, &DeviceEmulator::appCodeParsed, this, &LogicModule::slot_appCodeParsed);
		connect(&m_device, &DeviceEmulator::faulted, this, &LogicModule::faulted);

		return;
	}

	LogicModule::~LogicModule()
	{
		return;
	}

	bool LogicModule::load(const Hardware::LogicModuleInfo& lmInfo,
						   const LmDescription& lmDescription,
						   const Hardware::ModuleFirmware& firmware)
	{
		setOutputScope(QString("LM %1").arg(lmInfo.equipmentId));

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
		ok &= m_device.init(m_logicModuleInfo,
							m_lmDescription,
							m_tuningEeprom,
							m_confEeprom,
							m_appLogicEeprom);

		return ok;
	}

	void LogicModule::clear()
	{
		writeMessage(QObject::tr("Clear."));

		m_lmDescription.clear();

		m_tuningEeprom.clear();
		m_confEeprom.clear();
		m_appLogicEeprom.clear();

		m_device.clear();

		return;
	}

	QFuture<bool> LogicModule::asyncRunCycle(std::chrono::microseconds /*currentTime*/, bool reset)
	{
		if (reset == true)
		{
			m_device.reset();
		}

		auto result = QtConcurrent::run<bool>(&m_device, &DeviceEmulator::run, 1);
		return result;
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
			writeError(QObject::tr("Loading eeprom data error, UartID = %1").arg(uartId));
			return false;
		}

		ok = eeprom->init(data);
		if (ok == false)
		{
			writeError(QObject::tr("LogicModule: Loading EEPROM error"));
			return false;
		}

		return true;
	}

	void LogicModule::slot_appCodeParsed(bool ok)
	{
		if (ok == true)
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

	QString LogicModule::equipmentId() const
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

}
