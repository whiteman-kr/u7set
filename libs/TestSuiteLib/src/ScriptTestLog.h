#pragma once

#include <TestSuiteLib/TestLog.h>

namespace TestSuite
{
	// Wrapper for TestLog for JavaScript.
	//
	class ScriptTestLog : public QObject
	{
		Q_OBJECT

	public:
		ScriptTestLog(ILogFile& testLog);

	public slots:
		void writeError(QString message, QString tag = QString());
		void writeWarning(QString message, QString tag = QString());
		void writeMessage(QString message, QString tag = QString());
		void writeText(QString message, QString tag = QString());

	private:
		ILogFile& m_testLog;
	};
}
