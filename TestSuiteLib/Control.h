#pragma once

#include <QObject>
#include <chrono>
#include "TestLog.h"
#include "TestSuiteSettings.h"
#include "TestSuiteConfigController.h"
#include "TestScriptsStorage.h"
#include "IInputController.h"
#include "IOutputController.h"
#include "ControlState.h"
#include "../ClientLib/AppSignalManager.h"

namespace TestSuite
{
	using namespace std::literals::chrono_literals;

	class ControlThread : public QThread
	{
		Q_OBJECT

	public:
		ControlThread(ILogFile* appLog, ITestLog* testLog);

	public:
		void setTestParams(const SoftwareInfo& softwareInfo,
						   const TestSuiteSettings& settings,
						   const QStringList& scriptsFiles,		// List of script files for execution, if empty then exec all.
						   const QString& scriptsPath,			// Load scripts from disk, path to dir for *.js files.
						   const TestScriptFilter& testsFilter);			// Tests filter

		int result() const;

		ControlStatus status() const;

		ReportLib::ReportTemplateStorage reportTemplates() const;	// Returns templates received by taskCfgServiceConnection

	signals:
		void testFinished(QString scriptFileName, QString testFunction, bool result);

	protected:
		virtual void run() override;

	private:
		void cleanUp();
		void checkAndInterruptTestExecution();

		void taskCfgServiceConnection();
		void taskInitInputController();
		void taskInitOutputController();

		void taskRunTests();
		void taskCreateReports();

	private:
		HasLogFile m_appLog;
		ITestLog* m_testLog = nullptr;

		SoftwareInfo m_softwareInfo;
		TestSuiteSettings m_settings;

		QStringList m_scriptsToRun;		// List of script files for execution, if empty then exec all.
		QString m_scriptsPath;			// Load scripts from disk, path to dir for *.js files.
		TestScriptFilter m_testsFilter;		// Tests filter

		// --
		//
		mutable QMutex m_statusMutex;
		ControlStatus m_status;

		std::atomic<int> m_result{0};

		ConfigSettings m_configuration;
		std::vector<TestScript> m_scripts;

		mutable QMutex m_reportTemplatesMutex;
		ReportLib::ReportTemplateStorage m_reportTemplates;

		ClientLib::AppSignalManager m_signals;
		std::unique_ptr<IInputController> m_inputController;
		std::unique_ptr<IOutputController> m_outputController;
	};


	class Control : public QObject
	{
		Q_OBJECT

	public:
		explicit Control(ILogFile* appLog, ITestLog* testLog);

	public:
		bool execute(const SoftwareInfo& softwareInfo,
					 const TestSuiteSettings& settings,
					 const QStringList& scriptsFiles,
					 const QString& scriptsPath,
					 const TestScriptFilter& testsFilter);
		bool stop();
		bool isRunning() const;

		ControlStatus status() const;

		ReportLib::ReportTemplateStorage reportTemplates() const;	// Returns templates received by taskCfgServiceConnection

	signals:
		void testFinished(QString scriptFileName, QString testFunction, bool result);
		void finished(int result);

	private:
		ILogFile* m_appLog = nullptr;
		ITestLog* m_testLog = nullptr;

		ControlThread m_controlThread;
	};
}
