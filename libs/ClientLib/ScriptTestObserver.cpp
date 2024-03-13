#include "./include/ClientLib/ScriptTestObserver.h"

ScriptTestObserver::ScriptTestObserver(std::unique_ptr<ITestObserver> observer, ILogFile* logFile, QObject* parent) :
	QObject{parent},
	m_observer{std::move(observer)},
	m_logFile(logFile)
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
		reportError("ScriptTestObserver::start() failed, check connection to services.", false);
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
		QString errorMessage = tr("ScriptTestObserver::wait() failed.");
		reportError(errorMessage.trimmed(), false);

		auto expectations = m_observer->expectations();
		for (int expectationId : expectations)
		{
			// -1 means expectation has not been met.
			//
			if (m_observer->expectationResult(expectationId) == -1)
			{
				// Expectation has not been met.
				//
				QString expectationStr = m_observer->expectationStr(expectationId);
				reportError(QString("Failed expectation: ") + expectationStr, false);
			}
		}
	}

	return result;
}

void ScriptTestObserver::usePlantTime()
{
	return m_observer->setTimeType(E::TimeType::Plant);
}

void ScriptTestObserver::useLocalTime()
{
	return m_observer->setTimeType(E::TimeType::Local);
}

void ScriptTestObserver::setInitiator(int initialExpectationId)
{
	bool result = m_observer->setInitiator(initialExpectationId);
	if (result == false)
	{
		reportError(tr("ScriptTestObserver::setInitiator() Initiator was not set, wrong initialExpectationId %1.").arg(initialExpectationId), true);
	}

	return;
}

int ScriptTestObserver::addEqualExpectation(QString appSignalId, double expectedValue, double tolerance)
{
	int result = m_observer->addEqualExpectation(appSignalId, expectedValue, tolerance);
	if (result == ITestObserver::InvalidExpectationId)
	{
		reportError(tr("ScriptTestObserver::addEqualExpectation() Expectation was not added, check appSignalId %1.").arg(appSignalId), true);
	}

	return result;
}

int ScriptTestObserver::addGreaterExpectation(QString appSignalId, double threshold)
{
	int result = m_observer->addGreaterExpectation(appSignalId, threshold);
	if (result == ITestObserver::InvalidExpectationId)
	{
		reportError(tr("ScriptTestObserver::addGreaterExpectation() Expectation was not added, check appSignalId %1.").arg(appSignalId), true);
	}

	return result;
}

int ScriptTestObserver::addLessExpectation(QString appSignalId, double threshold)
{
	int result = m_observer->addLessExpectation(appSignalId, threshold);
	if (result == ITestObserver::InvalidExpectationId)
	{
		reportError(tr("ScriptTestObserver::addLessExpectation() Expectation was not added, check appSignalId %1.").arg(appSignalId), true);
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

void ScriptTestObserver::reportError(const QString& message, bool throwException)
{
	qDebug() << message;

	QJSEngine* jsEngine = qjsEngine(this);
	Q_ASSERT(jsEngine);

	if ((jsEngine != nullptr && throwException == true) || m_logFile == nullptr)
	{
		jsEngine->throwError(message);
	}
	else
	{
		assert(m_logFile);
		m_logFile->writeError(message);
	}

	return;
}