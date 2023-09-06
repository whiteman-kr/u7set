#include "ScriptRunner.h"

namespace TestSuite
{
	ScriptRunner::ScriptRunner(TestController& testController, ITestLog& scriptTestLog, ControlStatus& status, QMutex& statusMutex) :
		m_testController(testController),
		m_scriptTestLog(scriptTestLog),
		m_status(status),
		m_statusMutex(statusMutex)
	{
		m_jsEngine.installExtensions(QJSEngine::ConsoleExtension);

		// Controllers
		//
		m_jsTestController = m_jsEngine.newQObject(&m_testController);
		QQmlEngine::setObjectOwnership(&m_testController, QQmlEngine::CppOwnership);

		// Log
		//
		m_jsLog = m_jsEngine.newQObject(&m_scriptTestLog);
		QQmlEngine::setObjectOwnership(&m_scriptTestLog, QQmlEngine::CppOwnership);

		m_jsEngine.globalObject().setProperty("log", m_jsLog);
		m_jsEngine.globalObject().setProperty("isSimulator", QJSValue{false});
		m_jsEngine.globalObject().setProperty("isTestSuite", QJSValue{true});

		return;
	}

	ScriptRunner::~ScriptRunner()
	{
		qDebug() << "ScriptRunner::~ScriptRunner()";
	}

	bool ScriptRunner::getScriptTestFunctions(const TestScript& script, QStringList& functionsList, QString& errorMsg)
	{
		return evaluateScript(script, {}, functionsList, errorMsg);
	}

	bool ScriptRunner::runScript(const TestScript& script, const TestScript* globalScript, const TestScriptSelection& filter)
	{
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_testIndex = 0;
			m_status.m_testFunction = "evaluate";
		}

		if (globalScript != nullptr)
		{
			// Evaluate global script
			// 
			QString errorMsg;
			QStringList functionsList;
			if (evaluateScript(*globalScript, {}, functionsList, errorMsg) == false)
			{
				m_scriptTestLog.writeError(errorMsg);
				return false;
			}
		}

		// Evaluate script

		QString errorMsg;
		QStringList functionsList;
		if (evaluateScript(script, filter, functionsList, errorMsg) == false)
		{
			m_scriptTestLog.writeError(errorMsg);
			return false;
		}

		// Exit if no functions to run in this file
		if (functionsList.empty() == true)
		{
			return true;
		}

		m_scriptTestLog.writeMessage(tr("********** Start test script %1 **********").arg(script.fileName()));

		// initTestCase() - will be called before the first test function is executed.
		// cleanupTestCase() - will be called after the last test function was executed.
		// init() - will be called before each test function is executed.
		// cleanup() - will be called after every test function.
		//
		QElapsedTimer timer;
		timer.start();

		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_testFunction = "initTestCase";
		}

		// initTestCase() - will be called before the first test function is executed.
		//
		if (bool initTestCaseResult = runScriptFunction("initTestCase");
			initTestCaseResult == false)
		{
			m_scriptTestLog.writeError(script.fileName() + ": initTestCase() failed, test terminated.");
			return false;
		}

		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_testIndex = 0;
			m_status.m_testCount = functionsList.size();
		}

		int failed = 0;
		for (const QString& testFunc : functionsList)
		{
			{
				QMutexLocker l(&m_statusMutex);
				m_status.m_testIndex++;
				m_status.m_testFunction = testFunc;
			}

			m_scriptTestLog.writeMessage(testFunc + ": RUN");

			bool initOk = false;
			bool testOk = false;
			bool cleanupOk = false;
			
			// init() - called before each test function is executed.
			//
			initOk = runScriptFunction("init");
			if (initOk == true)
			{
				// run test function
				//
				testOk = runScriptFunction(testFunc);
				if (testOk == true)
				{
					m_scriptTestLog.writeMessage(testFunc + ": PASS");
				}
				else
				{
					failed++;
					//totalFailed ++;
					m_scriptTestLog.writeError(testFunc + ": FAIL");
				}

				// cleanup() - called after every test function.
				//
				cleanupOk = runScriptFunction("cleanup");
				if (cleanupOk == false)
				{
					m_scriptTestLog.writeError(testFunc + ": cleanup() failed, test terminated.");
				}
			}
			else
			{
				m_scriptTestLog.writeError(testFunc + ": init() failed, test terminated.");
			}
			
			emit testFinished(script.fileName(), testFunc, initOk == true && testOk == true && cleanupOk == true);

			if (initOk == false || testOk == false || cleanupOk == false)
			{
				break;
			}
		}

		// cleanupTestCase() - will be called after the last test function was executed.
		//
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_testFunction = "cleanupTestCase";
		}

		if (bool cleanupTestCaseResult = runScriptFunction("cleanupTestCase");
			cleanupTestCaseResult == false)
		{
			m_scriptTestLog.writeError(script.fileName() + ": cleanupTestCase() failed, test terminated.");
		}

		qint64 elapsedMsTotal = timer.elapsed();

		if (failed == 0)
		{
			m_scriptTestLog.writeMessage(tr("Totals: %1 tests, %2 failed, %3ms").arg(functionsList.size()).arg(failed).arg(elapsedMsTotal));
		}
		else
		{
			m_scriptTestLog.writeError(tr("Totals: %1 tests, %2 failed, %3ms").arg(functionsList.size()).arg(failed).arg(elapsedMsTotal));
		}

		m_scriptTestLog.writeMessage(tr("********** Finished test script %1 **********").arg(script.fileName()));

		return failed == 0;
	}

	bool ScriptRunner::evaluateScript(const TestScript& script, const TestScriptSelection& filter, QStringList& functionsList, QString& errorMsg)
	{
		// Evaluate script.
		//
		QJSValue scriptValue = m_jsEngine.evaluate(script.script());

		if (scriptValue.isError() == true)
		{
			errorMsg = tr("Script %1 evaluate error at line %2\n"
						  "\tClass: %3\n"
						  "\tStack: %4\n"
						  "\tMessage: %5")
					.arg(script.fileName())
					.arg(scriptValue.property("lineNumber").toInt())
					.arg(metaObject()->className())
					.arg(scriptValue.property("stack").toString())
					.arg(scriptValue.toString());
			return false;
		}

		// Find all functions which starts from 'test', like 'testTgnAboveTNom(), using filter'
		//
		QJSValueIterator it(m_jsEngine.globalObject());
		while (it.hasNext() == true)
		{
			it.next();

			QString functionName = it.name();

			if (functionName.startsWith("test"))
			{
				bool filterMatch = true;

				// Process function list filter
				//
				const QStringList& functions = filter.selectedFunctions(script.fileName());
				if (functions.empty() == false)
				{
					if (std::find(functions.begin(), functions.end(), functionName) == functions.end())
					{
						filterMatch = false;
					}
				}

				if (filterMatch == false)
				{
					continue;
				}

				// Process function mask filter
				//
				for (const QString& mask : filter.testMasks())
				{
					bool matchValue = mask.startsWith('-') == false;	// Should match if no '-', otherwise should NOT match

					QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(
											  matchValue ? mask : mask.right(mask.length() - 1)));

					if(rx.match(functionName).hasMatch() != matchValue)
					{
						filterMatch = false;
					}
				}

				if (filterMatch == false)
				{
					continue;
				}

				// Add function for execution
				//
				functionsList.push_back(functionName);
			}
		}

		std::sort(functionsList.begin(), functionsList.end());

		return true;
	}

	bool ScriptRunner::runScriptFunction(const QString& functionName)
	{
		QJSValue funcProp = m_jsEngine.globalObject().property(functionName);
		if (funcProp.isUndefined() == true)
		{
			return false;
		}

		if (funcProp.isCallable() == false)
		{
			m_scriptTestLog.writeError(tr("%1 is callable function").arg(functionName));
			return false;
		}

		Q_ASSERT(m_jsTestController.isUndefined() == false && m_jsTestController.isObject() == true);
		Q_ASSERT(m_jsLog.isUndefined() == false && m_jsLog.isObject() == true);

		// Run script function
		//
		std::condition_variable callFinishedCondVariable;
		std::atomic<bool> callFinished{false};

		QThread* scriptRunThread = QThread::currentThread();

		auto terminateListenner = [&callFinishedCondVariable, &callFinished, scriptRunThread, this]() -> void
		{
			// Timeout can be added here.
			//
			do
			{
				std::mutex fakeMutex;
				std::unique_lock locker{fakeMutex};

				callFinishedCondVariable.wait_for(
							locker,
							std::chrono::milliseconds{200},
							[&callFinished, scriptRunThread](){ return callFinished.load() || scriptRunThread->isInterruptionRequested(); });
			} while (callFinished.load() == false && scriptRunThread->isInterruptionRequested() == false);

			if (scriptRunThread->isInterruptionRequested() == true)
			{
				m_jsEngine.setInterrupted(true);
			}
		};

		[[maybe_unused]] auto f = std::async(std::launch::async, terminateListenner);

		// Run script, while the script is running func terminateListenner() is executed in another thread and
		// waits for script call end or InterruptRequested for the thread.
		//
		QJSValue callResult;
		try
		{
			callResult = funcProp.call(QJSValueList{} << m_jsTestController);
		}
		catch(...)
		{
			Q_ASSERT(false);	// What the reason of this exception?
			m_scriptTestLog.writeError(tr("Unexpected exception occured in %1.").arg(functionName));

			callFinished.store(true);
			callFinishedCondVariable.notify_one();

			return false;
		}

		callFinished.store(true);
		callFinishedCondVariable.notify_one();

		// Log errors and exit
		//
		if (callResult.isError() == true)
		{
			if (callResult.errorType() == QJSValue::ErrorType::GenericError)
			{
				// Assume that JS code must report about the error
				//
				m_scriptTestLog.writeError(tr("Error, stack trace: %1\n\tMessage: %2")
										   .arg(callResult.property("stack").toString())
										   .arg(callResult.toString()));
			}
			else
			{
				m_scriptTestLog.writeError(tr("Error at line %1\n"
											  "\tStack: %2\n"
											  "\tMessage: %3")
										   .arg(callResult.property("lineNumber").toInt())
										   .arg(callResult.property("stack").toString())
										   .arg(callResult.toString()));
			}

			return false;
		}

		return true;
	}
}
