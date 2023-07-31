#pragma once

#include "../UtilsLib/ILogFile.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "TestSuiteSettings.h"
#include "TestSuiteConfigController.h"
#include "Control.h"
#include "TestLog.h"

//#include "InputController.h"
//#include "OutputController.h"
//#include "ScriptTestLog.h"
//#include "TestWorker.h"

//#include "TestScriptsStorage.h"



namespace TestSuite
{
	class TestSuite : public QObject
	{
		Q_OBJECT

	public:
		TestSuite(const SoftwareInfo& softwareInfo, const TestSuiteSettings& settings, ILogFile* appLog, ITestLogOutput* testOutput);
		~TestSuite() = default;

	public:
		bool execute(const QStringList& scriptsFiles,		// List of script files for execution, if empty then exec all.
					 const QString& scriptsPath,			// Load scripts from disk, path to dir for *.js files.
					 const TestScriptFilter& testsFilter,	// Tests filter
					 const QString& userName,
					 const QString& password);
		void stop();

		bool isRunning() const;

		TestLog& testLog();

		ControlStatus status() const;

		ReportLib::ReportTemplateStorage reportTemplates() const;	// Returns templates received by taskCfgServiceConnection in Control thread

	signals:
		void testFinished(QString scriptFileName, QString testFunction, bool result);
		void finished(int result);

	private:
		HasLogFile m_appLog;
		TestLog m_testLog;

		SoftwareInfo m_softwareInfo;
		TestSuiteSettings m_settings;

		// Test runtime
		//
		Control m_control;
	};
}

