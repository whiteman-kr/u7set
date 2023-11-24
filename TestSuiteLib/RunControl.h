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

	public:
		bool scriptPermission(const QString& fileName) const;
		bool globalPermission() const;

	private:
		virtual void run() override;
		
		void taskPrepare();
		void taskQueryPermission();

		void taskCleanup();

	signals:
		void scriptPermissionChanged(QString scriptFileName, bool result);
		void globalPermissionChanged(bool result);
		void noPermissionsExist();

	private:
		std::vector<std::unique_ptr<TestController>> m_testControllers;
		std::vector<std::unique_ptr<ScriptRunner>> m_runners;

		mutable QMutex m_permissionsMutex;	// to protect m_scriptPermissions and m_globalPermission
		std::map<QString, bool> m_scriptPermissions;	// Key is script filename, value is running permission
		std::optional<bool> m_globalPermission;			// State of global permission
		bool m_noPermissionsExist{false};				// Set to true if no permission functions were found
		
		std::atomic<bool> m_resetFlag{false};			// This flag means to clear permissions map and to restart quering process with new scripts
	};

	class RunControl : public Control
	{
		Q_OBJECT
	public:
		explicit RunControl(ILogFile* appLog, TestLog* testLog);
		void reset();	// Clear permissions map and to restart quering process with new scripts

		bool scriptPermission(const QString& fileName) const;
		bool globalPermission() const;

	signals:
		void scriptPermissionChanged(QString scriptFileName, bool result);
		void globalPermissionChanged(bool result);
		void noPermissionsExist();
	};
}
