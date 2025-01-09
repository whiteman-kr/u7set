#include "ScriptTestLog.h"

namespace TestSuite
{
	ScriptTestLog::ScriptTestLog(ILogFile& testLog):
		m_testLog(testLog)
	{
	}

	void ScriptTestLog::writeError(QString message, QString tag)
	{
		m_testLog.writeError(message, tag);
	}

	void ScriptTestLog::writeWarning(QString message, QString tag)
	{
		m_testLog.writeWarning(message, tag);
	}

	void ScriptTestLog::writeMessage(QString message, QString tag)
	{
		m_testLog.writeMessage(message, tag);
	}

	void ScriptTestLog::writeText(QString message, QString tag)
	{
		m_testLog.writeText(message, tag);
	}
}
