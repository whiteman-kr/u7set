#include <ClientLib/ScriptTestObserver.h>
#include <TestSuiteLib/TestController.h>

#include <QJSEngine>

#include <CommonLib/expected.hpp>


namespace TestSuite
{
	TestController::TestController(ILogFile* appLog,
								   ILogFile* testLog,
								   IInputController* inputController,
								   IOutputController* outputController,
								   QObject* parent) :
		QObject{parent},
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

		if (m_inputController == nullptr)
		{
			throwScriptException(this, tr("createObserver(), InputController is not set."));
			return result;
		}

		tl::expected<std::unique_ptr<ITestObserver>, QString> testObserver = m_inputController->createTestObserver();

		if (testObserver.has_value() == false)
		{
			throwScriptException(this, tr("createObserver(), TestObserver creating error: %1").arg(testObserver.error()));
			return result;
		}

		ScriptTestObserver* observer = new ScriptTestObserver{std::move(*testObserver), m_testLog, this};
		result = jsEngine->newQObject(observer);

		return result;
	}


	QJSValue TestController::signalState(QString appSignalId)
	{
		if (m_inputController == nullptr)
		{
			throwScriptException(this, tr("signalState(), InputController is not set."));
			return {};
		}

		auto state = m_inputController->signalState(appSignalId);
		if (state.has_value() == false)
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

		return jsEngine->toScriptValue(state.value());
	}

	double TestController::signalValue(QString appSignalId)
	{
		if (m_inputController == nullptr)
		{
			throwScriptException(this, tr("signalValue(), InputController is not set."));
			return {};
		}

		auto state = m_inputController->signalState(appSignalId);
		if (state.has_value() == false)
		{
			throwScriptException(this, tr("signalValue(%1), signal not found.").arg(appSignalId));
			return -1;
		}

		if (state->isStateAvailable() == false || state->isValid() == false)
		{
			return std::numeric_limits<double>::quiet_NaN();
		}

		return state->value();
	}

	bool TestController::overrideSignalValue(QString appSignalId, QVariant value)
	{
		if (m_outputController == nullptr)
		{
			throwScriptException(this, tr("overrideSignalValue(), OutputController is not set."));
			return false;
		}

		bool ok = m_outputController->writeSignalValue(appSignalId, value);

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
		if (m_outputController == nullptr)
		{
			throwScriptException(this, tr("waitForSignalOverrides(), OutputController is not set."));
			return false;
		}

		qint64 elapsedMs = 0;
		return m_outputController->waitForAllSignalsWritten(timeoutMs, elapsedMs);
	}

	bool TestController::expectSignalValue(QString appSignalId, qint64 timeoutMs, double value, double tolerance /*= 0*/)
	{
		qint64 elapsedMs = 0;

		// Before expecting any values, wait for all writing processes are finished
		//
		if (m_outputController != nullptr)
		{
			bool allWritten = m_outputController->waitForAllSignalsWritten(timeoutMs, elapsedMs);
			if (allWritten == false)
			{
				return false;
			}
		}

		if (m_inputController == nullptr)
		{
			throwScriptException(this, tr("expectSignalValue(), InputController is not set."));
			return {};
		}

		return m_inputController->expectSignalValue(appSignalId, timeoutMs - elapsedMs, value, tolerance);
	}

	void TestController::overridesReset(qint64 timeoutMs, QStringList excludeAppSignals)
	{
		if (m_outputController == nullptr)
		{
			throwScriptException(this, tr("overridesReset(), OutputController is not set."));
			return;
		}

		for (const QString& appSignalId : m_overridenSignals)
		{
			AppSignalParam asp = signalParam(appSignalId);

			// Exclude specified signals
			//
			if (excludeAppSignals.contains(appSignalId) == true)
			{
				continue;
			}

			bool ok = m_outputController->writeSignalValue(appSignalId, asp.tuningDefaultValueToVariant().toDouble());
			if (ok == false)
			{
				throwScriptException(this, tr("overrideSignalValue(%1, ...), signal write error.").arg(appSignalId));
				return;
			}
		}

		std::erase_if(m_overridenSignals,
					  [&excludeAppSignals](const QString& s)
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
		if (m_inputController == nullptr)
		{
			throwScriptException(this, tr("signalExists(), InputController is not set."));
			return {};
		}

		return m_inputController->signalExists(appSignalId);
	}

	AppSignalParam TestController::signalParam(QString appSignalId)
	{
		if (m_inputController == nullptr)
		{
			throwScriptException(this, tr("signalParam(), InputController is not set."));
			return {};
		}

		auto result = m_inputController->signalParam(appSignalId);
		if (result.has_value() == false)
		{
			throwScriptException(this, tr("signalParam(%1), signal not found.").arg(appSignalId));
			return {};
		}

		return result.value();
	}

	bool TestController::tuningSourceIsActive(QString lmEquipmentId)
	{
		if (m_outputController == nullptr)
		{
			throwScriptException(this, tr("tuningSourceIsActive(), OutputController is not set."));
			return false;
		}

		return m_outputController->tuningSourceIsActive(lmEquipmentId);
	}

	bool TestController::tuningSourceIsInactive(QString lmEquipmentId)
	{
		if (m_outputController == nullptr)
		{
			throwScriptException(this, tr("tuningSourceIsInactive(), OutputController is not set."));
			return false;
		}

		return m_outputController->tuningSourceIsInactive(lmEquipmentId);
	}

	bool TestController::activateTuningSource(QString lmEquipmentId, bool activate)
	{
		if (m_outputController == nullptr)
		{
			throwScriptException(this, tr("activateTuningSource(), OutputController is not set."));
			return false;
		}

		return m_outputController->activateTuningSource(lmEquipmentId, activate);
	}

	QString TestController::projectName() const
	{
		return m_projectName;
	}

	void TestController::setProjectName(const QString& value)
	{
		m_projectName = value;
	}

	int TestController::buildNo() const
	{
		return m_buildNo;
	}

	void TestController::setBuildNo(int value)
	{
		m_buildNo = value;
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
		m_debugMessagesEnabled = value;
	}

} // namespace TestSuite
