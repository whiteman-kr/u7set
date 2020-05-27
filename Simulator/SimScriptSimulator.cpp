#include "SimScriptSimulator.h"
#include "Simulator.h"

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
		if (m_scriptSimulator == nullptr)
		{
			writeError(tr("Internal error: m_scriptSimulator == nullptr"));
			m_result = false;
			return;
		}

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

		if (scriptValue.isCallable() == false)
		{
			writeError(tr("Script does not have an entry point (function(simulator){})"));
			m_result = false;
			return;
		}

		// Run timeout control thread
		//

		// Run script
		//
		QJSValue arg = m_jsEngine.newQObject(m_scriptSimulator);
		QQmlEngine::setObjectOwnership(m_scriptSimulator, QQmlEngine::CppOwnership);

		QJSValue jsResult = scriptValue.call(QJSValueList{} << arg);
		if (jsResult.isError() == true)
		{
			writeError(tr("Script error:").arg(jsResult.toString()));
			m_result = false;
			return;
		}

		bool boolResult = jsResult.toBool();
		if (boolResult == false)
		{
			writeError(tr("Script finished with result: FAILED"));
		}
		else
		{
			writeError(tr("Script finished with result: ok"));
		}

		m_result = boolResult;

		return;
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
#elif
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

	bool ScriptSimulator::runScript(QString script)
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

		writeMessage(tr("Start script"));

		m_workerThread.setScript(script);
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
		return ok & m_workerThread.result();
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

	void ScriptSimulator::debugOutput(QString str)
	{
		qDebug() << str;
	}

	bool ScriptSimulator::startForMs(int msecs)
	{
		int to_do_startForMs;
		assert(false);
		return true;
	}

	bool ScriptSimulator::reset()
	{
		int to_do_reset;
		assert(false);
		return true;
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



}
