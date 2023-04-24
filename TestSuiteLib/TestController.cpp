#include "TestController.h"

namespace TestSuite
{
	TestController::TestController(IInputController& inputController, IOutputController& outputController, QObject* parent) :
		QObject{parent},
		m_inputController{inputController},
		m_outputController{outputController}
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

		return ok;
	}

	bool TestController::waitForAllSignalsWritten(qint64 timeoutMs)
	{
		return m_outputController.waitForAllSignalsWritten(timeoutMs);
	}

	bool TestController::expectSignalValue(QString appSignalId, double value, qint64 timeoutMs)
	{
		return m_inputController.expectSignalValue(appSignalId, value, timeoutMs);
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

}
