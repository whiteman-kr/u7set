#include "ScriptTestObserver.h"

ScriptTestObserver::ScriptTestObserver(std::unique_ptr<ITestObserver> observer, QObject* parent) :
	QObject{parent},
	m_observer{std::move(observer)}
{
	if (m_observer == nullptr)
	{
		Q_ASSERT(m_observer);
		throw std::runtime_error{"ScriptTestObserver::ScriptTestObserver observer is nullptr."};
	}
}

bool ScriptTestObserver::start()
{
	bool result = m_observer->start();
	if (result == false)
	{
		reportError("ScriptTestObserver::start() failed, check connection to services.");
	}

	return result;
}

void ScriptTestObserver::stop()
{
	return m_observer->stop();
}

void ScriptTestObserver::clear()
{
	return m_observer->clear();
}

bool ScriptTestObserver::wait(int timeoutMs)
{
	bool result = m_observer->wait(timeoutMs);
	if (result == false)
	{
		reportError("ScriptTestObserver::wait() filed.");
	}

	return result;
}

void ScriptTestObserver::usePlantTime()
{
	return m_observer->setTimeType(E::TimeType::Plant);
}

void ScriptTestObserver::useSystemTime()
{
	return m_observer->setTimeType(E::TimeType::System);
}

bool ScriptTestObserver::setInitiator(int initialExpectationId)
{
	bool result = m_observer->setInitiator(initialExpectationId);
	if (result == false)
	{
		reportError(tr("ScriptTestObserver::setInitiator() Initiator was not set, wrong initialExpectationId %1.").arg(initialExpectationId));
	}

	return result;
}

int ScriptTestObserver::addEqualExpectation(QString appSignalId, double expectedValue, double tolerance)
{
	int result = m_observer->addEqualExpectation(appSignalId, expectedValue, tolerance);
	if (result == ITestObserver::InvalidExpectationId)
	{
		reportError(tr("ScriptTestObserver::addEqualExpectation() Expectation was not added, check appSignalId %1.").arg(appSignalId));
	}

	return result;
}

int ScriptTestObserver::addGreaterExpectation(QString appSignalId, double threshold)
{
	int result = m_observer->addGreaterExpectation(appSignalId, threshold);
	if (result == ITestObserver::InvalidExpectationId)
	{
		reportError(tr("ScriptTestObserver::addGreaterExpectation() Expectation was not added, check appSignalId %1.").arg(appSignalId));
	}

	return result;
}

int ScriptTestObserver::addLessExpectation(QString appSignalId, double threshold)
{
	int result = m_observer->addLessExpectation(appSignalId, threshold);
	if (result == ITestObserver::InvalidExpectationId)
	{
		reportError(tr("ScriptTestObserver::addLessExpectation() Expectation was not added, check appSignalId %1.").arg(appSignalId));
	}

	return result;
}

int ScriptTestObserver::elapsedMs(QString appSignalId) const
{
	return m_observer->elapsedMs(appSignalId);
}

int ScriptTestObserver::expectationResult(int expectationId) const
{
	return m_observer->expectationResult(expectationId);
}

void ScriptTestObserver::reportError(const QString& message)
{
	qDebug() << message;

	QJSEngine* jsEngine = qjsEngine(this);
	Q_ASSERT(jsEngine);

	if (jsEngine != nullptr)
	{
		jsEngine->throwError(message);
	}

	return;
}