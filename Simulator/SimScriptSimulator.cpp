#include "../ClientLib/ScriptTestObserver.h"

#include "SimScriptLogicModule.h"
#include "SimScriptSimulator.h"
#include "SimTestObserver.h"
#include "Simulator.h"


namespace Sim
{
	ScriptWorkerThread::ScriptWorkerThread(ScriptSimulator* scriptSimulator) :
		QThread{},
		m_scriptSimulator{scriptSimulator},
		m_log{m_scriptSimulator->log()}
	{
		assert(m_scriptSimulator);

		setObjectName("Sim::ScriptWorkerThread");

		connect(this, &QThread::started, scriptSimulator->simulator(), &Simulator::scriptStarted);
		connect(this, &QThread::finished, scriptSimulator->simulator(), &Simulator::scriptFinished);

		return;
	}

	void ScriptWorkerThread::run()
	{
		m_result = true;

		if (m_scriptSimulator == nullptr)
		{
			m_log.writeError(tr("Internal error: m_scriptSimulator == nullptr"));
			m_result = false;
			return;
		}

		// Run watchdog thread
		//
		QFuture<void> wdResult = QtConcurrent::run(
			[waitThread = this, timeout = this->m_scriptSimulator->executionTimeout(), log = ScopedLog{m_log}]() mutable
			{
				bool ok = waitThread->wait(static_cast<unsigned long>(timeout));
				if (ok == false)
				{
					log.writeError("Script execution timeout.");
					waitThread->interruptScript();
				}
			});

		Q_UNUSED(wdResult);

		// --
		//
		int totalFailed = 0;

		try // runScriptFunction() can throw an exception in case of script interruption
		{
			for (const SimScriptItem& script : m_scripts)
			{
				if (m_jsEngine != nullptr)
				{
					m_jsEngine->collectGarbage();
					m_jsEngine.reset();
				}

				m_jsEngine = std::make_unique<QJSEngine>(); // Creating new QJSEngine clears all old context
				m_jsEngine->installExtensions(QJSEngine::ConsoleExtension);

				m_log.writeMessage(tr("********** Start testing of %1 **********").arg(script.scriptCaption));

				m_jsThis = m_jsEngine->newQObject(m_scriptSimulator);
				QJSEngine::setObjectOwnership(m_scriptSimulator, QJSEngine::CppOwnership);

				m_jsLog = m_jsEngine->newQObject(&m_log);
				QJSEngine::setObjectOwnership(&m_log, QJSEngine::CppOwnership);

				m_jsEngine->globalObject().setProperty("log", m_jsLog);
				m_jsEngine->globalObject().setProperty("isSimulator", QJSValue{true});
				m_jsEngine->globalObject().setProperty("isTestSuite", QJSValue{false});

				// Evaluate GlobalScript
				//
				if (QJSValue globalScriptValue = m_jsEngine->evaluate(m_globalScript.script);
					globalScriptValue.isError() == true)
				{
					m_log.writeError(tr("GlobalScript %1 evaluate error at line %2\n"
										"\tClass: %3\n"
										"\tStack: %4\n"
										"\tMessage: %5")
										 .arg(m_globalScript.scriptCaption)
										 .arg(globalScriptValue.property("lineNumber").toInt())
										 .arg(metaObject()->className())
										 .arg(globalScriptValue.property("stack").toString())
										 .arg(globalScriptValue.toString()));

					m_result = false;
					return;
				}

				// Evaluate script
				//
				if (QJSValue scriptValue = m_jsEngine->evaluate(script.script);
					scriptValue.isError() == true)
				{
					m_log.writeError(tr("Script %1 evaluate error at line %2\n"
										"\tClass: %3\n"
										"\tStack: %4\n"
										"\tMessage: %5")
										 .arg(script.scriptCaption)
										 .arg(scriptValue.property("lineNumber").toInt())
										 .arg(metaObject()->className())
										 .arg(scriptValue.property("stack").toString())
										 .arg(scriptValue.toString()));

					m_result = false;
					return;
				}

				// If constant SkipOnBuild is true, then skip this test
				//
				if (m_scriptSimulator->checkSkipOnBuildConst() == true)
				{
					QJSValue prop = m_jsEngine->globalObject().property("SkipOnBuild");
					QVariant skipOnBuild = prop.toVariant();

					if (skipOnBuild.isValid() == true &&
						skipOnBuild.typeId() == QMetaType::Bool &&
						skipOnBuild.toBool() == true)
					{
						m_log.writeWarning(tr("Test %1 skipped (as SkipOnBuild is true)").arg(script.scriptCaption));
						continue;
					}
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
						m_log.writeMessage(testFunc + ": ok");
					}
					else
					{
						failed++;
						totalFailed++;
						m_log.writeError(testFunc + ": FAILED");
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
					m_log.writeError(tr("Totals: %1 tests, %2 failed, %3ms").arg(testList.size()).arg(failed).arg(elapsedMsTotal));
				}
				else
				{
					m_log.writeMessage(tr("Totals: %1 tests, %2 failed, %3ms").arg(testList.size()).arg(failed).arg(elapsedMsTotal));
				}

				m_log.writeMessage(tr("********** Finished testing of %1 **********").arg(script.scriptCaption));
			}
		}
		catch (...)
		{
			m_log.writeText(tr("Interrupted..."));
			m_result = false;
		}

		m_scriptSimulator->simulator()->control().stop();
		m_scriptSimulator->simulator()->clear();

		m_result = (totalFailed == 0);

		if (m_jsEngine != nullptr)
		{
			m_jsEngine->collectGarbage();
			m_jsEngine.reset();
		}

		return;
	}

	bool ScriptWorkerThread::runScriptFunction(const QString& functionName)
	{
		QJSValue funcProp = m_jsEngine->globalObject().property(functionName);
		if (funcProp.isUndefined() == true)
		{
			return false;
		}

		if (funcProp.isCallable() == false)
		{
			m_log.writeError(tr("%1 is callable function").arg(functionName));
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
				m_log.writeError(tr("Error, stack trace: %1\n\tMessage: %2")
									 .arg(result.property("stack").toString())
									 .arg(result.toString()));
			}
			else
			{
				m_log.writeError(tr("Error at line %1\n"
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

	void ScriptWorkerThread::start(QThread::Priority priority /* = InheritPriority*/)
	{
		m_result = true;

		QThread::start(priority);
		return;
	}

	bool ScriptWorkerThread::interruptScript()
	{
		qDebug() << "ScriptWorkerThread::interruptScript()";

		// Pause must be called first!!!
		//
		m_scriptSimulator->simulator()->control().pause();

		m_jsEngine->setInterrupted(true);

		// Wait for thread finishing
		//
		wait(20'000); // timeout is 20s

		return true;
	}

	bool ScriptWorkerThread::result() const
	{
		return m_result;
	}

	void ScriptWorkerThread::setScripts(const std::vector<SimScriptItem>& scripts, const SimScriptItem& globalScript)
	{
		m_globalScript = globalScript;
		m_scripts = scripts;

		std::sort(m_scripts.begin(),
				  m_scripts.end(),
				  [](const auto& s1, const auto& s2)
				  {
					  return s1.scriptCaption < s2.scriptCaption;
				  });

		return;
	}

	ScriptSimulator::ScriptSimulator(Simulator* simulator, QObject* parent) :
		QObject(parent),
		m_simulator(simulator),
		m_log(simulator->log())
	{
		assert(m_simulator);

		m_log.setDebugMessagesEnabled(false);
	}

	ScriptSimulator::~ScriptSimulator()
	{
		stopScript();
	}

	bool ScriptSimulator::runScripts(const std::vector<SimScriptItem>& scripts, const SimScriptItem& globalScript)
	{
		if (m_simulator == nullptr)
		{
			assert(m_simulator);
			return false;
		}

		if (m_simulator->isLoaded() == false)
		{
			m_log.writeError(tr("Script start error: project is not loaded."));
			return false;
		}

		if (scripts.empty() == true)
		{
			// It is not an error, just warn user about empty script and return true
			//
			m_log.writeWarning(tr("Script is empty."));
			return true;
		}

		m_log.writeDebug(tr("Start script(s)"));

		// Set script and start thread
		//
		m_workerThread.setScripts(scripts, globalScript);
		m_workerThread.start();

		return true;
	}

	bool ScriptSimulator::stopScript()
	{
		if (isRunning() == false)
		{
			return true;
		}

		return m_workerThread.interruptScript();
	}

	bool ScriptSimulator::isRunning() const
	{
		return m_workerThread.isRunning();
	}

	bool ScriptSimulator::wait(unsigned long msecs /*= ULONG_MAX*/)
	{
		bool ok = m_workerThread.wait(msecs);
		return ok && m_workerThread.result();
	}

	bool ScriptSimulator::result() const
	{
		if (isRunning() == true)
		{
			m_log.writeError("Cannot get script result, script is still running.");
			return false;
		}

		return m_workerThread.result();
	}

	void ScriptSimulator::throwScriptException(const QObject* object, QString text)
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

	void ScriptSimulator::debugOutput(QString str)
	{
		qDebug() << str;
	}

	bool ScriptSimulator::startForMs(int msecs)
	{
		using namespace std::chrono;

		QJSEngine* jsEngine = qjsEngine(this);
		if (jsEngine == nullptr)
		{
			assert(jsEngine);
		}

		if (m_simulator->isRunning() == true)
		{
			m_log.writeWarning(tr("Simulation already running"));
			return false;
		}

		milliseconds durationMs{msecs};
		microseconds durationUs = duration_cast<microseconds>(durationMs);

		bool ok = m_simulator->control().startSimulation(durationUs);
		if (ok == false)
		{
			jsEngine->throwError(tr("Start simulation error."));
			return false;
		}

		// This is blocking call, wait for finishing simulation
		//
		while (m_simulator->control().isRunning() == true)
		{
			QThread::msleep(0);
		}

		return ok;
	}

	bool ScriptSimulator::waitForMs(int msecs)
	{
		return startForMs(msecs);
	}

	bool ScriptSimulator::reset()
	{
		if (m_simulator->isRunning() == true)
		{
			m_simulator->control().stop();
		}

		m_simulator->control().reset();
		m_simulator->control().setRunList({});

		return true;
	}

	QJSValue ScriptSimulator::createObserver()
	{
		QJSValue result;

		QJSEngine* jsEngine = qjsEngine(this);
		if (jsEngine == nullptr)
		{
			assert(jsEngine);
			return result;
		}

		ScriptTestObserver* observer = new ScriptTestObserver{std::make_unique<Sim::TestObserver>(*m_simulator), m_log.logInterface(), nullptr};
		result = jsEngine->newQObject(observer);

		return result;
	}

	QJSValue ScriptSimulator::signalState(QString appSignalId)
	{
		QJSValue result;

		bool ok = false;
		AppSignalState state = m_simulator->appSignalManager().signalState(appSignalId, &ok, true);

		if (ok == false)
		{
			throwScriptException(this, tr("signalState(%1), signal not found.").arg(appSignalId));
			result = -1;
			return result;
		}

		QJSEngine* jsEngine = qjsEngine(this);
		if (jsEngine == nullptr)
		{
			assert(jsEngine);
			return result;
		}

		result = jsEngine->toScriptValue(state);
		return result;
	}

	double ScriptSimulator::signalValue(QString appSignalId)
	{
		bool ok = false;
		AppSignalState state = m_simulator->appSignalManager().signalState(appSignalId, &ok, true);

		if (ok == false)
		{
			throwScriptException(this, tr("signalValue(%1), signal not found.").arg(appSignalId));
			return -1;
		}

		return state.value();
	}

	bool ScriptSimulator::overrideSignalValue(QString appSignalId, double value)
	{
		if (m_simulator->overrideSignals().isSignalInOverrideList(appSignalId) == false)
		{
			int count = m_simulator->overrideSignals().addSignals(QStringList{} << appSignalId);
			if (count != 1)
			{
				return false;
			}
		}

		m_simulator->overrideSignals().setValue(appSignalId, OverrideSignalMethod::Value, value);
		return true;
	}

	bool ScriptSimulator::waitForSignalOverrides(qint64 timeoutMs)
	{
		Q_UNUSED(timeoutMs);

		// Always wait for one workcycle, it is a simulation, it is enough.
		//
		return startForMs(1);
	}

	bool ScriptSimulator::expectSignalValue(QString appSignalId, qint64 timeoutMs, double value, double tolerance)
	{
		auto deadline = m_simulator->control().controlData().m_currentTime + std::chrono::milliseconds{timeoutMs};

		auto isEqual = [](double expectedValue, double value, double tolerance)
		{
			if (std::isnan(expectedValue) == true && std::isnan(value) == true)
			{
				return true;
			}

			if (std::isinf(expectedValue) == true && std::isinf(value) == true && std::signbit(expectedValue) == std::signbit(value))
			{
				return true;
			}

			return std::abs(expectedValue - value) <= tolerance;
		};

		while (isEqual(value, signalValue(appSignalId), tolerance) == false && m_simulator->control().controlData().m_currentTime < deadline)
		{
			startForMs(1);
		}

		return isEqual(value, signalValue(appSignalId), tolerance) == true;
	}

	void ScriptSimulator::overridesReset(qint64 /*timeoutMs*/ /*= 5000*/, QStringList excludeAppSignals /*= {}*/)
	{
		QStringList signalsToRemove = m_simulator->overrideSignals().overrideSignalIds();
		for (const QString& s : excludeAppSignals)
		{
			signalsToRemove.removeAll(s);
		}
		
		m_simulator->overrideSignals().removeSignals(signalsToRemove);

		startForMs(1);
	}

	QStringList ScriptSimulator::getOverridenSignals() const
	{
		return m_simulator->overrideSignals().overrideSignalIds();
	}

	bool ScriptSimulator::logicModuleExists(QString equipmentId) const
	{
		auto lm = m_simulator->logicModule(equipmentId);
		return lm != nullptr;
	}

	QJSValue ScriptSimulator::logicModule(QString equipmentId)
	{
		auto lm = m_simulator->logicModule(equipmentId);
		if (lm == nullptr)
		{
			throwScriptException(this, tr("LogicModule %1 not found").arg(equipmentId));
			return {};
		}

		QJSEngine* jsEngine = qjsEngine(this);
		if (jsEngine == nullptr)
		{
			assert(jsEngine);
			return {};
		}

		ScriptLogicModule* slm = new ScriptLogicModule{lm};
		return jsEngine->newQObject(slm);
	}

	QJSValue ScriptSimulator::connection(QString connectionID)
	{
		auto conn = m_simulator->connections().connection(connectionID);
		if (conn == nullptr)
		{
			throwScriptException(this, tr("Connection %1 not found").arg(connectionID));
			return {};
		}

		QJSEngine* jsEngine = qjsEngine(this);
		if (jsEngine == nullptr)
		{
			assert(jsEngine);
			return {};
		}

		ScriptConnection* sconn = new ScriptConnection{conn};
		return jsEngine->newQObject(sconn);
	}

	void ScriptSimulator::connectionsSetEnabled(bool value)
	{
		auto connections = m_simulator->connections().connections();
		for (auto c : connections)
		{
			c->setEnabled(value);
		}

		return;
	}

	bool ScriptSimulator::signalExists(QString appSignalId) const
	{
		return m_simulator->appSignalManager().signalExists(appSignalId);
	}

	AppSignalParam ScriptSimulator::signalParam(QString appSignalId)
	{
		bool ok = false;

		AppSignalParam result = m_simulator->appSignalManager().signalParam(appSignalId, &ok);
		if (ok == false)
		{
			throwScriptException(this, tr("signalParam(%1), signal not found.").arg(appSignalId));
		}

		return result;
	}


	ScriptSignal ScriptSimulator::signalParamExt(QString appSignalId)
	{
		ScriptSignal scriptSignal;

		std::optional<AppSignal> s = m_simulator->appSignalManager().signalParamExt(appSignalId);
		if (s.has_value() == false)
		{
			throwScriptException(this, tr("signalParamExt(%1), signal not found.").arg(appSignalId));
			return scriptSignal;
		}

		scriptSignal.setSignal(s.value());
		return scriptSignal;
	}

	ScriptLmDescription ScriptSimulator::scriptLmDescription(QString equipmentId)
	{
		ScriptLmDescription lmDesc;

		std::shared_ptr<LogicModule> lm = m_simulator->logicModule(equipmentId);

		if (lm == nullptr)
		{
			throwScriptException(this, tr("scriptLmDescription(%1), LM is not found.").arg(equipmentId));
			return lmDesc;
		}

		lmDesc.setLogicModule(lm);

		return lmDesc;
	}

	ScriptDevUtils ScriptSimulator::devUtils()
	{
		return ScriptDevUtils{m_simulator};
	}

	RamAddress ScriptSimulator::createRamAddress()
	{
		return {};
	}

	RamAddress ScriptSimulator::createRamAddress(int offset, int bit)
	{
		return {static_cast<quint32>(offset), static_cast<quint32>(bit)};
	}

	ScopedLog& ScriptSimulator::log()
	{
		return m_log;
	}

	QString ScriptSimulator::buildPath() const
	{
		return m_simulator->buildPath();
	}

	QString ScriptSimulator::projectName() const
	{
		return m_simulator->projectName();
	}

	int ScriptSimulator::buildNo() const
	{
		return m_simulator->buildNo();
	}

	qint64 ScriptSimulator::executionTimeout() const
	{
		return m_executionTimeout;
	}

	void ScriptSimulator::setExecutionTimeout(qint64 value)
	{
		m_executionTimeout = value;
	}

	bool ScriptSimulator::checkSkipOnBuildConst() const
	{
		return m_checkSkipOnBuildConst;
	}

	void ScriptSimulator::setCheckSkipOnBuildConst(bool value)
	{
		m_checkSkipOnBuildConst = value;
	}

	Simulator* ScriptSimulator::simulator()
	{
		return m_simulator;
	}

	const Simulator* ScriptSimulator::simulator() const
	{
		return m_simulator;
	}

	bool ScriptSimulator::unlockTimer() const
	{
		return m_simulator->control().speedFactor() >= 128.0;
	}

	void ScriptSimulator::setUnlockTimer(bool value)
	{
		m_simulator->control().setSpeedFactor(value ? 256.0 : 1.0);
	}

	double ScriptSimulator::speedFactor() const
	{
		return m_simulator->control().speedFactor();
	}

	void ScriptSimulator::setSpeedFactor(double value)
	{
		m_simulator->control().setSpeedFactor(value);
	}

	bool ScriptSimulator::enabledLanComm() const
	{
		return m_simulator->software().enabled();
	}

	void ScriptSimulator::setEnabledLanComm(bool value)
	{
		m_simulator->software().setEnabled(value);
	}
} // namespace Sim
