#pragma once

#include <QObject>
#include "TestLog.h"
#include "TestSuiteSettings.h"
#include "TestSuiteConfigController.h"
#include "TestScriptsStorage.h"
#include "IInputController.h"
#include "IOutputController.h"
#include "ControlState.h"
#include "ScriptRunner.h"
#include <ClientLib/AppSignalManager.h>

namespace TestSuite
{
	struct ControlParams
	{

		ControlParams() = default;

		ControlParams(const QStringList& scriptsFiles,
					  const QString& scriptsPath,
					  const QString& reportsPath,
					  const TestScriptSelection& testsFilter,
					  const QString& userName,
					  const QString& password)
		{
			this->scriptsFiles = scriptsFiles;
			this->scriptsPath = scriptsPath;
			this->reportsPath = reportsPath;
			this->testsFilter = testsFilter;
			this->userName = userName;
			this->password = password;
		}
			
		explicit ControlParams(const QString& scriptsPath)
		{
			this->scriptsPath = scriptsPath;
		}

		QStringList scriptsFiles;        // List of script files for execution, if empty then exec all.
		QString scriptsPath;             // Load scripts from disk, path to dir for *.js files.
		QString reportsPath;             // Save reports to disk if path is not empty
		TestScriptSelection testsFilter; // Tests filter
		QString userName;
		QString password;
	};

	class ControlThread : public QThread
	{
	public:
		ControlThread(ILogFile* appLog, TestLog* testLog, const QString& runContext);
		virtual ~ControlThread();

	public:
		void setTestParams(const SoftwareInfo& softwareInfo,
						   const TestSuiteSettings& settings,
						   const ControlParams& controlParams);

		int result() const;

		ControlStatus status() const;

	protected:
		virtual void run() = 0;

	protected:
		void checkAndInterruptTestExecution();

		void cleanUp();
		void taskCfgServiceConnection();
		void taskInitInputController();
		void taskInitOutputController();

	protected:
		HasLogFile m_appLog;
		TestLog* m_testLog = nullptr;

		// --
		//
		SoftwareInfo m_softwareInfo;
		TestSuiteSettings m_settings;
		ConfigSettings m_configuration;
		ConfigData m_configData;
		ControlParams m_controlParams;

		// --
		//
		ClientLib::AppSignalManager m_signals;
		std::unique_ptr<IInputController> m_inputController;
		std::unique_ptr<IOutputController> m_outputController;

		// --
		//
		mutable QMutex m_statusMutex;
		ControlStatus m_status;

		std::atomic<int> m_result{0};
	};

	class Control : public QObject
	{
	public:
		Control(ILogFile* appLog, TestLog* testLog, ControlThread* controlThread);
		virtual ~Control();

	public:
		bool execute(const SoftwareInfo& softwareInfo,
					 const TestSuiteSettings& settings,
					 const ControlParams& controlParams);
		bool stop();
		bool isRunning() const;

		ControlStatus status() const;

	protected:
		ILogFile* m_appLog = nullptr;
		TestLog* m_testLog = nullptr;

		std::unique_ptr<ControlThread> m_controlThread{nullptr};

	private:
		std::atomic<bool> m_stopRequested{false};
	};
}
