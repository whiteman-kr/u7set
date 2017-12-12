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


	bool LogicModule::load(const Hardware::LogicModuleInfo& lmInfo, const LmDescription& lmDescription,
						   const Hardware::ModuleFirmware& firmware,
						   const QString& simulationScript)
	{
		setOutputScope(QString("LM %1").arg(lmInfo.equipmentId));

		clear();

		bool ok = true;

//		ok &= loadLmDescription(lmDescription);
//		if (ok == false)
//		{
//			return false;
//		}

//		assert(m_lmDescription);

//		m_tuningEeprom.init(m_lmDescription->flashMemory().m_tuningFrameSize + 8, m_lmDescription->flashMemory().m_tuningFrameCount, 0xFF);
//		m_confEeprom.init(m_lmDescription->flashMemory().m_configFrameSize + 8, m_lmDescription->flashMemory().m_configFrameCount, 0xFF);
//		m_appLogicEeprom.init(m_lmDescription->flashMemory().m_appLogicFrameSize + 8, m_lmDescription->flashMemory().m_appLogicFrameCount, 0xFF);

//		Hardware::ModuleFirmwareStorage mf;
//		QString errorMessage;

//		ok = mf.load(firmware, &errorMessage);
//		if (ok == false)
//		{
//			output() << "Parse bitstream file error: " << errorMessage << endl;
//			return false;
//		}

//		if (m_lmDescription->flashMemory().m_tuningWriteBitstream == true)
//		{
//			ok &= loadEeprom(mf, &m_tuningEeprom);
//		}

//		if (m_lmDescription->flashMemory().m_configWriteBitstream == true)
//		{
//			ok &= loadEeprom(mf, &m_confEeprom);
//		}

//		if (m_lmDescription->flashMemory().m_appLogicWriteBitstream == true)
//		{
//			ok &= loadEeprom(mf, &m_appLogicEeprom);
//		}

//		m_simulationScript = simulationScript;

		return ok;
	}

	void LogicModule::clear()
	{
		writeMessage(QObject::tr("Clear."));

		m_lmDescription.reset();

		m_tuningEeprom.clear();
		m_confEeprom.clear();
		m_appLogicEeprom.clear();

		return;
	}

	bool LogicModule::powerOn(int logicModuleNumber, bool autoStart)
	{
		writeMessage(tr("PowerOn, autoStart = ").arg(autoStart));

		if (m_workerThread.isRunning() == true)
		{
			writeWaning(tr("PowerOn, previous device emulation is in progress, device will be stopped and new emulation will start."));
			powerOff();
			assert(m_workerThread.isFinished());
		}

//		m_device = new DeviceEmulator(&output());
//		bool ok = m_device->init(logicModuleNumber, *m_lmDescription.get(), m_tuningEeprom, m_confEeprom, m_appLogicEeprom, m_simulationScript);

//		if (ok == false)
//		{
//			delete m_device;
//			m_device = nullptr;
//			return false;
//		}

//		m_device->moveToThread(&m_workerThread);

//		connect(&m_workerThread, &QThread::finished, m_device, &QObject::deleteLater);

//		connect(this, &LogicModule::signal_pause, m_device, &DeviceEmulator::pause, Qt::QueuedConnection);
//		connect(this, &LogicModule::signal_start, m_device, &DeviceEmulator::start, Qt::QueuedConnection);

//		m_workerThread.start();

//		if (autoStart == true)
//		{
//			start();
//		}

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

	bool LogicModule::loadLmDescription(const QByteArray& logicModuleDescription)
	{
		writeMessage(tr("Load Logic Module description."));

		m_lmDescription = std::make_unique<LmDescription>();

		QString errorString;

		bool ok = m_lmDescription->load(logicModuleDescription, &errorString);
		if (ok == false)
		{
			writeMessage(tr("Load Logic Module description error: %1").arg(errorString));
		}

		return ok;
	}

	bool LogicModule::loadEeprom(const Hardware::ModuleFirmware& firmware, Eeprom* eeprom)
	{
		if (eeprom == nullptr)
		{
			assert(eeprom);
			return false;
		}

		QString errorMessage;

//		bool ok = eeprom->loadData(fileData, &errorMessage);
//		if (ok == false)
//		{
//			output() << "LogicModule: Loading EEPROM error: " << errorMessage << endl;
//			return false;
//		}

		return true;
	}

	const Hardware::LogicModuleInfo& LogicModule::logicModuleInfo() const
	{
		return m_logicModuleInfo;
	}
}
