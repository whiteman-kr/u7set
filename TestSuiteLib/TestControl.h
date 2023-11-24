#pragma once

#include <QObject>
#include "Control.h"

namespace TestSuite
{
	class TestControlThread : public ControlThread
	{
		Q_OBJECT
	public:
		TestControlThread(ILogFile* appLog, TestLog* testLog);

	signals:
		void testStarted(QString scriptFileName, QString testFunction);
		void testFinished(QString scriptFileName, QString testFunction, bool result);

	private:
		virtual void run() override;
		
		void taskCheckLogin();
		void taskRunTests();
		void taskCreateReports();
	};

	class TestControl : public Control
	{
		Q_OBJECT
	public:
		explicit TestControl(ILogFile* appLog, TestLog* testLog);

	signals:
		void testStarted(QString scriptFileName, QString testFunction);
		void testFinished(QString scriptFileName, QString testFunction, bool result);
		void finished(int result);
	};
}
