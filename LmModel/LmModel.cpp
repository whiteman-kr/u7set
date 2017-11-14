#include "LmModel.h"


namespace LmModel
{

	LogicModule::LogicModule(QTextStream* textStream/* = nullptr*/) :
		m_textStream(textStream),
#ifdef Q_OS_WIN
		m_nullDevice(fopen("NUL:", "w")),
#endif
#ifdef Q_OS_UNIX
		m_nullDevice(fopen("/dev/null", "w")),
#endif
		m_nullTextStream(m_nullDevice)
	{
	}

	LogicModule::~LogicModule()
	{
		powerOff();

		if (m_nullDevice != nullptr)
		{
			fclose(m_nullDevice);
		}

		return;
	}

	bool LogicModule::load(const QByteArray& logicModuleDescription,
						   const QByteArray& tuningBitsream,
						   const QByteArray& confBitstream,
						   const QByteArray& appLogicBitstream)
	{
		clear();

		bool ok = true;

		ok &= loadLmDescription(logicModuleDescription);
		if (ok == false)
		{
			return false;
		}

		assert(m_lmDescription);

		m_tuningEeprom.init(m_lmDescription->flashMemory().m_tuningFrameSize + 8, m_lmDescription->flashMemory().m_tuningFrameCount, 0xFF);
		m_confEeprom.init(m_lmDescription->flashMemory().m_configFrameSize + 8, m_lmDescription->flashMemory().m_configFrameCount, 0xFF);
		m_appLogicEeprom.init(m_lmDescription->flashMemory().m_appLogicFrameSize + 8, m_lmDescription->flashMemory().m_appLogicFrameCount, 0xFF);

		ok &= loadEeprom(tuningBitsream, &m_tuningEeprom);
		ok &= loadEeprom(confBitstream, &m_confEeprom);
		ok &= loadEeprom(appLogicBitstream, &m_appLogicEeprom);

		return ok;
	}

	void LogicModule::clear()
	{
		output() << "LogicModule: clear" << endl;

		m_lmDescription.reset();

		m_tuningEeprom.clear();
		m_confEeprom.clear();
		m_appLogicEeprom.clear();

		return;
	}

	bool LogicModule::powerOn(int logicModuleNumber, bool autoStart)
	{
		output() << "LogicModule: PowerOn, autoStart = " << autoStart << endl;

		if (m_workerThread.isRunning() == true)
		{
			output() << "LogicModule: PowerOn, previous device emulation is in progress, device will be stopped and new emulation will start." << endl;
			powerOff();
			assert(m_workerThread.isFinished());
		}

		m_device = new DeviceEmulator(logicModuleNumber, *m_lmDescription.get(), m_tuningEeprom, m_confEeprom, m_appLogicEeprom, &output());

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

	bool LogicModule::loadLmDescription(const QByteArray& logicModuleDescription)
	{
		output() << "LogicModule: Load Logic Module description" << endl;

		m_lmDescription = std::make_unique<LmDescription>();

		QString errorString;

		bool ok = m_lmDescription->load(logicModuleDescription, &errorString);
		if (ok == false)
		{
			output() << "LogicModule: Load Logic Module description error: " << errorString << endl;
		}

		return ok;
	}

	bool LogicModule::loadEeprom(const QByteArray& fileData, Eeprom* eeprom)
	{
		if (eeprom == nullptr)
		{
			assert(eeprom);
			return false;
		}

		QString errorMessage;

		bool ok = eeprom->loadData(fileData, &errorMessage);
		if (ok == false)
		{
			output() << "LogicModule: Loading EEPROM error: " << errorMessage << endl;
			return false;
		}

		return true;
	}

	QTextStream& LogicModule::output()
	{
		if (m_textStream != nullptr)
		{
			return *m_textStream;
		}
		else
		{
			return m_nullTextStream;
		}
	}

	QTextStream& LogicModule::output() const
	{
		if (m_textStream != nullptr)
		{
			return *m_textStream;
		}
		else
		{
			return m_nullTextStream;
		}
	}
}
