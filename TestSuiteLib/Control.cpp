#include "Control.h"
#include "AdsInputController.h"
#include "TunsOutputController.h"
#include <QSignalSpy>

namespace
{
	const qint64 ServiceConnectTimeoutMs = 5000;
}

namespace TestSuite
{
	//
	// ControlThread
	//

	ControlThread::ControlThread(ILogFile* appLog, TestLog* testLog, const QString& runContext) :
		m_appLog{appLog, runContext},
		m_testLog{testLog},
		m_signals(appLog)
	{
		Q_ASSERT(m_testLog);
		return;
	}

	ControlThread::~ControlThread()
	{
	}

	void ControlThread::setTestParams(const SoftwareInfo& softwareInfo,
									  const TestSuiteSettings& settings,
									  const ControlParams& controlParams)
	{
		Q_ASSERT(isRunning() == false);

		m_softwareInfo = softwareInfo;
		m_settings = settings;
		m_controlParams = controlParams;

		return;
	}

	int ControlThread::result() const
	{
		Q_ASSERT(isRunning() == false);
		return m_result.load();
	}

	ControlStatus ControlThread::status() const
	{
		QMutexLocker l(&m_statusMutex);
		return m_status;
	}

	void ControlThread::checkAndInterruptTestExecution()
	{
		if (isInterruptionRequested() == true)
		{
			m_appLog.writeError(tr("Test execution was interrupted by user."));
			throw 1;
		}

		return;
	}

	void ControlThread::cleanUp()
	{

		m_configuration = {};
		m_configData = {};
		m_signals.reset();
		m_inputController.reset();
		m_outputController.reset();

		{
			QMutexLocker l(&m_statusMutex);
			m_status.reset();
		}
		return;
	}

	void ControlThread::taskCfgServiceConnection()
	{
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_state = ControlState::RequestingConfiguration;
		}

		TestSuiteConfigController configController{m_softwareInfo,
												   m_settings.configuratorAddress1(),
												   m_settings.configuratorAddress2(),
												   m_appLog.logFile()};

		configController.start();

		// Wait that configuration arrived or error occurred.
		//
		QSignalSpy spySuccess{&configController, &TestSuiteConfigController::configurationArrived};
		QSignalSpy spyError{&configController, &TestSuiteConfigController::configrationError};

		for (int i = 0; i < 10; i++)
		{
			checkAndInterruptTestExecution();

			spySuccess.wait(ServiceConnectTimeoutMs / 10);
			if (spySuccess.isEmpty() == false || spyError.isEmpty() == false)
			{
				break;
			}
		}

		if (spyError.isEmpty() == false)
		{
			// Read configuration error.
			//
			throw 1;
		}

		if (spySuccess.isEmpty() == true)
		{
			// Config controller had not connected to CfgService.
			//
			m_appLog.writeError(tr("Cannot connect to CfgService, address1 %1, address2 %2, InstanceID %3")
									.arg(m_settings.configuratorAddress1().toString())
									.arg(m_settings.configuratorAddress2().toString())
									.arg(m_softwareInfo.equipmentID()));
			throw 1;
		}

		m_configuration = configController.configuration();
		m_configData = configController.configData();

		// Get arrived config data
		//
		if (m_controlParams.scriptsPath.isEmpty() == false)
		{
			// If scriptsPath is not empty, then load scripts from disk.
			//
			QString errorMessage;
			TestScriptsStorage scriptsStorage;

			bool ok = scriptsStorage.loadFromPath(m_controlParams.scriptsPath, &errorMessage);
			if (ok == false)
			{
				m_appLog.writeError(tr("Load script file error, path %1, error message: %2").arg(m_controlParams.scriptsPath).arg(errorMessage));
				throw 1;
			}

			m_configData.scripts = scriptsStorage.scripts();
		}

		return;
	}

	void ControlThread::taskInitInputController()
	{
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_state = ControlState::InitInputController;
		}

		auto controller = std::make_unique<AdsInputController>(m_signals,
															   m_softwareInfo,
															   m_configuration.appDataServices,
															   m_appLog.logFile());
		bool ok = controller->waitForConnection(ServiceConnectTimeoutMs);
		if (ok == false)
		{
			throw 1;
		}

		m_inputController = std::move(controller);
		return;
	}

	void ControlThread::taskInitOutputController()
	{
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_state = ControlState::InitOutputController;
		}

		if (m_configuration.tuningEnabled == false)
		{
			auto controller = std::make_unique<OutputControllerStub>();
			m_outputController = std::move(controller);
		}
		else
		{
			auto controller = std::make_unique<TunsOutputController>(m_softwareInfo,
																	 m_configuration.tuningServices,
																	 m_controlParams.userName,
																	 m_configuration.tuningSignalsFile,
																	 TuningClientSettings::LmStatusFlagMode::None, // Access key?
																	 m_appLog.logFile());
			bool ok = controller->waitForConnection(ServiceConnectTimeoutMs);
			if (ok == false)
			{
				throw 1;
			}
			m_outputController = std::move(controller);
		}

		return;
	}

	//
	// Control
	//

	Control::Control(ILogFile* appLog, TestLog* testLog, ControlThread* controlThread) :
		QObject{nullptr},
		m_appLog{appLog},
		m_testLog{testLog},
		m_controlThread{controlThread}
	{
		Q_ASSERT(m_appLog);
		Q_ASSERT(m_testLog);
		Q_ASSERT(m_controlThread);
		m_controlThread->moveToThread(m_controlThread.get());
		return;
	}

	Control::~Control()
	{
		if (isRunning() == true)
		{
			assert(false);
			stop();
		}
	}

	bool Control::execute(const SoftwareInfo& softwareInfo,
						  const TestSuiteSettings& settings,
						  const ControlParams& controlParams)
	{
		if (isRunning() == true)
		{
			Q_ASSERT(false);
			return false;
		}

		m_controlThread->setTestParams(softwareInfo, settings, controlParams);
		m_controlThread->start();

		// Wait that ControlThread actually started.
		//
		QSignalSpy spy{m_controlThread.get(), &QThread::started};
		spy.wait();

		return true;
	}

	bool Control::stop()
	{
		if (m_stopRequested.load() == false)
		{
			// Request test thread to stop
			//
			m_stopRequested.store(true);
			m_controlThread->requestInterruption();

			// Wait for thread to stop in other thread and do not block interface thread
			//
			auto waitForThreadStop = [this]() -> void
			{
				if (m_controlThread->wait(120'000) == false)
				{
					qDebug() << "Control::stop(): m_controlThread was not finished in time, terminate().";
					m_controlThread->terminate();
				}

				m_stopRequested.store(false);
			};

			[[maybe_unused]] auto f = std::async(std::launch::async, waitForThreadStop);
		}
		else
		{
			m_appLog->writeWarning("Already waiting for testing thread to stop.");
		}

		return true;
	}

	bool Control::isRunning() const
	{
		return m_controlThread->isRunning();
	}

	ControlStatus Control::status() const
	{
		return m_controlThread->status();
	}
} // namespace TestSuite
