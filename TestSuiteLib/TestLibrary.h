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

class TestEngineThread
{

};

class TestLibrary : public QObject, public HasLogFile
{
	Q_OBJECT
public:
	TestLibrary(const SoftwareInfo &softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile* appLogFile);

	TestSuiteConfigController& configController();
	const TestSuiteConfigController& configController() const;

	TestScriptsStorage& testScriptsStorage();
	const TestScriptsStorage& testScriptsStorage() const;

	//void setAppSignalTcpClients(std::vector<TcpSignalClient*> tcpClients);
	//void setTuningTcpClients(std::vector<TuningTcpClient*> tcpClients);

	void execute();
	void stop();

	bool isRunning() const;

signals:
	void logMessage(const QString& msg);
	void logError(const QString& errMsg);

	void readyForTesting();
	void finished(int result);

public:
	const TestLog& testResultLog() const;

private slots:
	void slot_configurationArrived(ConfigSettings configuration);
	void slot_finished(int errorCode);

private:
	void emitMessage(const QString& msg);
	void emitError(const QString& errorMsg);

private:
	InputController* m_inputController = nullptr;
	OutputController* m_outputController = nullptr;

	TestWorkerThread* m_testWorkerThread = nullptr;	// Main test worker thread

	TestLog m_testLog;
	TestLogController m_testLogController;

	ILogFile* m_appLogFile = nullptr;

private:
	TestScriptsStorage m_testScriptsStorage;

	TestSuiteConfigController m_configController;


};

