#pragma once

#include "Control.h"


namespace TestSuite
{
	class TestControlThread : public ControlThread
	{
		Q_OBJECT

	public:
		TestControlThread(ILogFile* appLog, ::TestSuite::TestLog* testLog);

	signals:
		void testStarted(QString scriptFileName, QString testFunction);
		void testFinished(QString scriptFileName, QString testFunction, bool result);

	protected:
		virtual void run() override;

		virtual void taskCheckLogin();
		virtual void taskRunTests();
		virtual void taskCreateReports();
	};


	class TestControl : public Control
	{
		Q_OBJECT

	public:
		explicit TestControl(ILogFile* appLog, ::TestSuite::TestLog* testLog);
		explicit TestControl(ILogFile* appLog, ::TestSuite::TestLog* testLog, ControlThread* controlThread);

	signals:
		void testStarted(QString scriptFileName, QString testFunction);
		void testFinished(QString scriptFileName, QString testFunction, bool result);
		void finished(int result);
	};
} // namespace TestSuite
