#include "LmModel.h"
#include "../lib/ModuleFirmware.h"

namespace Sim
{

	LogicModule::LogicModule(const Output& output) :
		Output(output, "LogicModule")
	{
	}

	LogicModule::~LogicModule()
	{
		powerOff();
		return;
	}


	bool LogicModule::load(const Hardware::LogicModuleInfo& lmInfo,
						   const LmDescription& lmDescription,
						   const Hardware::ModuleFirmware& firmware,
						   const QString& simulationScript)
	{
		setOutputScope(QString("LM %1").arg(lmInfo.equipmentId));

		clear();

		m_logicModuleInfo = lmInfo;
		m_simulationScript = simulationScript;
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

		return ok;
	}

	void LogicModule::clear()
	{
		writeMessage(QObject::tr("Clear."));

		m_lmDescription.clear();

		m_tuningEeprom.clear();
		m_confEeprom.clear();
		m_appLogicEeprom.clear();

		m_simulationScript.clear();

		return;
	}

	bool LogicModule::powerOn(bool autoStart)
	{
		writeMessage(tr("PowerOn, autoStart = %1").arg(autoStart));

		if (m_workerThread.isRunning() == true)
		{
			writeWaning(tr("PowerOn, previous device emulation is in progress, device will be stopped and new emulation will start."));
			powerOff();
			assert(m_workerThread.isFinished());
		}

		m_device = new DeviceEmulator(*this);

		bool ok = m_device->init(m_logicModuleInfo,
								 m_lmDescription,
								 m_tuningEeprom,
								 m_confEeprom,
								 m_appLogicEeprom,
								 m_simulationScript);

		if (ok == false)
		{
			delete m_device;
			m_device = nullptr;
			return false;
		}

		m_device->moveToThread(&m_workerThread);

		connect(&m_workerThread, &QThread::finished, m_device, &QObject::deleteLater);

		connect(this, &LogicModule::signal_pause, m_device, &DeviceEmulator::pause, Qt::QueuedConnection);
		connect(this, &LogicModule::signal_start, m_device, &DeviceEmulator::start, Qt::QueuedConnection);

		m_workerThread.start();

		if (autoStart == true)
		{
			start();
		}

		return true;
	}

	bool LogicModule::powerOff()
	{
		m_workerThread.quit();
		m_workerThread.wait();

		return true;
	}

	bool LogicModule::pause()
	{
		if (m_workerThread.isRunning() == true)
		{
			return false;
		}

		emit signal_pause();
		return true;
	}

	bool LogicModule::start(int cycles /*= -1*/)
	{
		if (m_workerThread.isRunning() == false)
		{
			return false;
		}

		emit signal_start(cycles);
		return true;
	}

	bool LogicModule::step()
	{
		return start(1);
	}

	bool LogicModule::isPowerOn() const
	{
		return m_workerThread.isRunning();
	}

	bool LogicModule::isFaultMode() const
	{
		if (isPowerOn() == false)
		{
			return true;
		}

		// To do
		//
		int ToDo = 0;

		return false;
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

	const Hardware::LogicModuleInfo& LogicModule::logicModuleInfo() const
	{
		return m_logicModuleInfo;
	}
}
