#pragma once

#include <QObject>
#include "TestLog.h"
#include "TestSuiteSettings.h"
#include "TestSuiteConfigController.h"
#include "TestScriptsStorage.h"
#include "IInputController.h"
#include "IOutputController.h"
#include "../ClientLib/AppSignalManager.h"

namespace TestSuite
{
	class ControlThread : public QThread
	{
		Q_OBJECT

	public:
		ControlThread(ILogFile* appLog, ITestLog* testLog);

	public:
		void setTestParams(const SoftwareInfo& softwareInfo,
						   const TestSuiteSettings& settings,
						   const QStringList& executionTests,	// List of tests for execution, if empty then exec all.
						   const QString& scriptsPath);			// Load scripts from disk, path to dir for *.js files.

		int result() const;

	protected:
		virtual void run() override;

	private:
		void cleanUp();
		void checkAndInterruptTestExecution();

		void taskCfgServiceConnection();
		void taskInitInputController();
		void taskInitOutputController();

		void taskRunTests();

	private:
		HasLogFile m_appLog;
		ITestLog* m_testLog = nullptr;

		SoftwareInfo m_softwareInfo;
		TestSuiteSettings m_settings;

		QStringList m_executionTests;	// List of tests for execution, if empty then exec all.
		QString m_scriptsPath;			// Load scripts from disk, path to dir for *.js files.

		// --
		//
		std::atomic<int> m_result{0};

		ConfigSettings m_configuration;
		std::vector<TestScript> m_scripts;

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
					 const QStringList& executionTests,
					 const QString& scriptsPath);
		bool stop();
		bool isRunning() const;

	signals:
		void finished(int result);

	private:
		ILogFile* m_appLog = nullptr;
		ITestLog* m_testLog = nullptr;

		ControlThread m_controlThread;
	};
}
