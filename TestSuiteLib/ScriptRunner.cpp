#include "ScriptRunner.h"

namespace TestSuite
{
	ScriptRunner::ScriptRunner(TestController& testController, ITestLog& scriptTestLog) :
		m_testController(testController),
		m_scriptTestLog(scriptTestLog)
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

		return;
	}

	ScriptRunner::~ScriptRunner()
	{
		qDebug() << "ScriptRunner::~ScriptRunner()";
	}

	bool ScriptRunner::runScript(const TestScript& script)
	{
		qDebug() << "ScriptRunner::runScript(), script file: " << script.fileName;

		m_scriptTestLog.writeMessage(tr("********** Start testing of %1 **********").arg(script.fileName));

		// Evaluate script.
		//
		QJSValue scriptValue = m_jsEngine.evaluate(script.script);

		if (scriptValue.isError() == true)
		{
			m_scriptTestLog.writeError(tr("Script %1 evaluate error at line %2\n"
										  "\tClass: %3\n"
										  "\tStack: %4\n"
										  "\tMessage: %5")
									   .arg(script.fileName)
									   .arg(scriptValue.property("lineNumber").toInt())
									   .arg(metaObject()->className())
									   .arg(scriptValue.property("stack").toString())
									   .arg(scriptValue.toString()));
			return false;
		}

		// initTestCase() - will be called before the first test function is executed.
		// cleanupTestCase() - will be called after the last test function was executed.
		// init() - will be called before each test function is executed.
		// cleanup() - will be called after every test function.
		//
		QElapsedTimer timer;
		timer.start();

		// initTestCase() - will be called before the first test function is executed.
		//
		if (bool initTestCaseResult = runScriptFunction("initTestCase");
			initTestCaseResult == false)
		{
			m_scriptTestLog.writeError(script.fileName + ": initTestCase() failed, test terminated.");
			return false;
		}

		// Call all functions which starts from 'test', like 'testTgnAboveTNom()'
		//
		QStringList testList;

		QJSValueIterator it(m_jsEngine.globalObject());
		while (it.hasNext() == true)
		{
			it.next();

			if (it.name().startsWith("test"))
			{
				testList.push_back(it.name());
			}
		}

		std::sort(testList.begin(), testList.end());

		int failed = 0;
		for (const QString& testFunc : testList)
		{
			// init() - called before each test function is executed.
			//
			if (bool initOk = runScriptFunction("init");
				initOk == false)
			{
				m_scriptTestLog.writeError(testFunc + ": init() failed, test terminated.");
				break;
			}

			if (bool testOk = runScriptFunction(testFunc);
				testOk == true)
			{
				m_scriptTestLog.writeMessage(testFunc + ": ok");
			}
			else
			{
				failed ++;
				//totalFailed ++;
				m_scriptTestLog.writeError(testFunc + ": FAILED");
			}

			// cleanup() - called after every test function.
			//
			if (bool cleanupOk = runScriptFunction("cleanup");
				cleanupOk == false)
			{
				m_scriptTestLog.writeError(testFunc + ": cleanup() failed, test terminated.");
				break;
			}
		}

		// cleanupTestCase() - will be called after the last test function was executed.
		//
		if (bool cleanupTestCaseResult = runScriptFunction("cleanupTestCase");
			cleanupTestCaseResult == false)
		{
			m_scriptTestLog.writeError(script.fileName + ": cleanupTestCase() failed, test terminated.");
		}

		qint64 elapsedMsTotal = timer.elapsed();

		if (failed == 0)
		{
			m_scriptTestLog.writeMessage(tr("Totals: %1 tests, %2 failed, %3ms").arg(testList.size()).arg(failed).arg(elapsedMsTotal));
		}
		else
		{
			m_scriptTestLog.writeError(tr("Totals: %1 tests, %2 failed, %3ms").arg(testList.size()).arg(failed).arg(elapsedMsTotal));
		}

		m_scriptTestLog.writeMessage(tr("********** Finished testing of %1 **********").arg(script.fileName));

		return failed == 0;
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
