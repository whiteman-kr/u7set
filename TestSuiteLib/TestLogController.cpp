#include "TestLogController.h"

TestLogController::TestLogController(TestLog* testLog):
	m_testLog(testLog)
{

}


void TestLogController::addMessage(const QString& message)
{
	m_testLog->addMessage(message);
}

void TestLogController::writeError(const QString& message)
{
	m_testLog->addError(message);
}
