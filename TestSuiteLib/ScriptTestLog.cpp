#include "ScriptTestLog.h"

namespace TestSuite
{
	ScriptTestLog::ScriptTestLog(ITestLog& testLog):
		m_testLog(testLog)
	{
	}

	void ScriptTestLog::writeError(const QString& message, int level, int tag)
	{
		TestLogItemLevel l{TestLogItemLevel::Level0};
		switch (level)
		{
		case 0: l = TestLogItemLevel::Level0; break;
		case 1: l = TestLogItemLevel::Level1; break;
		case 2: l = TestLogItemLevel::Level2; break;
		}
		m_testLog.writeError(message, l, tag);
	}

	void ScriptTestLog::writeWarning(const QString& message, int level, int tag)
	{
		TestLogItemLevel l{TestLogItemLevel::Level0};
		switch (level)
		{
		case 0: l = TestLogItemLevel::Level0; break;
		case 1: l = TestLogItemLevel::Level1; break;
		case 2: l = TestLogItemLevel::Level2; break;
		}
		m_testLog.writeWarning(message, l, tag);
	}

	void ScriptTestLog::writeWarningLevel0(const QString& message, int tag)
	{
		m_testLog.writeWarning(message, TestLogItemLevel::Level0, tag);
	}

	void ScriptTestLog::writeWarningLevel1(const QString& message, int tag)
	{
		m_testLog.writeWarning(message, TestLogItemLevel::Level1, tag);
	}

	void ScriptTestLog::writeWarningLevel2(const QString& message, int tag)
	{
		m_testLog.writeWarning(message, TestLogItemLevel::Level2, tag);
	}

	void ScriptTestLog::writeMessage(const QString& message, int level, int tag)
	{
		TestLogItemLevel l{TestLogItemLevel::Level0};
		switch (level)
		{
		case 0: l = TestLogItemLevel::Level0; break;
		case 1: l = TestLogItemLevel::Level1; break;
		case 2: l = TestLogItemLevel::Level2; break;
		}
		m_testLog.writeMessage(message, l, tag);
	}

	void ScriptTestLog::writeMessageLevel0(const QString& message, int tag)
	{
		m_testLog.writeMessage(message, TestLogItemLevel::Level0, tag);
	}

	void ScriptTestLog::writeMessageLevel1(const QString& message, int tag)
	{
		m_testLog.writeMessage(message, TestLogItemLevel::Level1, tag);
	}

	void ScriptTestLog::writeMessageLevel2(const QString& message, int tag)
	{
		m_testLog.writeMessage(message, TestLogItemLevel::Level2, tag);
	}

	void ScriptTestLog::writeText(const QString& message, int level, int tag)
	{
		TestLogItemLevel l{TestLogItemLevel::Level0};
		switch (level)
		{
		case 0: l = TestLogItemLevel::Level0; break;
		case 1: l = TestLogItemLevel::Level1; break;
		case 2: l = TestLogItemLevel::Level2; break;
		}
		m_testLog.writeText(message, l, tag);
	}
}
