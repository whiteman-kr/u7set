#pragma once

#include "../UtilsLib/ILogFile.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "TestSuiteSettings.h"
#include "RunControl.h"
#include "TestControl.h"
#include "TestLog.h"

namespace TestSuite
{
	class TestSuite : public QObject
	{
		Q_OBJECT

	public:
		TestSuite(const SoftwareInfo& softwareInfo, const TestSuiteSettings& settings, ILogFile* appLog, ITestLogOutput* testOutput);
		virtual ~TestSuite();

	public:
		bool executeRunControl(const ControlParams& controlParams);
		bool hasRunControl();
		void resetRunControl();
		void stopRunControl();

		bool execute(const ControlParams& controlParams);
		void stop();
		bool isRunning() const;

		void updateSettings(const TestSuiteSettings& settings, const ControlParams& controlParams);

		TestLog& testLog();
		ControlStatus testStatus() const;
		ControlStatus runStatus() const;

		bool scriptPermission(const QString& fileName) const;
		bool globalPermission() const;

	signals:
		void testStarted(QString scriptFileName, QString testFunction);
		void testFinished(QString scriptFileName, QString testFunction, bool result);
		void finished(int result);
		void scriptPermissionChanged(QString scriptFileName, bool result);
		void globalPermissionChanged(bool result);
		void noPermissionsExist();


	private:
		HasLogFile m_appLog;
		TestLog m_testLog;

		SoftwareInfo m_softwareInfo;
		TestSuiteSettings m_settings;

		// Test runtime
		//
		TestControl m_testControl;

		// Run runtime
		//
		RunControl m_runControl;
	};
}

