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
		TestSuite(const SoftwareInfo& softwareInfo, const TestSuiteSettings& settings, ILogFile* appLog, ITestLog* testLog);
		~TestSuite() = default;

	public:
//		TestSuiteConfigController& configController();
//		const TestSuiteConfigController& configController() const;

//		TestScriptsStorage& testScriptsStorage();
//		const TestScriptsStorage& testScriptsStorage() const;

//		const TestLog& testResultLog() const;

		//void setAppSignalTcpClients(std::vector<TcpSignalClient*> tcpClients);
		//void setTuningTcpClients(std::vector<TuningTcpClient*> tcpClients);

		bool execute(const QStringList& executionTests,		// List of tests for execution, if empty then exec all.
					 const QString& scriptsPath);			// Load scripts from disk, path to dir for *.js files.);
		void stop();

		bool isRunning() const;

	signals:
		void finished(int result);

//	private:
//		bool loadTestsFromPath();
//		bool loadTestsFromConfiguration();
//		void runTests();
//		void stopTests();

//	private slots:
//		void onConfigurationArrived();
//		void onTestingFinished(int errorCode);

	private:
		HasLogFile m_appLog;
		TestLog m_testLog;

		SoftwareInfo m_softwareInfo;
		TestSuiteSettings m_settings;

		// Test runtime
		//
		Control m_control;

		//InputController* m_inputController = nullptr;
		//OutputController* m_outputController = nullptr;

		//TestController m_testController;
		//ScriptTestLog m_scriptTestLog;

//		TestWorkerThread* m_testWorkerThread = nullptr;	// Main test worker thread

//		TestScriptsStorage m_testScriptsStorage;

//		TestLibraryState m_state = TestLibraryState::Idle;
	};
}

