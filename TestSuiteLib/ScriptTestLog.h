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
		void writeError(const QString& message);
		void writeWarning(const QString& message);
		void writeMessage(const QString& message);

	private:
		ITestLog& m_testLog;
	};
}
