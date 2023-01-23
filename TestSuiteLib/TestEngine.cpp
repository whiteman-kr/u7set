#include "TestEngine.h"


TestEngine::TestEngine()
{

}

void TestEngine::start()
{
	m_testResultLog.addMessage("TestEngine started.");

	QTimer::singleShot(1000, this, [this](){
	emit finished(-2);
	});

}

void TestEngine::stop()
{

}

bool TestEngine::isRunning() const
{
	return false;
}

const TestResultLog& TestEngine::testResultLog() const
{
	return m_testResultLog;
}
