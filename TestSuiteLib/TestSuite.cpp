#include "TestSuite.h"

namespace TestSuite
{

	TestSuite::TestSuite(const SoftwareInfo& softwareInfo, const TestSuiteSettings& settings, ILogFile* appLog, ITestLogOutput* testOutput):
		m_appLog{appLog, "TestLibrary"},
		m_testLog{testOutput},
		m_softwareInfo{softwareInfo},
		m_settings{settings},
		m_control{appLog, &m_testLog}
		//m_configController(softwareInfo, settings.configuratorAddress1(), settings.configuratorAddress2(), appLog)
		//m_scriptTestLog(&m_testLog)
	{
		//connect(&m_configController, &TestSuiteConfigController::configurationArrived, this, &TestSuite::onConfigurationArrived);

		//m_configController.start();

		//	if (m_librarySettings.loadScriptsFromPath() == true)
		//	{
		//		loadTestsFromPath();
		//	}

		connect(&m_control, &Control::finished, this, &TestSuite::finished);
		return;
	}

//	TestSuiteConfigController& TestSuite::configController()
//	{
//		return m_configController;
//	}

//	const TestSuiteConfigController& TestSuite::configController() const
//	{
//		return m_configController;
//	}

	//TestScriptsStorage& TestLibrary::testScriptsStorage()
	//{
	//	return m_testScriptsStorage;
	//}

	//const TestScriptsStorage& TestLibrary::testScriptsStorage() const
	//{
	//	return m_testScriptsStorage;
	//}

	//const TestLog& TestLibrary::testResultLog() const
	//{
	//	return m_testLog;
	//}

	bool TestSuite::execute(const QStringList& scriptsFiles,		// List of script files for execution, if empty then exec all.
							const QString& scriptsPath,				// Load scripts from disk, path to dir for *.js files.
							const QString& testsFilter)				// Tests filter
	{
		m_testLog.clear();

		return m_control.execute(m_softwareInfo, m_settings, scriptsFiles, scriptsPath, testsFilter);
	}

	void TestSuite::stop()
	{
		m_control.stop();
		return;
	}

	bool TestSuite::isRunning() const
	{
		return m_control.isRunning();
	}

	//void TestLibrary::runTests()
	//{
	//	if (state() == TestLibraryState::Running)
	//	{
	//		Q_ASSERT(false);
	//		return;
	//	}

	//	if (m_testWorkerThread != nullptr)
	//	{
	//		Q_ASSERT(m_testWorkerThread == nullptr);
	//		return;
	//	}

	//	setState(TestLibraryState::Running);

	//	m_testLog.writeMessage("TestLibrary::execute");

	//	m_testWorkerThread = new TestWorkerThread(&m_testController, &m_scriptTestLog, this);
	//	connect(m_testWorkerThread, &TestWorkerThread::finished, this, &TestLibrary::onTestingFinished);
	//	m_testWorkerThread->worker().setScripts(m_testScriptsStorage.scripts());	// Set all scripts to the worker
	//	m_testWorkerThread->run();
	//}

//	void TestSuite::onConfigurationArrived()
//	{
		//	if (state() == TestLibraryState::Running)
		//	{
		//		stopTests();
		//	}

//		if (m_librarySettings.loadScriptsFromPath() == false)
//		{
//			bool ok = loadTestsFromConfiguration();
//			if (ok == false)
//			{
//				setState(TestLibraryState::Error);
//			}
//		}

//		if (state() == TestLibraryState::WaitingForConfiguration)
//		{
//			execute();
//		}

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

//		return;
//	}

	//void TestLibrary::onTestingFinished(int errorCode)
	//{
	//	if (m_testWorkerThread == nullptr)
	//	{
	//		Q_ASSERT(m_testWorkerThread);
	//		return;
	//	}

	//	m_testLog.writeMessage("TestLibrary::finished");
	//	m_testWorkerThread->deleteLater();
	//	m_testWorkerThread = nullptr;

	//	setState(TestLibraryState::Idle);

	//	emit testingFinished(errorCode);
	//}

}
