#pragma once

#include "InputController.h"
#include "OutputController.h"
#include "TestLogController.h"
#include "TestWorker.h"
#include "TestLog.h"
#include "../UtilsLib/ILogFile.h"
#include "TestScriptsStorage.h"
#include "../OnlineLib/SoftwareInfo.h"
#include "TestSuiteConfigController.h"
#include "TestLibrarySettings.h"

enum class TestLibraryState
{
	Idle,
	WaitingForConfiguration,
	Running,
	Error
};

class TestLibrary : public QObject, public HasLogFile
{
	Q_OBJECT
public:
	TestLibrary(const TestLibrarySettings& settings, ILogFile* appLogFile, IOutputLog* outputLog);

	TestSuiteConfigController& configController();
	const TestSuiteConfigController& configController() const;

	TestScriptsStorage& testScriptsStorage();
	const TestScriptsStorage& testScriptsStorage() const;

	const TestLog& testResultLog() const;

	//void setAppSignalTcpClients(std::vector<TcpSignalClient*> tcpClients);
	//void setTuningTcpClients(std::vector<TuningTcpClient*> tcpClients);

	void execute();
	void stop();

	bool isRunning() const;
	TestLibraryState state() const;

signals:
	void testingFinished(int result);

private:
	bool loadTestsFromPath();
	bool loadTestsFromConfiguration();
	void runTests();

private slots:
	void onConfigurationArrived();
	void onTestingFinished(int errorCode);

private:
	InputController* m_inputController = nullptr;
	OutputController* m_outputController = nullptr;

	TestWorkerThread* m_testWorkerThread = nullptr;	// Main test worker thread

	TestLog m_testLog;
	TestLogController m_testLogController;

private:
	TestScriptsStorage m_testScriptsStorage;

	TestSuiteConfigController m_configController;

	TestLibrarySettings m_librarySettings;

	TestLibraryState m_state = TestLibraryState::Idle;


};

