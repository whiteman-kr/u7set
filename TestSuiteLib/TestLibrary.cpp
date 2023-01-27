#include "TestLibrary.h"

TestLibrary::TestLibrary(const SoftwareInfo& softwareInfo, HostAddressPort address1, HostAddressPort address2, ILogFile *appLogFile):
	HasLogFile(appLogFile, "TestLibrary"),
	m_configController(softwareInfo, address1, address2, appLogFile)
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


const TestScriptsStorage& TestLibrary::testScripts() const
{
	return m_testScriptsStorage;
}

void TestLibrary::setTestScripts(TestScriptsStorage& testScriptsStorage)
{
	m_testScriptsStorage.move(testScriptsStorage);
}

void TestLibrary::start()
{
	m_testLog.addMessage("TestLibrary started.");

	QTimer::singleShot(1000, this, [this](){
	emit finished(-2);
	});

}

void TestLibrary::stop()
{

}

bool TestLibrary::isRunning() const
{
	return false;
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

	setTestScripts(m_configController.testScriptsStorage());

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

void TestLibrary::emitMessage(const QString& msg)
{
	this->writeMessage(msg);
	emit logMessage(msg);
}

void TestLibrary::emitError(const QString& errorMsg)
{
	this->writeError(errorMsg);
	emit logError(errorMsg);
}
