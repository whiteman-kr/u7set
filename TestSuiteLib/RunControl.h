#pragma once

#include <QObject>
#include <optional>
#include "Control.h"

namespace TestSuite
{
	class RunControlThread : public ControlThread
	{
		Q_OBJECT
	public:
		RunControlThread(ILogFile* appLog, TestLog* testLog);

		void reset();

	private:
		virtual void run() override;
		
		void taskPrepare();
		void taskQueryPermission();

		void taskCleanup();

	signals:
		void scriptPermissionChanged(QString scriptFileName, bool result);
		void globalPermissionChanged(bool result);

	private:
		std::vector<std::unique_ptr<TestController>> m_testControllers;
		std::vector<std::unique_ptr<ScriptRunner>> m_runners;

		std::map<QString, bool> m_scriptPermissions;	// Key is script filename, value is running permission
		std::optional<bool> m_globalPermission;			// State of global permission
		
		std::atomic<bool> m_resetFlag{false};			// This flag means to clear permissions map and to restart quering process with new scripts
	};

	class RunControl : public Control
	{
		Q_OBJECT
	public:
		explicit RunControl(ILogFile* appLog, TestLog* testLog);
		void reset();	// Clear permissions map and to restart quering process with new scripts

	signals:
		void scriptPermissionChanged(QString scriptFileName, bool result);
		void globalPermissionChanged(bool result);
	};
}
