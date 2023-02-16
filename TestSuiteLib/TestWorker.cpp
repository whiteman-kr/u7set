#include "TestWorker.h"

TestController::TestController(QObject* parent)
{

}

TestController::~TestController()
{

}

void TestController::throwScriptException(const QObject* object, QString text)
{
	if (object == nullptr)
	{
		Q_ASSERT(object);
		return;
	}

	QJSEngine* jsEngine = qjsEngine(object);
	Q_ASSERT(jsEngine);

	if (jsEngine != nullptr)
	{
		jsEngine->throwError(QJSValue::ErrorType::GenericError, text);
	}

	return;
}

void TestController::debugOutput(QString str)
{
	qDebug() << str;
}


QJSValue TestController::signalState(QString appSignalId)
{
	/*
	bool ok = false;
	AppSignalState state = m_simulator->appSignalManager().signalState(appSignalId, &ok, true);

	if (ok == false)
	{
		throwScriptException(this, tr("signalState(%1), signal not found.").arg(appSignalId));
		return -1;
	}

	QJSEngine* jsEngine = qjsEngine(this);
	if (jsEngine == nullptr)
	{
		assert(jsEngine);
		return {};
	}

	return jsEngine->toScriptValue(state);
	*/
	return {};
}

double TestController::signalValue(QString appSignalId)
{
	/*
	bool ok = false;
	AppSignalState state = m_simulator->appSignalManager().signalState(appSignalId, &ok, true);

	if (ok == false)
	{
		throwScriptException(this, tr("signalValue(%1), signal not found.").arg(appSignalId));
		return -1;
	}

	return state.value();*/
	return 0;
}

bool TestController::overrideSignalValue(QString appSignalId, double value)
{
	/*if (m_simulator->overrideSignals().isSignalInOverrideList(appSignalId) == false)
	{
		int count = m_simulator->overrideSignals().addSignals(QStringList{} << appSignalId);
		if (count != 1)
		{
			return false;
		}
	}

	m_simulator->overrideSignals().setValue(appSignalId, OverrideSignalMethod::Value, value);*/
	return true;
}

bool TestController::signalExists(QString appSignalId) const
{
	return true;//m_simulator->appSignalManager().signalExists(appSignalId);
}

AppSignalParam TestController::signalParam(QString appSignalId)
{
	/*bool ok = false;

	AppSignalParam result = m_simulator->appSignalManager().signalParam(appSignalId, &ok);
	if (ok == false)
	{
		throwScriptException(this, tr("signalParam(%1), signal not found.").arg(appSignalId));
	}

	return result;*/
	return {};
}

//
// ----------------------------------- TestWorker -----------------------------
//
TestWorker::TestWorker(TestController* testController, ScriptTestLog* scriptTestLog):
	m_testController(testController),
	m_scriptTestLog(scriptTestLog)
{

}

void TestWorker::setScripts(const std::vector<TestScript>& scripts)
{
	m_scripts = scripts;
}

void TestWorker::run()
{
	m_result = true;

	int totalFailed = 0;
	try
	{
		// Evaluate script
		//
		for (const TestScript& script :  m_scripts)
		{
			m_jsEngine = std::make_unique<QJSEngine>();						// Creating new QJSEngine clears all old context
			m_jsEngine->installExtensions(QJSEngine::ConsoleExtension);

			// Controllers
			//
			m_jsThis = m_jsEngine->newQObject(m_testController);
			QQmlEngine::setObjectOwnership(m_testController, QQmlEngine::CppOwnership);

			// Log
			//
			m_jsLog = m_jsEngine->newQObject(m_scriptTestLog);
			QQmlEngine::setObjectOwnership(m_scriptTestLog, QQmlEngine::CppOwnership);

			m_jsEngine->globalObject().setProperty("log", m_jsLog);

			m_scriptTestLog->writeMessage(tr("********** Start testing of %1 **********").arg(script.fileName));

			QJSValue scriptValue = m_jsEngine->evaluate(script.script);

			if (scriptValue.isError() == true)
			{
				m_scriptTestLog->writeError(tr("Script %1 evaluate error at line %2\n"
									"\tClass: %3\n"
									"\tStack: %4\n"
									"\tMessage: %5")
								 .arg(script.fileName)
								 .arg(scriptValue.property("lineNumber").toInt())
								 .arg(metaObject()->className())
								 .arg(scriptValue.property("stack").toString())
								 .arg(scriptValue.toString()));

				continue;
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
			m_result = runScriptFunction("initTestCase");
			if (m_result == false)
			{
				return;
			}

			// Call all functions which starts from 'test', like 'testAfbNot()'
			//
			QStringList testList;

			QJSValueIterator it(m_jsEngine->globalObject());
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
				runScriptFunction("init");

				if (bool testOk = runScriptFunction(testFunc);
						testOk == true)
				{
					m_scriptTestLog->writeMessage(testFunc + ": ok");
				}
				else
				{
					failed ++;
					totalFailed ++;
					m_scriptTestLog->writeError(testFunc + ": FAILED");
				}

				// cleanup() - called after every test function.
				//
				runScriptFunction("cleanup");
			}

			// cleanup() - will be called after every test function.
			//
			runScriptFunction("cleanupTestCase");

			qint64 elapsedMsTotal = timer.elapsed();

			if (failed != 0)
			{
				m_scriptTestLog->writeError(tr("Totals: %1 tests, %2 failed, %3ms").arg(testList.size()).arg(failed).arg(elapsedMsTotal));
			}
			else
			{
				m_scriptTestLog->writeMessage(tr("Totals: %1 tests, %2 failed, %3ms").arg(testList.size()).arg(failed).arg(elapsedMsTotal));
			}

			m_scriptTestLog->writeMessage(tr("********** Finished testing of %1 **********").arg(script.fileName));
		}
	}
	catch(...)
	{
		m_scriptTestLog->writeMessage(tr("Interrupted..."));
		m_result = false;
	}

	m_result = (totalFailed == 0);

	emit finished();
}

void TestWorker::stop()
{
	//m_jsStop = true;
}

bool TestWorker::runScriptFunction(const QString& functionName)
{
	QJSValue funcProp = m_jsEngine->globalObject().property(functionName);
	if (funcProp.isUndefined() == true)
	{
		return false;
	}

	if (funcProp.isCallable() == false)
	{
		m_scriptTestLog->writeError(tr("%1 is callable function").arg(functionName));
		return false;
	}

	Q_ASSERT(m_jsThis.isUndefined() == false && m_jsThis.isObject() == true);
	Q_ASSERT(m_jsLog.isUndefined() == false && m_jsLog.isObject() == true);

	// Run script function
	//
	QJSValue result = funcProp.call(QJSValueList{} << m_jsThis);

	if (m_jsEngine->isInterrupted())
	{
		throw 1;
	}

	// Log errors and exit
	//
	if (result.isError() == true)
	{
		if (result.errorType() == QJSValue::ErrorType::GenericError)
		{
			// Assume that JS code must report about the error
			//
			m_scriptTestLog->writeError(tr("Error, stack trace: %1\n\tMessage: %2")
							 .arg(result.property("stack").toString())
							 .arg(result.toString()));
		}
		else
		{
			m_scriptTestLog->writeError(tr("Error at line %1\n"
								"\tStack: %2\n"
								"\tMessage: %3")
							 .arg(result.property("lineNumber").toInt())
							 .arg(result.property("stack").toString())
							 .arg(result.toString()));
		}

		return false;
	}

	return true;
}

//
// ----------------------------------- TestWorkerThread -----------------------------
//

TestWorkerThread::TestWorkerThread(TestController* testController, ScriptTestLog* scriptTestLog, QObject *parent):
	m_testWorker(testController, scriptTestLog)
{
	Q_UNUSED(parent);

	m_testWorker.moveToThread(&m_thread);

	QObject::connect(&m_thread, &QThread::started, &m_testWorker, &TestWorker::run);
	QObject::connect(&m_testWorker, &TestWorker::finished, this, &TestWorkerThread::workerFinished);

	return;
}

const TestWorker& TestWorkerThread::worker() const
{
	return m_testWorker;
}

TestWorker& TestWorkerThread::worker()
{
	return m_testWorker;
}

void TestWorkerThread::run()
{
	if (m_thread.isRunning() == true)
	{
		Q_ASSERT(false);
		return;
	}

	m_thread.start();

	return;
}

void TestWorkerThread::stop()
{
	m_testWorker.stop();

	return;
}

bool TestWorkerThread::isRunning() const
{
	return m_thread.isRunning();
}

void TestWorkerThread::workerFinished()
{
	m_thread.quit();
	m_thread.wait();

	emit finished(0);

	return;
}

