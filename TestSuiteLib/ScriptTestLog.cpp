#include "ScriptTestLog.h"

ScriptTestLog::ScriptTestLog(TestLog* testLog):
	m_testLog(testLog)
{
	Q_ASSERT(m_testLog);

}

void ScriptTestLog::writeError(const QString& message)
{
	if (m_testLog == nullptr)
	{
		Q_ASSERT(m_testLog);
		return;
	}
	m_testLog->writeError(message);
}

void ScriptTestLog::writeWarning(const QString& message)
{
	if (m_testLog == nullptr)
	{
		Q_ASSERT(m_testLog);
		return;
	}
	m_testLog->writeError(message);
}

void ScriptTestLog::writeMessage(const QString& message)
{
	if (m_testLog == nullptr)
	{
		Q_ASSERT(m_testLog);
		return;
	}
	m_testLog->writeMessage(message);
}

