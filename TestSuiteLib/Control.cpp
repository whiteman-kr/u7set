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
	//
	// TestSuiteUserManager
	//
	TestSuiteUserManager::TestSuiteUserManager(const QString& userName, const QString& password):
		m_userName(userName),
		m_password(password)
	{
	}

	bool TestSuiteUserManager::askForPassword(QString* userName, QString* password, QWidget* /*parent*/)
	{
		*userName = m_userName;
		*password = m_password;
		return true;
	}

	//
	// ControlThread
	//

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
									  const QStringList& scriptsFiles,		// List of script files for execution, if empty then exec all.
									  const QString& scriptsPath,			// Load scripts from disk, path to dir for *.js files.)
									  const TestScriptSelection& testsFilter,			// Tests filter
									  const QString& userName,
									  const QString& password)
	{
		Q_ASSERT(isRunning() == false);

		m_softwareInfo = softwareInfo;
		m_settings = settings;

		m_scriptsToRun = scriptsFiles;
		m_scriptsPath = scriptsPath;
		m_testsFilter = testsFilter;

		m_userName = userName;
		m_password = password;

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

	ReportLib::ReportTemplateStorage ControlThread::reportTemplates() const
	{
		QMutexLocker l(&m_reportTemplatesMutex);
		return m_reportTemplates;
	}

	void ControlThread::run()
	{
		{
			QMutexLocker l(&m_statusMutex);
			m_status.reset();
		}

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

		{
			QMutexLocker l(&m_statusMutex);
			m_status.reset();
		}
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

		{
			QMutexLocker l(&m_reportTemplatesMutex);
			m_reportTemplates = configController.reportTemplates();
		}

		// Check user name
		//
		if (m_configuration.login == true)
		{
			if (m_userName.isEmpty() == true)
			{
				m_appLog.writeError(tr("Tests execution failed: no user name is supplied! Please check the configuration."));
				throw 1;
			}

			TestSuiteUserManager userManager(m_userName, m_password);
			userManager.setConfiguration(true, m_configuration.userAccounts, true, 120);

			if (userManager.login(nullptr) == false)
			{
				m_appLog.writeError(tr("Tests execution failed: authorization failed!"));
				throw 1;
			}
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
			m_outputController.reset();
			return;
		}

		auto controller = std::make_unique<TunsOutputController>(m_softwareInfo,
																 m_configuration.tuningServices,
																 m_configuration.tuningSignalsFile,
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
		Q_ASSERT(m_inputController);
		Q_ASSERT(m_outputController);

		std::vector<const TestScript*> runScripts;

		// Build list of scripts to run

		for (const auto& script : m_scripts)
		{
			// Do not execute global scripts
			//
			if (script.isGlobalScript() == true)
			{
				continue;
			}

			// Process script files list, if it is not empty
			//
			if (m_scriptsToRun.empty() == false)
			{
				if (std::find(m_scriptsToRun.begin(), m_scriptsToRun.end(), script.fileName()) == m_scriptsToRun.end())
				{
					continue;
				}
			}

			runScripts.push_back(&script);
		}

		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_state = ControlState::RunningTests;
			m_status.m_scriptCount = runScripts.size();
		}

		TestController testController{m_configuration, m_softwareInfo, &m_signals, m_appLog.logFile(), *m_inputController, *m_outputController, this};

		bool fileTestResult = true;

		for (const auto& script : runScripts)
		{
			{
				QMutexLocker l(&m_statusMutex);
				m_status.m_scriptIndex++;
				m_status.m_scriptFile = script->fileName();

				m_status.setStartTime();
			}

			std::condition_variable callFinishedCondVariable;
			std::atomic<bool> callFinished{false};

			QThread* scriptRunThread = QThread::currentThread();

			// Execution timeout checking function
			//
			auto checkTestExecutionTime = [&callFinishedCondVariable, &callFinished, scriptRunThread, &testController, script, this]()->void
				{
					do
					{
						if (testController.executionTimeout() > 0 && status().duration().count() > testController.executionTimeout())
						{
							QString logMessage = tr("Script %1 execution timeout (%2 ms).").arg(script->fileName()).arg(status().duration().count());
							m_appLog.writeError(logMessage);
							requestInterruption();
							break;
						}

						std::mutex fakeMutex;
						std::unique_lock l(fakeMutex);
						auto threadStopped = callFinishedCondVariable.wait_for(
							l, 
							std::chrono::milliseconds{200},
							[&callFinished, scriptRunThread]() {return callFinished.load() || scriptRunThread->isInterruptionRequested(); });
					} while (callFinished.load() == false && scriptRunThread->isInterruptionRequested() == false);

					return;
				};
			auto f = std::async(std::launch::async, checkTestExecutionTime);
			
			checkAndInterruptTestExecution();

			QString logMessage = tr("Run test script: %1").arg(script->fileName());
			m_appLog.writeMessage(logMessage);

			ScriptRunner scriptRunner{testController, *m_testLog, m_status, m_statusMutex};

			connect(&scriptRunner, &ScriptRunner::testFinished, [this](QString scriptFileName, QString testFunction, bool result){
				emit testFinished(scriptFileName, testFunction, result);
			});

			fileTestResult &= scriptRunner.runScript(*script, m_testsFilter);

			callFinished.store(true);
			callFinishedCondVariable.notify_one();

			f.wait();	// Wait for checkTestExecutionTime function to complete
		}

		if (fileTestResult == false)
		{
			throw 1;
		}

		return;
	}

	void ControlThread::taskCreateReports()
	{
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_state = ControlState::CreatingReports;
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

		connect(&m_controlThread, &ControlThread::testFinished, [this](QString scriptFileName, QString testFunction, bool result){
			emit testFinished(scriptFileName, testFunction, result);
		});

		return;
	}

	bool Control::execute(const SoftwareInfo& softwareInfo,
						  const TestSuiteSettings& settings,
						  const QStringList& scriptsFiles,		// List of script files for execution, if empty then exec all.
						  const QString& scriptsPath,			// Load scripts from disk, path to dir for *.js files.
						  const TestScriptSelection& testsFilter,			// Tests filter
						  const QString& userName,
						  const QString& password)
	{
		if (isRunning() == true)
		{
			return false;
		}

		m_controlThread.moveToThread(&m_controlThread);

		m_controlThread.setTestParams(softwareInfo, settings, scriptsFiles, scriptsPath, testsFilter, userName, password);
		m_controlThread.start();

		// Wait that ControlThread actually started.
		//
		QSignalSpy spy{&m_controlThread, &QThread::started};
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
			m_controlThread.requestInterruption();

			// Wait for thread to stop in other thread and do not block interface thread
			//
			auto waitForThreadStop = [this]()->void
				{
					if (m_controlThread.wait(120'000) == false)
					{
						qDebug() << "Control::stop(): m_controlThread was not finished in time, terminate().";
						m_controlThread.terminate();
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
		return m_controlThread.isRunning();
	}

	ControlStatus Control::status() const
	{
		return m_controlThread.status();
	}

	ReportLib::ReportTemplateStorage Control::reportTemplates() const
	{
		return m_controlThread.reportTemplates();
	}
}

