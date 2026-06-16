#include <TestSuiteLib/ScriptRunner.h>
#include <TestSuiteLib/TestControl.h>
#include <TestSuiteLib/TestController.h>


namespace
{
	const qint64 ServiceConnectTimeoutMs = 20'000;
}

namespace TestSuite
{
	//
	// TestControlThread
	//
	TestControlThread::TestControlThread(ILogFile* appLog, TestLog* testLog) :
		ControlThread{appLog, testLog, "TestControlThread"}
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
			init();

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

	void TestControlThread::taskCheckLogin() {}

	void TestControlThread::taskRunTests()
	{
		if (m_scriptProvider == nullptr)
		{
			m_appLog.writeError("TestControlThread: script provider is not set.");
			return;
		}

		if (m_inputController == nullptr || m_outputController == nullptr)
		{
			m_appLog.writeWarning(
				QString{"TestControlThread: input or/and output controller is not set. InputController %1, OutputController %2"}
					.arg(m_inputController != nullptr)
					.arg(m_outputController != nullptr));
		}

		auto globalScript = m_scriptProvider->getGloablScript();

		// Build list of scripts to run
		//
		struct Executors
		{
			std::unique_ptr<TestController> testController;
			std::unique_ptr<ScriptRunner> scriptRunner;
		};

		auto&& scripts = m_scriptProvider->getScripts();

		std::vector<Executors> executors;
		executors.reserve(scripts.size());

		for (auto& script : scripts)
		{
			if (script.isGlobalScript() == true)
			{
				continue;
			}

			// Process script files list, if it is not empty
			//
			if (m_controlParams.scriptsFiles.empty() == false && m_controlParams.scriptsFiles.contains(script.fileName()) == false)
			{
				continue;
			}

			// Create test controller and runner for it
			//
			QString softwareEquipmentId = m_softwareInfo.equipmentID();

			auto testController = std::make_unique<TestController>(m_appLog.logFile(),
																   m_testLog,
																   m_inputController.get(),
																   m_outputController.get(),
																   m_setpoints,
																   this);

			testController->setProjectName(projectName());
			testController->setBuildNo(buildNo());

			auto scriptRunner = std::make_unique<ScriptRunner>(softwareEquipmentId,
															   script,
															   globalScript,
															   *testController,
															   *m_testLog,
															   m_status,
															   m_statusMutex);

			scriptRunner->setPlant(plant());
			scriptRunner->setUnit(unit());
			scriptRunner->setSystem(system());

			executors.push_back({std::move(testController), std::move(scriptRunner)});
		}

		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_state = ControlState::RunningTests;
			m_status.m_scriptCount = executors.size();
		}

		bool fileTestResult = true;

		for (const auto& [testController, scriptRunner] : executors)
		{
			{
				QMutexLocker l(&m_statusMutex);
				m_status.m_scriptIndex++;
				m_status.m_scriptFile = scriptRunner->scriptInfo().fileName;

				m_status.setStartTime();
			}

			std::condition_variable callFinishedCondVariable;
			std::atomic<bool> callFinished{false};

			QThread* scriptRunThread = QThread::currentThread();

			// Execution timeout checking function
			//
			auto checkTestExecutionTime = [&callFinishedCondVariable, &callFinished, scriptRunThread, &scriptRunner, this]() -> void
			{
				do
				{
					if (scriptRunner->testController().executionTimeout() > 0 &&
						status().duration().count() > scriptRunner->testController().executionTimeout())
					{
						QString logMessage = tr("Script %1 execution timeout (%2 ms).")
												 .arg(scriptRunner->scriptInfo().fileName)
												 .arg(status().duration().count());
						m_appLog.writeError(logMessage);
						requestInterruption();
						break;
					}

					std::mutex fakeMutex;
					std::unique_lock l(fakeMutex);
					[[maybe_unused]] auto threadStopped =
						callFinishedCondVariable.wait_for(l,
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

			QString logMessage = tr("Run test script: %1").arg(scriptRunner->scriptInfo().fileName);
			m_appLog.writeMessage(logMessage);

			connect(scriptRunner.get(),
					&ScriptRunner::testStarted,
					[this](QString scriptFileName, QString testFunction)
					{
						emit testStarted(scriptFileName, testFunction);
					});
			connect(scriptRunner.get(),
					&ScriptRunner::testFinished,
					[this](QString scriptFileName, QString testFunction, bool result)
					{
						emit testFinished(scriptFileName, testFunction, result);
					});

			bool fileTestError = false;

			try
			{
				fileTestResult &= scriptRunner->runTests(m_controlParams.testsFilter);
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
		QMutexLocker l(&m_statusMutex);
		m_status.m_state = ControlState::CreatingReports;

		return;
	}

	//
	// TestControl
	//
	TestControl::TestControl(ILogFile* appLog, TestLog* testLog) :
		TestControl{appLog, testLog, new TestControlThread{appLog, testLog}}
	{
	}

	TestControl::TestControl(ILogFile* appLog, TestLog* testLog, ControlThread* controlThread) :
		Control{appLog, testLog, controlThread}
	{
		assert(controlThread);

		connect(m_controlThread.get(),
				&QThread::finished,
				[this]()
				{
					emit finished(m_controlThread->result());
				});

		connect(static_cast<TestControlThread*>(m_controlThread.get()),
				&TestControlThread::testStarted,
				[this](QString scriptFileName, QString testFunction)
				{
					emit testStarted(scriptFileName, testFunction);
				});

		connect(static_cast<TestControlThread*>(m_controlThread.get()),
				&TestControlThread::testFinished,
				[this](QString scriptFileName, QString testFunction, bool result)
				{
					emit testFinished(scriptFileName, testFunction, result);
				});
	}

} // namespace TestSuite
