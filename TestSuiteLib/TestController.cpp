#include "../ClientLib/ScriptTestObserver.h"

#include "TestController.h"
#include "TestObserver.h"
#include "TestSuiteConfigController.h"

namespace TestSuite
{
	TestController::TestController(const ConfigSettings& configuration,
								   const SoftwareInfo& softwareInfo,
								   ISignalDataServer* signalDataServer,
								   ILogFile* appLog,
								   IInputController& inputController,
								   IOutputController& outputController,
								   QObject* parent) :
		QObject{parent},
		m_configuration{configuration},
		m_softwareInfo{softwareInfo},
		m_signalDataServer{signalDataServer},
		m_appLog{appLog},
		m_inputController{inputController},
		m_outputController{outputController}
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

	bool TestController::startForMs(int msecs)
	{
		return waitForMs(msecs);
	}

	bool TestController::waitForMs(int msecs)
	{
		if (msecs < 0)
		{
			return false;
		}

		qint64 nsecs = static_cast<qint64>(msecs) * 1'000'000;

		QElapsedTimer timer;
		timer.start();

		QThread* currentThread = QThread::currentThread();
		while (currentThread->isInterruptionRequested() == false)
		{
			qint64 timeLeftUs = std::min<qint64>((nsecs - timer.nsecsElapsed()) / 1'000, 100'000);
			if (timeLeftUs <= 0)
			{
				break;
			}

			QThread::usleep(static_cast<unsigned long>(timeLeftUs));
		}

		return true;
	}

	QJSValue TestController::createObserver()
	{
		QJSValue result;

		QJSEngine* jsEngine = qjsEngine(this);
		if (jsEngine == nullptr)
		{
			assert(jsEngine);
			return result;
		}

		if (m_configuration.appDataServices.empty() == true || m_signalDataServer == nullptr || m_appLog == nullptr)
		{
			jsEngine->throwError(tr("ScriptTestObserver can not be created as there is no configured AppDataService(s)."));
			return result;
		}

		auto testObserver = std::make_unique<TestSuite::TestObserver>(*m_signalDataServer, 
																	  m_softwareInfo,
																	  m_configuration.appDataServices, 
																	  m_appLog);

		ScriptTestObserver* observer = new ScriptTestObserver{std::move(testObserver), this};
		result = jsEngine->newQObject(observer);

		return result;
	}


	QJSValue TestController::signalState(QString appSignalId)
	{
		bool ok = false;
		AppSignalState state = m_inputController.signalState(appSignalId, &ok);

		if (ok == false)
		{
			throwScriptException(this, tr("signalState(%1), signal not found.").arg(appSignalId));
			return -1;
		}

		QJSEngine* jsEngine = qjsEngine(this);
		if (jsEngine == nullptr)
		{
			Q_ASSERT(jsEngine);
			return {};
		}

		return jsEngine->toScriptValue(state);
	}

	double TestController::signalValue(QString appSignalId)
	{
		bool ok = false;
		AppSignalState state = m_inputController.signalState(appSignalId, &ok);

		if (ok == false)
		{
			throwScriptException(this, tr("signalValue(%1), signal not found.").arg(appSignalId));
			return -1;
		}

		return state.value();
	}

	bool TestController::overrideSignalValue(QString appSignalId, QVariant value)
	{
		bool ok = m_outputController.writeSignalValue(appSignalId, value);
		if (ok == false)
		{
			throwScriptException(this, tr("overrideSignalValue(%1, ...), signal write error.").arg(appSignalId));
			return false;
		}
		else
		{
			m_overridenSignals.insert(appSignalId);
		}

		return ok;
	}

	bool TestController::waitForSignalOverrides(qint64 timeoutMs)
	{
		quint64 elapsedMs = 0;
		return m_outputController.waitForAllSignalsWritten(timeoutMs, elapsedMs);
	}

	bool TestController::expectSignalValue(QString appSignalId, double value, qint64 timeoutMs)
	{
		// Before expecting any values, wait for all writing processes are finished
		//
		quint64 elapsedMs = 0;
		bool allWritten = m_outputController.waitForAllSignalsWritten(timeoutMs, elapsedMs);
		if (allWritten == false)
		{
			return false;
		}

		return m_inputController.expectSignalValue(appSignalId, value, timeoutMs - elapsedMs);
	}

	void TestController::overridesReset(qint64 timeoutMs)
	{
		for (const QString& appSignalId : m_overridenSignals)
		{
			AppSignalParam asp = signalParam(appSignalId);

			bool ok = m_outputController.writeSignalValue(appSignalId, asp.tuningDefaultValueToVariant());
			if (ok == false)
			{
				throwScriptException(this, tr("overrideSignalValue(%1, ...), signal write error.").arg(appSignalId));
				return;
			}
		}
		m_overridenSignals.clear();

		bool ok = waitForSignalOverrides(timeoutMs);
		if (ok == false)
		{
			throwScriptException(this, tr("waitForSignalOverrides failed."));
			return;
		}
	}

	bool TestController::signalExists(QString appSignalId) const
	{
		return m_inputController.signalExists(appSignalId);
	}

	AppSignalParam TestController::signalParam(QString appSignalId)
	{
		bool ok = false;

		AppSignalParam result = m_inputController.signalParam(appSignalId, &ok);
		if (ok == false)
		{
			throwScriptException(this, tr("signalParam(%1), signal not found.").arg(appSignalId));
		}

		return result;
	}

} // namespace TestSuite
