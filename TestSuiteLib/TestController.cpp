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
								   ILogFile* testLog,
								   IInputController& inputController,
								   IOutputController& outputController,
								   QObject* parent) :
		QObject{parent},
		m_configuration{configuration},
		m_softwareInfo{softwareInfo},
		m_signalDataServer{signalDataServer},
		m_appLog{appLog},
		m_testLog{testLog},
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
		if (m_debugMessagesEnabled == true)
		{
			qDebug() << str;
			m_appLog->writeMessage("<DEBUG> " + str);
		}
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

		ScriptTestObserver* observer = new ScriptTestObserver{std::move(testObserver), m_testLog, this};
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

		if (state.isStateAvailable() == false || state.isValid() == false)
		{
			return std::numeric_limits<double>::quiet_NaN();
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

	bool TestController::expectSignalValue(QString appSignalId, qint64 timeoutMs, double value, double tolerance /*= 0*/)
	{
		// Before expecting any values, wait for all writing processes are finished
		//
		quint64 elapsedMs = 0;
		bool allWritten = m_outputController.waitForAllSignalsWritten(timeoutMs, elapsedMs);
		if (allWritten == false)
		{
			return false;
		}

		return m_inputController.expectSignalValue(appSignalId, timeoutMs - elapsedMs, value, tolerance);
	}

	void TestController::overridesReset(qint64 timeoutMs, QStringList excludeAppSignals)
	{
		for (const QString& appSignalId : m_overridenSignals)
		{
			AppSignalParam asp = signalParam(appSignalId);

			// Exclude specified signals
			//
			if (excludeAppSignals.contains(appSignalId) == true)
			{
				continue;
			}

			bool ok = m_outputController.writeSignalValue(appSignalId, asp.tuningDefaultValueToVariant().toDouble());
			if (ok == false)
			{
				throwScriptException(this, tr("overrideSignalValue(%1, ...), signal write error.").arg(appSignalId));
				return;
			}
		}

		std::erase_if(m_overridenSignals, [&excludeAppSignals](const QString& s)
					  {
						  return excludeAppSignals.contains(s) == false;
					  });

		bool ok = waitForSignalOverrides(timeoutMs);
		if (ok == false)
		{
			throwScriptException(this, tr("waitForSignalOverrides failed."));
			return;
		}
	}

	QStringList TestController::getOverridenSignals() const
	{
		QStringList result;
		result.reserve(m_overridenSignals.size());
		for (const QString& appSignalId : m_overridenSignals)
		{
			result.push_back(appSignalId);
		}
		return result;
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

	bool TestController::tuningSourceIsActive(QString lmEquipmentId)
	{
		return m_outputController.tuningSourceIsActive(lmEquipmentId);
	}

	bool TestController::tuningSourceIsInactive(QString lmEquipmentId)
	{
		return m_outputController.tuningSourceIsInactive(lmEquipmentId);
	}

	bool TestController::activateTuningSource(QString lmEquipmentId, bool activate)
	{
		return m_outputController.activateTuningSource(lmEquipmentId, activate);
	}

	QString TestController::projectName() const
	{
		return m_configuration.configInfo.project;
	}

	int TestController::buildNo() const
	{
		return m_configuration.configInfo.buildNo;
	}

	qint64 TestController::executionTimeout() const
	{
		return m_executionTimeout.load();
	}

	void TestController::setExecutionTimeout(qint64 value)
	{
		m_executionTimeout.store(value);
	}

	bool TestController::debugMessagesEnabled() const
	{
		return m_debugMessagesEnabled;
	}

	void TestController::setDebugMessagesEnabled(bool value)
	{
		m_debugMessagesEnabled = true;
	}

} // namespace TestSuite
