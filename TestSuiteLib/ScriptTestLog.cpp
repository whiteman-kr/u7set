#include "ScriptTestLog.h"

namespace TestSuite
{
	ScriptTestLog::ScriptTestLog(ITestLog& testLog):
		m_testLog(testLog)
	{
	}

	void ScriptTestLog::writeError(const QString& message, int level, int tag)
	{
		m_testLog.writeError(message, level, tag);
	}

	void ScriptTestLog::writeWarning(const QString& message, int level, int tag)
	{
		m_testLog.writeWarning(message, level, tag);
	}

	void ScriptTestLog::writeWarningLevel0(const QString& message, int tag)
	{
		m_testLog.writeWarning(message, 0, tag);
	}

	void ScriptTestLog::writeWarningLevel1(const QString& message, int tag)
	{
		m_testLog.writeWarning(message, 1, tag);
	}

	void ScriptTestLog::writeWarningLevel2(const QString& message, int tag)
	{
		m_testLog.writeWarning(message, 2, tag);
	}

	void ScriptTestLog::writeMessage(const QString& message, int level, int tag)
	{
		m_testLog.writeMessage(message, level, tag);
	}

	void ScriptTestLog::writeMessageLevel0(const QString& message, int tag)
	{
		m_testLog.writeMessage(message, 0, tag);
	}

	void ScriptTestLog::writeMessageLevel1(const QString& message, int tag)
	{
		m_testLog.writeMessage(message, 1, tag);
	}

	void ScriptTestLog::writeMessageLevel2(const QString& message, int tag)
	{
		m_testLog.writeMessage(message, 2, tag);
	}

	void ScriptTestLog::writeText(const QString& message, int level, int tag)
	{
		m_testLog.writeText(message, level, tag);
	}
}
