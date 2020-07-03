#include "SimScriptSimulator.h"
#include "Simulator.h"
#include <QJSValueIterator>

namespace Sim
{
	ScriptWorkerThread::ScriptWorkerThread(ScriptSimulator* scriptSimulator) :
		QThread(),
		m_scriptSimulator{scriptSimulator}
	{
		assert(m_scriptSimulator);

		setObjectName("Sim::ScriptWorkerThread");

		m_jsEngine.installExtensions(QJSEngine::ConsoleExtension);

		return;
	}

	void ScriptWorkerThread::run()
	{
        m_result = true;

		if (m_scriptSimulator == nullptr)
		{
			writeError(tr("Internal error: m_scriptSimulator == nullptr"));
			m_result = false;
			return;
		}

		writeMessage(tr("********** Start testing of %1 **********").arg(m_testName));

        m_jsThis = m_jsEngine.newQObject(m_scriptSimulator);
        QQmlEngine::setObjectOwnership(m_scriptSimulator, QQmlEngine::CppOwnership);

		// Evaluate script
		//
		QJSValue scriptValue = m_jsEngine.evaluate(m_script);

		if (scriptValue.isError() == true)
		{
			writeError(tr("Script evaluate error at line %1\n"
						 "\tClass: %2\n"
						 "\tStack: %3\n"
						 "\tMessage: %4")
					  .arg(scriptValue.property("lineNumber").toInt())
					  .arg(metaObject()->className())
					  .arg(scriptValue.property("stack").toString())
					  .arg(scriptValue.toString()));

			m_result = false;
			return;
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
			runScriptFunction("init");

			if (bool testOk = runScriptFunction(testFunc);
				testOk == true)
			{
				writeMessage(testFunc + ": ok");
			}
			else
			{
				failed ++;
				writeError(testFunc + ": FAILED");
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
			writeError(tr("Totals: %1 tests, %2 failed, %3ms").arg(testList.size()).arg(failed).arg(elapsedMsTotal));
		}
		else
		{
			writeMessage(tr("Totals: %1 tests, %2 failed, %3ms").arg(testList.size()).arg(failed).arg(elapsedMsTotal));
		}

		writeMessage(tr("********** Finished testing of %1 **********").arg(m_testName));

		m_result = (failed == 0);
		return;
	}

    bool ScriptWorkerThread::runScriptFunction(const QString& functionName)
    {
        QJSValue funcProp = m_jsEngine.globalObject().property(functionName);
        if (funcProp.isUndefined() == true)
        {
            return false;
        }

        if (funcProp.isCallable() == false)
        {
            writeError(tr("%1 is callable function").arg(functionName));
            return false;
        }

        Q_ASSERT(m_jsThis.isUndefined() == false && m_jsThis.isObject() == true);

        // Run script function
        //
        QJSValue result = funcProp.call(QJSValueList{} << m_jsThis);

        // Log errors and exit
        //
		if (result.isError() == true)
        {
			if (result.errorType() == QJSValue::ErrorType::GenericError)
			{
				// Assume that JS code must report about the error
				//
				writeError(tr("Error, stack trace: %1\n\tMessage: %2")
						   .arg(result.property("stack").toString())
						   .arg(result.toString()));
			}
			else
			{
				writeError(tr("Error at line %1\n"
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

	void ScriptWorkerThread::start(QThread::Priority priority/* = InheritPriority*/)
	{
		m_jsEngine.moveToThread(this);
		m_result = true;

		QThread::start(priority);
		return;
	}

	bool ScriptWorkerThread::interruptScript()
	{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
		m_jsEngine.setInterrupted(true);
		return true;
#else
		return false;
#endif
	}

	bool ScriptWorkerThread::result() const
	{
		return m_result;
	}

	void ScriptWorkerThread::setScript(QString value)
	{
		m_script = value;
	}

	void ScriptWorkerThread::setTestName(QString value)
	{
		m_testName = value;
	}

	ScriptSimulator::ScriptSimulator(Simulator* simulator, QObject* parent) :
		QObject(parent),
		Output(),
		m_simulator(simulator)
	{
		assert(m_simulator);
	}

	ScriptSimulator::~ScriptSimulator()
	{
	}

	bool ScriptSimulator::runScript(QString script, QString testName)
	{
		if (m_simulator == nullptr)
		{
			assert(m_simulator);
			return false;
		}

		if (m_simulator->isLoaded() == false)
		{
			writeError(tr("Script start error: project is not loaded."));
			return false;
		}

		if (script.isEmpty())
		{
			// It is not an error, just warn user about empty script and return true
			//
			writeWaning(tr("Script is empty."));
			return true;
		}

		writeDebug(tr("Start script"));

		m_workerThread.setScript(script);
		m_workerThread.setTestName(testName);

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
			writeError("Cannot get script result, script is still running.");
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
			writeWaning(tr("Simulation already running"));
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

	QJSValue ScriptSimulator::signalState(QString appSignalId)
	{
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

	void ScriptSimulator::overridesReset()
	{
		m_simulator->overrideSignals().clear();
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

		std::optional<Signal> s = m_simulator->appSignalManager().signalParamExt(appSignalId);
		if (s.has_value() == false)
		{
			throwScriptException(this, tr("signalParamExt(%1), signal not found.").arg(appSignalId));
			return scriptSignal;
		}

		scriptSignal.setSignal(s.value());
		return scriptSignal;
	}

	ScriptDevUtils ScriptSimulator::devUtils()
	{
		return ScriptDevUtils{m_simulator};
	}

	QString ScriptSimulator::buildPath() const
	{
		return m_simulator->buildPath();
	}

	qint64 ScriptSimulator::executionTimeOut() const
	{
		return m_executionTimeOut;
	}

	void ScriptSimulator::setExecutionTimeOut(qint64 value)
	{
		m_executionTimeOut = value;
	}

	bool ScriptSimulator::unlockTimer() const
	{
		return m_simulator->control().unlockTimer();
	}

	void ScriptSimulator::setUnlockTimer(bool value)
	{
		m_simulator->control().setUnlockTimer(value);
	}

	bool ScriptSimulator::appDataTrasmittion() const
	{
		return m_simulator->appDataTransmitter().enabled();
	}

	void ScriptSimulator::setAppDataTrasmittion(bool value)
	{
		m_simulator->appDataTransmitter().setEnabled(value);
	}
}
