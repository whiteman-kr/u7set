#include "RunControl.h"
#include "AdsInputController.h"
#include "TunsOutputController.h"
#include <QSignalSpy>
#include <chrono>

namespace
{
	const qint64 ServiceConnectTimeoutMs = 5000;
}

namespace TestSuite
{
	using namespace std::chrono;

	//
	// RunControlThread
	//
	RunControlThread::RunControlThread(ILogFile* appLog, TestLog* testLog) :
		ControlThread(appLog, testLog, "RunControlThread")
	{
	}

	void RunControlThread::reset()
	{
		m_resetFlag.store(true);
	}
	
	void RunControlThread::run()
	{
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_state = ControlState::Permission;
		}

		m_appLog.writeMessage("Started");
		m_result.store(0);

		do
		{
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

				// Evaluate all tests
				//
				taskPrepare();

				// Run tests.
				//
				do
				{
					taskQueryPermission();

					if (m_scriptPermissions.empty() == true && m_globalPermission.has_value() == false)
					{
						m_appLog.writeMessage("No permission functons for tests exist, exit the permission control thread.");
						requestInterruption();
					}

					// Wait 500 ms before repeating the request
					//
					for (int i = 0; i < 5; i++)
					{
						if (isInterruptionRequested() == true || m_resetFlag.load() == true)
						{
							break;
						}
						QThread::msleep(100);
					}
				} while (isInterruptionRequested() == false && m_resetFlag.load() == false);
			}
			catch (...)
			{
				m_result.store(1);
			}

			// Cleanup
			//
			cleanUp();

			// Local cleanup
			//
			taskCleanup();

			if (m_resetFlag.load() == false)
			{
				// Wait 5000 ms before reconnecting
				//
				for (int i = 0; i < 50; i++)
				{
					if (isInterruptionRequested() == true)
					{
						break;
					}
					QThread::msleep(100);
				}
			}
			else
			{
				// Do not wait if this is reset process
				//
				m_resetFlag.store(false);
			}
		} while (isInterruptionRequested() == false);

		m_appLog.writeMessage(tr("Finished, exit code %1").arg(m_result));
		return;
	}

	void RunControlThread::taskPrepare()
	{
		Q_ASSERT(m_inputController);
		Q_ASSERT(m_outputController);

		Q_ASSERT(m_testController == nullptr);
		m_testController = std::make_unique<TestController>(m_configuration, m_softwareInfo, &m_signals, m_appLog.logFile(), m_testLog, *m_inputController, *m_outputController, this);

		TestScript* globalScript = nullptr;

		// Find global script

		for (auto& script : m_configData.scripts)
		{
			if (script.isGlobalScript() == true)
			{
				globalScript = &script;
				break;
			}
		}

		for (const auto& script : m_configData.scripts)
		{
			if (script.isGlobalScript() == true)
			{
				continue;
			}

			m_runners.push_back(std::make_unique<ScriptRunner>(script, globalScript, m_configuration, *m_testController, *m_testLog, m_status, m_statusMutex));
		}
	}

	void RunControlThread::taskQueryPermission()
	{
		Q_ASSERT(m_inputController);
		Q_ASSERT(m_outputController);
		Q_ASSERT(m_testController);

		for (const auto& runner : m_runners)
		{
			if (runner->scriptInfo().globalAllowFunction.isEmpty() == true && runner->scriptInfo().allowFunction.isEmpty() == true)
			{
				continue;
			}

			// Check script tags
			//
			if (runner->scriptInfo().checkScriptTags(m_configuration.scriptTags) == false)
			{
				continue;
			}

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
			QString fileName = runner->scriptInfo().fileName;
			auto checkTestExecutionTime = [&callFinishedCondVariable, &callFinished, scriptRunThread, fileName, this]() -> void
			{
				do
				{
					if (m_testController->executionTimeout() > 0 && status().duration().count() > m_testController->executionTimeout())
					{
						QString logMessage = tr("Script %1 execution timeout (%2 ms).").arg(fileName).arg(status().duration().count());
						m_appLog.writeError(logMessage);
						requestInterruption();
						break;
					}

					std::mutex fakeMutex;
					std::unique_lock l(fakeMutex);
					auto threadStopped = callFinishedCondVariable.wait_for(
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

			bool fileTestResult = false;

			try
			{
				bool globalPermission = true;
				bool scriptPermission = true;

				fileTestResult = runner->queryPermission(globalPermission, scriptPermission);

				bool permission = globalPermission && scriptPermission;

				auto it = m_scriptPermissions.find(runner->scriptInfo().fileName);

				if (it == m_scriptPermissions.end())
				{
					m_scriptPermissions[runner->scriptInfo().fileName] = permission;
					emit scriptPermissionChanged(runner->scriptInfo().fileName, permission);
				}
				else
				{
					if (permission != it->second)
					{
						m_scriptPermissions[runner->scriptInfo().fileName] = permission;
						emit scriptPermissionChanged(runner->scriptInfo().fileName, permission);
					}
				}

				if (m_globalPermission.has_value() == false || m_globalPermission != globalPermission)
				{
					m_globalPermission = globalPermission;
					emit globalPermissionChanged(m_globalPermission.value());

					{
						QMutexLocker l(&m_statusMutex);
						m_status.m_state = m_globalPermission == true ? ControlState::Permission : ControlState::NoPermission;
						m_status.m_scriptCount = m_runners.size();
					}
				}
			}
			catch (int)
			{
				fileTestResult = false;
			}

			callFinished.store(true);
			callFinishedCondVariable.notify_one();

			f.wait(); // Wait for checkTestExecutionTime function to complete*/

			if (fileTestResult == false)
			{
				throw 1;
			}
		}

		return;
	}

	void RunControlThread::taskCleanup()
	{
		m_runners.clear();
		m_testController.reset();
		m_scriptPermissions.clear();
	}

	//
	// RunControl
	//
	RunControl::RunControl(ILogFile* appLog, TestLog* testLog) :
		Control(appLog, testLog, new RunControlThread(appLog, testLog))
	{
		connect(static_cast<RunControlThread*>(m_controlThread.get()), &RunControlThread::scriptPermissionChanged, [this](QString scriptFileName, bool result)
				{
					emit scriptPermissionChanged(scriptFileName, result);
				});
		connect(static_cast<RunControlThread*>(m_controlThread.get()), &RunControlThread::globalPermissionChanged, [this](bool result)
				{
					emit globalPermissionChanged(result);
				});
	}

	void RunControl::reset()
	{
		static_cast<RunControlThread*>(m_controlThread.get())->reset();
	}

} // namespace TestSuite
