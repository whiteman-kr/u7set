#include <QSignalSpy>
#include "Control.h"
#include "AdsInputController.h"
#include "TunsOutputController.h"
#include "ScriptRunner.h"

namespace
{
	const qint64 ServiceConnectTimeoutMs = 5000;
}

namespace TestSuite
{
	ControlThread::ControlThread(ILogFile* appLog, ITestLog* testLog) :
		m_appLog{appLog, "ControlThread"},
		m_testLog{testLog},
		m_signals(appLog)
	{
		Q_ASSERT(m_testLog);
		return;
	}

	void ControlThread::setTestParams(const SoftwareInfo& softwareInfo,
									  const TestSuiteSettings& settings,
									  const QStringList& executionTests,	// List of tests for execution, if empty then exec all.
									  const QString& scriptsPath)			// Load scripts from disk, path to dir for *.js files.)
	{
		Q_ASSERT(isRunning() == false);

		m_softwareInfo = softwareInfo;
		m_settings = settings;

		m_executionTests = executionTests;
		m_scriptsPath = scriptsPath;

		return;
	}

	int ControlThread::result() const
	{
		Q_ASSERT(isRunning() == false);
		return m_result.load();
	}

	void ControlThread::run()
	{
		m_appLog.writeMessage("ThreadStarted");
		m_result.store(0);

		try
		{
			// Connect to CfgService and get configuration and scripts.
			//
			taskCfgServiceConnection();

			// Create input controller based on configuration.
			//
			taskInitInputController();

			// Create output controller based on configuration.
			//
			taskInitOutputController();

			// Run tests.
			//
			taskRunTests();
		}
		catch(...)
		{
			m_result.store(1);
		}

		cleanUp();
		m_appLog.writeMessage(tr("ThreadFinished, exit code %1").arg(m_result));
		return;
	}

	void ControlThread::cleanUp()
	{
		m_configuration = {};
		m_scripts.clear();
		m_signals.reset();
		m_inputController.reset();
		m_outputController.reset();
		return;
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

	void ControlThread::taskCfgServiceConnection()
	{
		TestSuiteConfigController configController{m_softwareInfo,
					m_settings.configuratorAddress1(),
					m_settings.configuratorAddress2(),
					m_appLog.logFile()};

		configController.start();

		// Wait that configuration arrived or error accured.
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
			// Read configuration errror.
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

		// Get arrived config data
		//
		if (m_scriptsPath.isEmpty() == true)
		{
			// Load scripts from the project (CfgServices)
			//
			m_scripts = std::move(configController.scripts());
		}
		else
		{
			// If scriptsPath is not empty, then load scripts from disk.
			//
			QString errorMessage;
			TestScriptsStorage scriptsStorage;

			bool ok = scriptsStorage.loadFromPath(m_scriptsPath, &errorMessage);
			if (ok == false)
			{
				m_appLog.writeError(tr("Load script file error, path %1, error message: %2").arg(m_scriptsPath).arg(errorMessage));
				throw 1;
			}

			m_scripts = scriptsStorage.scripts();
		}

		m_configuration = configController.configuration();
		return;
	}

	void ControlThread::taskInitInputController()
	{
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
		if (m_configuration.tuningEnabled == false)
		{
			m_outputController.reset();
			return;
		}

		auto controller = std::make_unique<TunsOutputController>(m_softwareInfo,
																 m_configuration.tuningServices,
																 TuningClientSettings::LmStatusFlagMode::None,	// Access key?
																 m_appLog.logFile());
		bool ok = controller->waitForConnection(ServiceConnectTimeoutMs);
		if (ok == false)
		{
			throw 1;
		}

		m_outputController = std::move(controller);
		return;
	}

	void ControlThread::taskRunTests()
	{
		TestController testController;

		bool fileTestResult = true;

		for (const auto& script : m_scripts)
		{
			if (m_executionTests.empty() == false &&
					std::find(m_executionTests.begin(), m_executionTests.end(), script.fileName) == m_executionTests.end())
			{
				continue;
			}

			checkAndInterruptTestExecution();

			QString logMessage = tr("Run test script: %1").arg(script.fileName);
			m_appLog.writeMessage(logMessage);

			ScriptRunner scriptRunner{testController, *m_testLog};
			fileTestResult &= scriptRunner.runScript(script);
		}

		if (fileTestResult == false)
		{
			throw 1;
		}

		return;
	}


	Control::Control(ILogFile* appLog, ITestLog* testLog) :
		QObject{nullptr},
		m_appLog{appLog},
		m_testLog{testLog},
		m_controlThread{m_appLog, m_testLog}
	{
		Q_ASSERT(m_appLog);
		Q_ASSERT(m_testLog);

		connect(&m_controlThread, &QThread::finished, [this](){
			emit finished(m_controlThread.result());
		});

		return;
	}

	bool Control::execute(const SoftwareInfo& softwareInfo,
						  const TestSuiteSettings& settings,
						  const QStringList& executionTests,	// List of tests for execution, if empty then exec all.
						  const QString& scriptsPath)			// Load scripts from disk, path to dir for *.js files.
	{
		if (isRunning() == true)
		{
			return false;
		}

		m_controlThread.moveToThread(&m_controlThread);

		m_controlThread.setTestParams(softwareInfo, settings, executionTests, scriptsPath);
		m_controlThread.start();

		// Wait that ControlThread actually started.
		//
		QSignalSpy spy{&m_controlThread, &QThread::started};
		spy.wait();

		return true;
	}

	bool Control::stop()
	{
		m_controlThread.requestInterruption();

		if (m_controlThread.wait(120'000) == false)
		{
			qDebug() << "Control::stop(): m_controlThread was not finished in time, terminate().";
			m_controlThread.terminate();
		}

		return true;
	}

	bool Control::isRunning() const
	{
		return m_controlThread.isRunning();
	}
}

