#include "TestController.h"

namespace TestSuite
{
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

}
