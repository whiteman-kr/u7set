#include "TestLibrary.h"

TestLibrary::TestLibrary(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile *appLogFile):
	HasLogFile(appLogFile, "TestLibrary"),
	m_appLogFile(appLogFile),
	m_configController(softwareInfo, address1, address2, appLogFile),
	m_testLogController(&m_testLog)
{
	connect(&m_configController, &TestSuiteConfigController::configurationArrived, this, &TestLibrary::slot_configurationArrived);

	m_configController.start();

	emitMessage("Waiting for connection with Configuration Service...");

}

TestSuiteConfigController& TestLibrary::configController()
{
	return m_configController;
}

const TestSuiteConfigController& TestLibrary::configController() const
{
	return m_configController;
}

TestScriptsStorage& TestLibrary::testScriptsStorage()
{
	return m_testScriptsStorage;
}

const TestScriptsStorage& TestLibrary::testScriptsStorage() const
{
	return m_testScriptsStorage;
}

void TestLibrary::execute()
{
	if (isRunning() == true)
	{
		Q_ASSERT(false);
		return;
	}

	if (m_testWorkerThread != nullptr)
	{
		Q_ASSERT(m_testWorkerThread == nullptr);
		return;
	}

	m_testLog.addMessage("TestLibrary::execute");

	TestWorkerContext context(m_outputController, m_inputController, &m_testLogController);
	context.scripts = m_testScriptsStorage.scripts();	// Set all scripts to the context

	m_testWorkerThread = new TestWorkerThread(context, this);
	connect(m_testWorkerThread, &TestWorkerThread::finished, this, &TestLibrary::slot_finished);
	m_testWorkerThread->run();
}

void TestLibrary::stop()
{
	if (isRunning() == false)
	{
		return;
	}

	if (m_testWorkerThread == nullptr)
	{
		Q_ASSERT(m_testWorkerThread == nullptr);
		return;
	}

	m_testWorkerThread->stop();
}

bool TestLibrary::isRunning() const
{
	if (m_testWorkerThread == nullptr)
	{
		return false;
	}

	return m_testWorkerThread->isRunning();
}

const TestLog& TestLibrary::testResultLog() const
{
	return m_testLog;
}

void TestLibrary::slot_configurationArrived(ConfigSettings configuration)
{
	if (isRunning() == true)
	{
		stop();
	}

	if (m_testScriptsStorage.isLoadedFromFiles() == false)
	{
		m_testScriptsStorage.move(m_configController.testScriptsStorage().scripts());
	}

	emit readyForTesting();

	// Log out from tuning
	//
	/*if (m_tuningUserManager.isLoggedIn() == true)
	{
		m_tuningUserManager.logout();
	}

	// Refresh TuningUserManager configuration
	//
	m_tuningUserManager.setConfiguration(configuration.tuningLogin,
										 configuration.tuningUserAccounts,
										 false,
										 configuration.tuningSessionTimeout);

	showTuningLoginControls();

	m_pTuningLogAction->setVisible(configuration.tuningEnabled == true);

	// Close TuningTcpClients
	//
	stopTuningTcpClients();

	// Create TuningTcpClients if tuning is enabled
	//
	if (configuration.tuningEnabled == true)
	{
		runTuningTcpClients();
	}

	m_tuningController->setTcpClients({m_tuningTcpClients.begin(),m_tuningTcpClients.end()});

	if (m_dialogDataSources != nullptr)
	{
		m_dialogDataSources->setTuningTcpClients(configuration.tuningEnabled, {m_tuningTcpClients.begin(),m_tuningTcpClients.end()}, false);
	}

	m_statusBarTuningConnection->setVisible(configuration.tuningEnabled == true);

	m_logoImage = configuration.logoImage;

	showLogo();*/

	//fillTestsTree();

	return;
}

void TestLibrary::slot_finished(int errorCode)
{
	if (m_testWorkerThread == nullptr)
	{
		Q_ASSERT(m_testWorkerThread);
		return;
	}

	m_testLog.addMessage("TestLibrary::finished");
	m_testWorkerThread->deleteLater();
	m_testWorkerThread = nullptr;

	emit finished(errorCode);
}

void TestLibrary::emitMessage(const QString& msg)
{
	writeMessage(msg);
	emit logMessage(msg);
}

void TestLibrary::emitError(const QString& errorMsg)
{
	writeError(errorMsg);
	emit logError(errorMsg);
}
