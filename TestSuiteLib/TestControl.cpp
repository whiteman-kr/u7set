#include "TestControl.h"
#include "AdsInputController.h"
#include "TunsOutputController.h"
#include <ClientLib/TuningUserManager.h>
#include <QSignalSpy>

namespace
{
	const qint64 ServiceConnectTimeoutMs = 5000;
}

namespace TestSuite
{
	//
	// TestControlThread
	//
	TestControlThread::TestControlThread(ILogFile* appLog, TestLog* testLog) :
		ControlThread(appLog, testLog, "TestControlThread")
	{
	}

	void TestControlThread::run()
	{
		{
			QMutexLocker l(&m_statusMutex);
			m_status.reset();
		}

		m_appLog.writeMessage("Started");
		m_result.store(0);

		try
		{
			// Connect to CfgService and get configuration and scripts.
			//
			taskCfgServiceConnection();

			// Check if user is logged in
			//
			taskCheckLogin();

			// Create input controller based on configuration.
			//
			taskInitInputController();

			// Create output controller based on configuration.
			//
			taskInitOutputController();

			// Run tests.
			//
			taskRunTests();

			if (m_controlParams.reportsPath.isEmpty() == false)
			{
				taskCreateReports();
			}
		}
		catch (...)
		{
			m_result.store(1);
		}

		cleanUp();
		m_appLog.writeMessage(tr("Finished, exit code %1").arg(m_result));
		return;
	}

	void TestControlThread::taskCheckLogin()
	{
		// Check user name
		//
		if (m_configuration.login == true)
		{
			if (m_controlParams.userName.isEmpty() == true)
			{
				m_appLog.writeError(tr("Tests execution failed: no user name is supplied! Please check the configuration."));
				throw 1;
			}

			if (ClientLib::TuningUserManager::checkPassword(m_controlParams.userName, m_controlParams.password) == false)
			{
				m_appLog.writeError(tr("Tests execution failed: authorization failed!"));
				throw 1;
			}
		}
	}
	
	void TestControlThread::taskRunTests()
	{
		Q_ASSERT(m_inputController);
		Q_ASSERT(m_outputController);

		std::vector<std::unique_ptr<TestController>> testControllers;
		std::vector<std::unique_ptr<ScriptRunner>> runners;

		TestScript* globalScript = nullptr;

		// Build list of scripts to run

		for (auto& script : m_configData.scripts)
		{
			if (script.isGlobalScript() == true)
			{
				// GlobalScript found
				//
				globalScript = &script;
				continue;
			}

			// Process script files list, if it is not empty
			//
			if (m_controlParams.scriptsFiles.empty() == false)
			{
				if (std::find(m_controlParams.scriptsFiles.begin(), m_controlParams.scriptsFiles.end(), script.fileName()) == m_controlParams.scriptsFiles.end())
				{
					continue;
				}
			}

			// Create test controller and runner for it
			//
			testControllers.push_back(std::make_unique<TestController>(m_configuration, m_softwareInfo, &m_signals, m_appLog.logFile(), m_testLog, *m_inputController, *m_outputController, this));
			runners.push_back(std::make_unique<ScriptRunner>(script, globalScript, m_configuration, *testControllers.back(), *m_testLog, m_status, m_statusMutex));

			// Script tags are from other configuration
			//
			if (runners.back()->scriptInfo().checkScriptTags(m_configuration.scriptTags) == false)
			{
				// Remove just added test controller and runner
				//
				testControllers.pop_back();
				runners.pop_back();
			}
		}

		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_state = ControlState::RunningTests;
			m_status.m_scriptCount = runners.size();
		}

		bool fileTestResult = true;

		for (const auto& runner : runners)
		{
			{
				QMutexLocker l(&m_statusMutex);
				m_status.m_scriptIndex++;
				m_status.m_scriptFile = runner->scriptInfo().fileName;

				m_status.setStartTime();
			}

			std::condition_variable callFinishedCondVariable;
			std::atomic<bool> callFinished{false};

			QThread* scriptRunThread = QThread::currentThread();

			// Execution timeout checking function
			//
			auto checkTestExecutionTime = [&callFinishedCondVariable, &callFinished, scriptRunThread, &runner, this]() -> void
			{
				do
				{
					if (runner->testController().executionTimeout() > 0 && status().duration().count() > runner->testController().executionTimeout())
					{
						QString logMessage = tr("Script %1 execution timeout (%2 ms).").arg(runner->scriptInfo().fileName).arg(status().duration().count());
						m_appLog.writeError(logMessage);
						requestInterruption();
						break;
					}

					std::mutex fakeMutex;
					std::unique_lock l(fakeMutex);
					[[maybe_unused]] auto threadStopped = callFinishedCondVariable.wait_for(
						l,
						std::chrono::milliseconds{200},
						[&callFinished, scriptRunThread]()
						{
							return callFinished.load() || scriptRunThread->isInterruptionRequested();
						});
				} while (callFinished.load() == false && scriptRunThread->isInterruptionRequested() == false);

				return;
			};
			auto f = std::async(std::launch::async, checkTestExecutionTime);

			checkAndInterruptTestExecution();

			QString logMessage = tr("Run test script: %1").arg(runner->scriptInfo().fileName);
			m_appLog.writeMessage(logMessage);

			connect(runner.get(), &ScriptRunner::testStarted, [this](QString scriptFileName, QString testFunction)
					{
						emit testStarted(scriptFileName, testFunction);
					});
			connect(runner.get(), &ScriptRunner::testFinished, [this](QString scriptFileName, QString testFunction, bool result)
					{
						emit testFinished(scriptFileName, testFunction, result);
					});

			bool fileTestError = false;

			try
			{
				fileTestResult &= runner->runTests(m_controlParams.testsFilter);
			}
			catch (int)
			{
				fileTestError = true;
			}

			callFinished.store(true);
			callFinishedCondVariable.notify_one();

			f.wait(); // Wait for checkTestExecutionTime function to complete

			if (fileTestError == true)
			{
				throw 1;
			}
		}

		if (fileTestResult == false)
		{
			throw 1;
		}

		return;
	}

	void TestControlThread::taskCreateReports()
	{
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_state = ControlState::CreatingReports;
		}

		QString reportsPath{m_controlParams.reportsPath};

		if (reportsPath.contains("default") == true)
		{
			reportsPath.replace("default", QString("TestReport_%1").arg(QDateTime::currentDateTime().toString("ddMMyyyy_HHmmss")));
		}

		TestSuite::TestReport::generateReports(m_configData.reportTemplates, *m_testLog, QString(), reportsPath, m_appLog.logFile());

		return;
	}

	//
	// TestControl
	//
	TestControl::TestControl(ILogFile* appLog, TestLog* testLog) :
		Control(appLog, testLog, new TestControlThread(appLog, testLog))
	{

		connect(m_controlThread.get(), &QThread::finished, [this]()
				{
					emit finished(m_controlThread->result());
				});

		 connect(static_cast<TestControlThread*>(m_controlThread.get()), &TestControlThread::testStarted, [this](QString scriptFileName, QString testFunction)
				{
					emit testStarted(scriptFileName, testFunction);
				});
		 connect(static_cast<TestControlThread*>(m_controlThread.get()), &TestControlThread::testFinished, [this](QString scriptFileName, QString testFunction, bool result)
				{
					emit testFinished(scriptFileName, testFunction, result);
				});

	}
} // namespace TestSuite
