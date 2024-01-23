#include "ScriptRunner.h"

namespace TestSuite
{
	ScriptRunner::ScriptRunner(const TestScript& script, const TestScript* globalScript, ConfigSettings& configuration, TestController& testController, ILogFile& scriptTestLog, ControlStatus& status, QMutex& statusMutex) :
		m_configuration(configuration),
		m_testController(testController),
		m_scriptTestLog(scriptTestLog),
		m_status(status),
		m_statusMutex(statusMutex),
		m_scriptInfo(script.fileName())
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

		// Evaluate scripts
		//
		if (globalScript != nullptr)
		{
			// Evaluate global script
			//
			QString errorMsg;
			ScriptInfo scriptInfo{"GlobalScript"};
			if (evaluateScript(*globalScript, scriptInfo, errorMsg) == false)
			{
				m_scriptTestLog.writeError(errorMsg);
			}
		}

		// Evaluate script
		//
		QString errorMsg;
		m_scriptInfo.fileName = script.fileName();
		if (evaluateScript(script, m_scriptInfo, errorMsg) == false)
		{
			m_scriptTestLog.writeError(errorMsg);
		}

		return;
	}

	ScriptRunner::~ScriptRunner()
	{
	}

	static QString timeMsToStr(qint64 mss)
	{

		quint64 ms = mss % 1000;
		mss /= 1000;
		if (mss == 0)
		{
			return QObject::tr("%1 ms").arg(ms);
		}

		quint64 sec = mss % 60;
		mss /= 60;
		if (mss == 0)
		{
			return QObject::tr("%1 s %2 ms").arg(sec).arg(ms);
		}

		quint64 min = mss % 60;
		mss /= 60;
		if (mss == 0)
		{
			return QObject::tr("%1m %2s %3ms").arg(min).arg(sec).arg(ms);
		}

		return QObject::tr("%1h %2m %3s %3ms").arg(mss).arg(min).arg(sec).arg(ms);
	};

	bool ScriptRunner::queryPermission(bool& allowGlobal, bool& allowLocal)
	{
		// Check if script has functions (evaluated)
		//
		if (m_scriptInfo.empty() == true)
		{
			allowGlobal = false;
			allowLocal = false;
			return false;
		}

		allowGlobal = true;
		allowLocal = true;

		// allowGlobal() - will be called before start of the script to check if testing is allowed
		// allow<SCRIPT_FILE_NAME>() - will be called before start of the script to check if testing is allowed
		// initTestCase() - will be called before the first test function is executed.
		// cleanupTestCase() - will be called after the last test function was executed.
		// init() - will be called before each test function is executed.
		// cleanup() - will be called after every test function.
		//

		// allowGlobal() - will be called to check if testing is allowed
		//
		if (m_scriptInfo.globalAllowFunction.isEmpty() == false)
		{
			{
				QMutexLocker l(&m_statusMutex);
				m_status.m_testFunction = m_scriptInfo.globalAllowFunction;
			}

			if (bool allowGlobalResult = runScriptFunction(m_scriptInfo.globalAllowFunction);
				allowGlobalResult == false)
			{
				allowGlobal = false;
			}
		}

		// allow<SCRIPT_FILE_NAME>() - will be called to check if current script running is allowed
		//
		if (m_scriptInfo.allowFunction.isEmpty() == false)
		{
			{
				QMutexLocker l(&m_statusMutex);
				m_status.m_testFunction = m_scriptInfo.allowFunction;
			}

			if (bool allowResult = runScriptFunction(m_scriptInfo.allowFunction);
				allowResult == false)
			{
				allowLocal = false;
			}
		}

		return true;
	}

	bool ScriptRunner::runTests(const TestScriptSelection& filter)
	{
		// Check if script has functions (evaluated)
		//
		if (m_scriptInfo.empty() == true)
		{
			return true;
		}

		// Process tests filter
		//
		QStringList testsToRun;

		for (const QString& testFunc : m_scriptInfo.testsList)
		{
			bool filterMatch = true;

			// Process function list filter
			//
			const QStringList& functions = filter.selectedFunctions(m_scriptInfo.fileName);
			if (functions.empty() == false)
			{
				if (std::find(functions.begin(), functions.end(), testFunc) == functions.end())
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
				bool matchValue = mask.startsWith('-') == false; // Should match if no '-', otherwise should NOT match

				QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(
					matchValue ? mask : mask.right(mask.length() - 1)));

				if (rx.match(testFunc).hasMatch() != matchValue)
				{
					filterMatch = false;
				}
			}

			if (filterMatch == false)
			{
				continue;
			}

			testsToRun.push_back(testFunc);
		}

		if (testsToRun.isEmpty() == true)
		{
			return true;
		}
		
		QStringList reportStrings;

		m_scriptTestLog.writeMessage(tr("********** Start test script %1 **********").arg(m_scriptInfo.fileName));

		// Write plant name, unit name and system name to test log if they are specified in configuration
		//
		if (m_configuration.plant.isEmpty() == false)
		{
			m_scriptTestLog.writeMessage(m_configuration.plant, "PLANT");
		}
		if (m_configuration.unit.isEmpty() == false)
		{
			m_scriptTestLog.writeMessage(m_configuration.unit, "UNIT");
		}
		if (m_configuration.system.isEmpty() == false)
		{
			m_scriptTestLog.writeMessage(m_configuration.system, "SYSTEM");
		}
		
		// allowGlobal() - will be called before start of the script to check if testing is allowed
		// allow<SCRIPT_FILE_NAME>() - will be called before start of the script to check if testing is allowed
		// initTestCase() - will be called before the first test function is executed.
		// cleanupTestCase() - will be called after the last test function was executed.
		// init() - will be called before each test function is executed.
		// cleanup() - will be called after every test function.
		//
		QElapsedTimer timer;
		timer.start();

		// Check script running permission
		//
		bool allowGlobal = true;
		bool allowLocal = true;
		if (queryPermission(allowGlobal, allowLocal) == false)
		{
			m_scriptTestLog.writeError(m_scriptInfo.fileName + ": queryPermission() failed, test terminated.");
			throw 1;
		}
		if (allowGlobal == false)
		{
			m_scriptTestLog.writeError(m_scriptInfo.fileName + ": no global permission: script is not allowed to run.");
			throw 1;
		}
		if (allowLocal == false)
		{
			m_scriptTestLog.writeError(m_scriptInfo.fileName + ": no local permission: script is not allowed to run.");
			throw 1;
		}

		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_testFunction = "initTestCase";
		}

		// initTestCase() - will be called before the first test function is executed.
		//
		if (bool initTestCaseResult = runScriptFunction("initTestCase");
			initTestCaseResult == false)
		{
			m_scriptTestLog.writeError(m_scriptInfo.fileName + ": initTestCase() failed, test terminated.");
			return false;
		}

		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_testIndex = 0;
			m_status.m_testCount = testsToRun.count();
		}

		int failed = 0;
		for (const QString& testFunc : testsToRun)
		{
			// Check script running permission
			//
			bool allowGlobal = true;
			bool allowLocal = true;
			if (queryPermission(allowGlobal, allowLocal) == false)
			{
				m_scriptTestLog.writeError(m_scriptInfo.fileName + ": queryPermission() failed, test terminated.");
				throw 1;
			}
			if (allowGlobal == false)
			{
				m_scriptTestLog.writeError(m_scriptInfo.fileName + ": no global permission: script is not allowed to run.");
				throw 1;
			}
			if (allowLocal == false)
			{
				m_scriptTestLog.writeError(m_scriptInfo.fileName + ": no local permission: script is not allowed to run.");
				throw 1;
			}

			{
				QMutexLocker l(&m_statusMutex);
				m_status.m_testIndex++;
				m_status.m_testFunction = testFunc;
			}

			// Find test caption
			//
			QString testCaption = m_scriptInfo.testCaption(testFunc);

			// Mark test function start time
			//
			qint64 startMsTestFunc = timer.elapsed();

			m_scriptTestLog.writeMessage(testCaption + ": RUN");

			emit testStarted(m_scriptInfo.fileName, testFunc);

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
					m_scriptTestLog.writeMessage(tr("%1: %2").arg(testCaption).arg(ConstStrings::TEST_PASSED()));
				}
				else
				{
					failed++;
					//totalFailed ++;
					m_scriptTestLog.writeError(tr("%1: %2").arg(testCaption).arg(ConstStrings::TEST_FAILED()));
				}

				// cleanup() - called after every test function.
				//
				cleanupOk = runScriptFunction("cleanup");
				if (cleanupOk == false)
				{
					m_scriptTestLog.writeError(tr("%1: cleanup() failed, test terminated.").arg(testCaption));
				}
			}
			else
			{
				m_scriptTestLog.writeError(tr("%1: init() failed, test terminated.").arg(testCaption));
			}

			bool testFuncResult = initOk == true && testOk == true && cleanupOk == true;

			// Mark test function finish time
			//
			qint64 elapsedMsTestFunc = timer.elapsed();

			// Write message to the test log for generating default report
			//

			reportStrings.push_back(tr("\u2800%1;%2;%3")
										.arg(testCaption)
										.arg(testFuncResult ? ConstStrings::TEST_PASSED() : ConstStrings::TEST_FAILED())
										.arg(timeMsToStr(elapsedMsTestFunc - startMsTestFunc)));
			
			emit testFinished(m_scriptInfo.fileName, testFunc, testFuncResult);

			if (initOk == false || cleanupOk == false)
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
			m_scriptTestLog.writeError(m_scriptInfo.fileName, + ": cleanupTestCase() failed, test terminated.");
		}

		qint64 elapsedMsTotal = timer.elapsed();

		if (failed == 0)
		{
			m_scriptTestLog.writeMessage(tr("Totals: %1 tests, %2 failed, %3ms").arg(testsToRun.count()).arg(failed).arg(elapsedMsTotal));
		}
		else
		{
			m_scriptTestLog.writeError(tr("Totals: %1 tests, %2 failed, %3ms").arg(testsToRun.count()).arg(failed).arg(elapsedMsTotal));
		}

		m_scriptTestLog.writeMessage(tr("********** Finished test script %1 **********").arg(m_scriptInfo.fileName));

		// Write report messages
		//
		reportStrings.insert(reportStrings.begin(), 
			tr("%1;%2;%3")
								 .arg(m_scriptInfo.scriptCaption)
								 .arg(failed == 0 ? ConstStrings::TEST_PASSED() : tr("%1 %2").arg(ConstStrings::TEST_FAILED()).arg(failed))
								 .arg(timeMsToStr(elapsedMsTotal)));

		for (const QString& s : reportStrings)
		{
			m_scriptTestLog.writeMessage(s, "TEST_RESULT");
		}

		return failed == 0;
	}
		
	const ScriptInfo& ScriptRunner::scriptInfo() const
	{
		return m_scriptInfo;
	}

	const TestController& ScriptRunner::testController() const
	{
		return m_testController;
	}
	
	bool ScriptRunner::evaluateScript(const TestScript& script, ScriptInfo& scriptInfo, QString& errorMsg)
	{
		QFileInfo fi(script.fileName());
		const QString allogGlobalFunctionName = "allowGlobal";
		const QString allowFunctionName = tr("allow%1").arg(fi.baseName()).remove(".js", Qt::CaseInsensitive);

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

		// This map contains values of caption$TestName variables like captionWaterPressure
		// Key is variable name, value is variable value 
		//
		std::map<QString, QString> captionValues;	

		// Find all functions which starts from 'test', like 'testTgnAboveTNom(), using filter'
		//
		QJSValueIterator it(m_jsEngine.globalObject());
		while (it.hasNext() == true)
		{
			it.next();

			QString objectName = it.name();

			if (objectName.startsWith("caption"))
			{
				// This is a variable named like captionWaterPressure
				//
				QString value = it.value().toString();
				if (value.isEmpty() == false)
				{
					captionValues[objectName] = value;
				}
				continue;
			}

			// Check if this is permission function
			//
			if (objectName == allogGlobalFunctionName)
			{
				scriptInfo.globalAllowFunction = objectName;
			}

			if (objectName == allowFunctionName)
			{
				scriptInfo.allowFunction = objectName;
			}

			if (objectName.compare("ScriptTags", Qt::CaseInsensitive) == 0)
			{
				scriptInfo.scriptTags = QVariant().fromValue(it.value()).toStringList();
			}

			// Check if this is a test function
			//
			if (objectName.startsWith("test"))
			{
				// This is a variable named testTestName
				//
				scriptInfo.testsList.push_back(objectName);
			}
		}

		std::sort(scriptInfo.testsList.begin(), scriptInfo.testsList.end());

		// Build tests captions map
		//
		for (const QString& testFunc : scriptInfo.testsList)
		{
			QString captionVariable = "caption" + testFunc;
			captionVariable.replace("captiontest", "caption", Qt::CaseSensitive);
			 
			auto it = captionValues.find(captionVariable);
			if (it != captionValues.end())
			{
				scriptInfo.testsCaptions[testFunc] = it->second;
			}
		}

		// Parse script filename caption
		//
		scriptInfo.scriptCaption = script.fileName();
		{
			QFileInfo fi(scriptInfo.scriptCaption);
			QString scriptCaption = fi.baseName(); // Get the filename without path and extension

			auto it = captionValues.find("caption" + scriptCaption);
			if (it != captionValues.end())
			{
				scriptInfo.scriptCaption = it->second;
			}
		}

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

		if (callResult.isBool())
		{
			return callResult.toBool();
		}

		return true;
	}
}
