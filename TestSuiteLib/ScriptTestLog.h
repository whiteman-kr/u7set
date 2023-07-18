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
		void writeError(QString message, QString tag = QString());
		void writeWarning(QString message, QString tag = QString());
		void writeMessage(QString message, QString tag = QString());
		void writeText(QString message, QString tag = QString());

	private:
		ITestLog& m_testLog;
	};
}
