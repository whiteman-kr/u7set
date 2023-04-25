#include "ScriptTestLog.h"

namespace TestSuite
{
	ScriptTestLog::ScriptTestLog(ITestLog& testLog):
		m_testLog(testLog)
	{
	}

	void ScriptTestLog::writeError(const QString& message)
	{
		m_testLog.writeError(message);
	}

	void ScriptTestLog::writeWarning(const QString& message)
	{
		m_testLog.writeWarning(message);
	}

	void ScriptTestLog::writeMessage(const QString& message)
	{
		m_testLog.writeMessage(message);
	}
}
