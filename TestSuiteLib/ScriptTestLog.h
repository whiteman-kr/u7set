#pragma once

#include "TestLog.h"

namespace TestSuite
{
	// Wrapper for TestLog for JavaScript.
	//
	class ScriptTestLog : public QObject
	{
		Q_OBJECT

	public:
		ScriptTestLog(ITestLog& testLog);

	public slots:
		void writeError(const QString& message, int level = 0, int tag = -1);

		void writeWarning(const QString& message, int level = 0, int tag = -1);
		void writeWarningLevel0(const QString& message, int tag = -1);
		void writeWarningLevel1(const QString& message, int tag = -1);
		void writeWarningLevel2(const QString& message, int tag = -1);

		void writeMessage(const QString& message, int level = 0, int tag = -1);
		void writeMessageLevel0(const QString& message, int tag = -1);
		void writeMessageLevel1(const QString& message, int tag = -1);
		void writeMessageLevel2(const QString& message, int tag = -1);

		void writeText(const QString& message, int level = 0, int tag = -1);

	private:
		ITestLog& m_testLog;
	};
}
