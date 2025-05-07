#include "ScriptRunnerImpl.h"
#include "ScriptTestLog.h"
#include "ScriptTestSuiteApplication.h"
#include <TestSuiteLib/ScriptRunner.h>

#include <QFileInfo>
#include <QJSValueIterator>
#include <QRegularExpression>

// #define DO_NOT_SET_CONTROL_STATUS

#include <QMutexLocker>

namespace TestSuite
{
	ScriptRunnerImpl::ScriptRunnerImpl(QString softwareEquipmentId,
									   const TestScript& script,
									   std::optional<TestScript> globalScript,
									   TestController& testController,
									   ILogFile& scriptTestLog,
									   ControlStatus& status,
									   QMutex& statusMutex) :
		m_testController(testController),
		m_scriptTestLog{std::make_unique<ScriptTestLog>(scriptTestLog)},
		m_app{std::make_unique<::ScriptTestSuiteApplication>(softwareEquipmentId)},
		m_status(status),
		m_statusMutex(statusMutex),
		m_scriptInfo(script.fileName())
	{
		m_jsEngine.installExtensions(QJSEngine::ConsoleExtension);

		// Controllers
		//
		m_jsTestController = m_jsEngine.newQObject(&m_testController);
		QJSEngine::setObjectOwnership(&m_testController, QJSEngine::CppOwnership);

		// Log
		//
		m_jsLog = m_jsEngine.newQObject(m_scriptTestLog.get());
		QJSEngine::setObjectOwnership(m_scriptTestLog.get(), QJSEngine::CppOwnership);

		// theApp
		//
		m_jsApp = m_jsEngine.newQObject(m_app.get());
		QJSEngine::setObjectOwnership(m_app.get(), QJSEngine::CppOwnership);

		m_jsEngine.globalObject().setProperty("log", m_jsLog);
		m_jsEngine.globalObject().setProperty("isSimulator", QJSValue{false});
		m_jsEngine.globalObject().setProperty("isTestSuite", QJSValue{true});
		m_jsEngine.globalObject().setProperty("app", m_jsApp);

		// Evaluate scripts
		//
		if (globalScript.has_value() == true)
		{
			// Evaluate global script
			//
			QString errorMsg;
			ScriptInfo scriptInfo{"GlobalScript"};
			if (evaluateScript(globalScript.value(), scriptInfo, errorMsg) == false)
			{
				m_scriptTestLog->writeError(errorMsg);
			}
		}

		// Evaluate script
		//
		QString errorMsg;
		m_scriptInfo.fileName = script.fileName();
		if (evaluateScript(script, m_scriptInfo, errorMsg) == false)
		{
			m_scriptTestLog->writeError(errorMsg);
		}

		return;
	}

	ScriptRunnerImpl::~ScriptRunnerImpl() = default;

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

	bool ScriptRunnerImpl::queryPermission(bool& allowGlobal, bool& allowLocal)
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
#ifndef DO_NOT_SET_CONTROL_STATUS
			{
				QMutexLocker l(&m_statusMutex);
				m_status.m_testFunction = m_scriptInfo.globalAllowFunction;
			}
#endif
			RunScriptError allowGlobalResult = runScriptFunction(m_scriptInfo.globalAllowFunction);
			if (allowGlobalResult != RunScriptError::Success)
			{
				allowGlobal = false;
			}
		}

		// allow<SCRIPT_FILE_NAME>() - will be called to check if current script running is allowed
		//
		if (m_scriptInfo.allowFunction.isEmpty() == false)
		{
#ifndef DO_NOT_SET_CONTROL_STATUS
			{
				QMutexLocker l(&m_statusMutex);
				m_status.m_testFunction = m_scriptInfo.allowFunction;
			}
#endif
			RunScriptError allowResult = runScriptFunction(m_scriptInfo.allowFunction);
			if (allowResult != RunScriptError::Success)
			{
				allowLocal = false;
			}
		}

		return true;
	}

	bool ScriptRunnerImpl::runTests(const TestScriptSelection& filter)
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

				QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(matchValue ? mask : mask.right(mask.length() - 1)));

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

		m_scriptTestLog->writeMessage(tr("********** Start test script %1 **********").arg(m_scriptInfo.fileName));

		// Write plant name, unit name and system name to test log if they are specified in configuration
		//
		if (m_plant.isEmpty() == false)
		{
			m_scriptTestLog->writeMessage(m_plant, "PLANT");
		}

		if (m_unit.isEmpty() == false)
		{
			m_scriptTestLog->writeMessage(m_unit, "UNIT");
		}

		if (m_system.isEmpty() == false)
		{
			m_scriptTestLog->writeMessage(m_system, "SYSTEM");
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
			m_scriptTestLog->writeError(m_scriptInfo.fileName + tr(": queryPermission() failed, test terminated."));
			throw 1;
		}
		if (allowGlobal == false)
		{
			m_scriptTestLog->writeError(m_scriptInfo.fileName + tr(": no global permission: script is not allowed to run."));
			throw 1;
		}
		if (allowLocal == false)
		{
			m_scriptTestLog->writeError(m_scriptInfo.fileName + tr(": no local permission: script is not allowed to run."));
			throw 1;
		}

#ifndef DO_NOT_SET_CONTROL_STATUS
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_testFunction = "initTestCase";
		}
#endif

		// initTestCase() - will be called before the first test function is executed.
		//
		{
			RunScriptError initTestCaseResult = runScriptFunction("initTestCase");

			if (initTestCaseResult == RunScriptError::FunctionNotDefined)
			{
				m_scriptTestLog->writeWarning(m_scriptInfo.fileName + tr(": initTestCase() not defined."));
			}

			if (initTestCaseResult != RunScriptError::Success && initTestCaseResult != RunScriptError::FunctionNotDefined)
			{
				m_scriptTestLog->writeError(m_scriptInfo.fileName + tr(": initTestCase() failed, test terminated."));
				return false;
			}
		}

#ifndef DO_NOT_SET_CONTROL_STATUS
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_testIndex = 0;
			m_status.m_testCount = testsToRun.count();
		}
#endif

		int failed = 0;
		for (const QString& testFunc : testsToRun)
		{
			// Check script running permission
			//
			allowGlobal = true;
			allowLocal = true;
			if (queryPermission(allowGlobal, allowLocal) == false)
			{
				m_scriptTestLog->writeError(m_scriptInfo.fileName + tr(": queryPermission() failed, test terminated."));
				throw 1;
			}
			if (allowGlobal == false)
			{
				m_scriptTestLog->writeError(m_scriptInfo.fileName + tr(": no global permission: script is not allowed to run."));
				throw 1;
			}
			if (allowLocal == false)
			{
				m_scriptTestLog->writeError(m_scriptInfo.fileName + tr(": no local permission: script is not allowed to run."));
				throw 1;
			}

#ifndef DO_NOT_SET_CONTROL_STATUS
			{
				QMutexLocker l(&m_statusMutex);
				m_status.m_testIndex++;
				m_status.m_testFunction = testFunc;
			}
#endif

			// Find test caption
			//
			QString testCaption = m_scriptInfo.testCaption(testFunc);

			// Mark test function start time
			//
			qint64 startMsTestFunc = timer.elapsed();

			m_scriptTestLog->writeMessage(testCaption + ": RUN");

			emit testStarted(m_scriptInfo.fileName, testFunc);

			RunScriptError testResult = RunScriptError::FunctionNotDefined;
			RunScriptError cleanupResult = RunScriptError::Success;

			// init() - called before each test function is executed.
			//
			RunScriptError initResult = runScriptFunction("init");
			if (initResult == RunScriptError::FunctionNotDefined)
			{
				m_scriptTestLog->writeWarning(tr("%1: init() not defined.").arg(testCaption));
			}

			if (initResult != RunScriptError::Success && initResult != RunScriptError::FunctionNotDefined)
			{
				m_scriptTestLog->writeError(tr("%1: init() failed, test terminated.").arg(testCaption));
			}
			else
			{
				// run test function
				//
				testResult = runScriptFunction(testFunc);
				if (testResult == RunScriptError::Success)
				{
					m_scriptTestLog->writeMessage(tr("%1: %2").arg(testCaption).arg(ConstStrings::TEST_PASSED()));
				}
				else
				{
					failed++;
					// totalFailed ++;
					m_scriptTestLog->writeError(tr("%1: %2").arg(testCaption).arg(ConstStrings::TEST_FAILED()));
				}

				// cleanup() - called after every test function.
				//
				cleanupResult = runScriptFunction("cleanup");
				if (cleanupResult == RunScriptError::FunctionNotDefined)
				{
					m_scriptTestLog->writeWarning(tr("%1: cleanup() not defined.").arg(testCaption));
				}

				if (cleanupResult != RunScriptError::Success && cleanupResult != RunScriptError::FunctionNotDefined)
				{
					m_scriptTestLog->writeError(tr("%1: cleanup() failed, test terminated.").arg(testCaption));
				}
			}

			bool initOk = initResult == RunScriptError::Success || initResult == RunScriptError::FunctionNotDefined;
			bool testOk = testResult == RunScriptError::Success;
			bool cleanupOk = cleanupResult == RunScriptError::Success || cleanupResult == RunScriptError::FunctionNotDefined;

			bool testFuncResult = initOk && testOk && cleanupOk;

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
#ifndef DO_NOT_SET_CONTROL_STATUS
		{
			QMutexLocker l(&m_statusMutex);
			m_status.m_testFunction = "cleanupTestCase";
		}
#endif

		{
			RunScriptError cleanupTestCaseResult = runScriptFunction("cleanupTestCase");

			if (cleanupTestCaseResult == RunScriptError::FunctionNotDefined)
			{
				m_scriptTestLog->writeWarning(m_scriptInfo.fileName + tr(": cleanupTestCase() not defined."));
			}

			if (cleanupTestCaseResult != RunScriptError::Success && cleanupTestCaseResult != RunScriptError::FunctionNotDefined)
			{
				m_scriptTestLog->writeError(m_scriptInfo.fileName + tr(": cleanupTestCase() failed, test terminated."));
			}
		}

		qint64 elapsedMsTotal = timer.elapsed();

		if (failed == 0)
		{
			m_scriptTestLog->writeMessage(tr("Totals: %1 tests, %2 failed, %3ms").arg(testsToRun.count()).arg(failed).arg(elapsedMsTotal));
		}
		else
		{
			m_scriptTestLog->writeError(tr("Totals: %1 tests, %2 failed, %3ms").arg(testsToRun.count()).arg(failed).arg(elapsedMsTotal));
		}

		m_scriptTestLog->writeMessage(tr("********** Finished test script %1 **********").arg(m_scriptInfo.fileName));

		// Write report messages
		//
		reportStrings.insert(reportStrings.begin(),
							 tr("%1;%2;%3")
								 .arg(m_scriptInfo.scriptCaption)
								 .arg(failed == 0 ? ConstStrings::TEST_PASSED() : tr("%1 %2").arg(ConstStrings::TEST_FAILED()).arg(failed))
								 .arg(timeMsToStr(elapsedMsTotal)));

		for (const QString& s : reportStrings)
		{
			m_scriptTestLog->writeMessage(s, "TEST_RESULT");
		}

		return failed == 0;
	}

	const ScriptInfo& ScriptRunnerImpl::scriptInfo() const
	{
		return m_scriptInfo;
	}

	const TestController& ScriptRunnerImpl::testController() const
	{
		return m_testController;
	}

	QString ScriptRunnerImpl::plant() const
	{
		return m_plant;
	}

	void ScriptRunnerImpl::setPlant(const QString& value)
	{
		m_plant = value;
	}

	QString ScriptRunnerImpl::unit() const
	{
		return m_unit;
	}

	void ScriptRunnerImpl::setUnit(const QString& value)
	{
		m_unit = value;
	}

	QString ScriptRunnerImpl::system() const
	{
		return m_system;
	}

	void ScriptRunnerImpl::setSystem(const QString& value)
	{
		m_system = value;
	}

	bool ScriptRunnerImpl::evaluateScript(const TestScript& script, ScriptInfo& scriptInfo, QString& errorMsg)
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

			auto cit = captionValues.find(captionVariable);
			if (cit != captionValues.end())
			{
				scriptInfo.testsCaptions[testFunc] = cit->second;
			}
		}

		// Parse script filename caption
		//
		scriptInfo.scriptCaption = script.fileName();
		{
			QString scriptCaption = QFileInfo(scriptInfo.scriptCaption).baseName(); // Get the filename without path and extension

			auto cit = captionValues.find("caption" + scriptCaption);
			if (cit != captionValues.end())
			{
				scriptInfo.scriptCaption = cit->second;
			}
		}

		return true;
	}

	RunScriptError ScriptRunnerImpl::runScriptFunction(const QString& functionName)
	{
		QJSValue funcProp = m_jsEngine.globalObject().property(functionName);
		if (funcProp.isUndefined() == true)
		{
			return RunScriptError::FunctionNotDefined;
		}

		if (funcProp.isCallable() == false)
		{
			m_scriptTestLog->writeError(tr("%1 is callable function").arg(functionName));
			return RunScriptError::IsNotCallable;
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

				callFinishedCondVariable.wait_for(locker,
												  std::chrono::milliseconds{200},
												  [&callFinished, scriptRunThread]()
												  {
													  return callFinished.load() || scriptRunThread->isInterruptionRequested();
												  });
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
		catch (...)
		{
			Q_ASSERT(false); // What the reason of this exception?
			m_scriptTestLog->writeError(tr("Unexpected exception occured in %1.").arg(functionName));

			callFinished.store(true);
			callFinishedCondVariable.notify_one();

			return RunScriptError::RuntimeError;
		}

		callFinished.store(true);
		callFinishedCondVariable.notify_one();

		// Log errors and exit
		//
		if (callResult.isError() == true)
		{
			QString stack = callResult.property("stack").toString();
			if (stack.isEmpty() == true)
			{
				m_scriptTestLog->writeError(tr("%1").arg(callResult.toString()));
			}
			else
			{
				if (callResult.errorType() == QJSValue::ErrorType::GenericError)
				{
					// Assume that JS code must report about the error
					//
					m_scriptTestLog->writeError(tr("Error, stack trace: %1\n%2").arg(stack).arg(callResult.toString()));
				}
				else
				{
					m_scriptTestLog->writeError(tr("Error at line %1\n"
												   "\tStack trace: %2\n"
												   "\t%3")
													.arg(callResult.property("lineNumber").toInt())
													.arg(stack)
													.arg(callResult.toString()));
				}
			}

			return RunScriptError::ReturnedError;
		}

		if (callResult.isBool())
		{
			return callResult.toBool() ? RunScriptError::Success : RunScriptError::ReturnedError;
		}

		return RunScriptError::Success;
	}
} // namespace TestSuite
