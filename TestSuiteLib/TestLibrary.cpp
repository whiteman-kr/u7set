#include "TestLibrary.h"

TestLibrary::TestLibrary(const TestLibrarySettings& settings, ILogFile* appLogFile, IOutputLog* outputLog):
	HasLogFile(appLogFile, "TestLibrary"),
	m_librarySettings(settings),
	m_configController(settings.instanceStrId(), settings.configuratorAddress1(), settings.configuratorAddress2(), appLogFile),
	m_testLog(outputLog),
	m_scriptTestLog(&m_testLog)
{
	connect(&m_configController, &TestSuiteConfigController::configurationArrived, this, &TestLibrary::onConfigurationArrived);

	m_configController.start();

	if (m_librarySettings.loadScriptsFromPath() == true)
	{
		loadTestsFromPath();
	}
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

const TestLog& TestLibrary::testResultLog() const
{
	return m_testLog;
}

void TestLibrary::execute()
{
	if (state() == TestLibraryState::Running)
	{
		return;
	}

	if (configController().configuration().isValid() == false)
	{
		setState(TestLibraryState::WaitingForConfiguration);
	}
	else
	{
		runTests();
	}

	return;
}
void TestLibrary::stop()
{
	switch (state())
	{
	case TestLibraryState::WaitingForConfiguration:
	case TestLibraryState::Error:
		{
			setState(TestLibraryState::Idle);
			break;
		}
	case TestLibraryState::Running:
		{
			stopTests();
			break;
		}
	}
}
bool TestLibrary::loadTestsFromPath()
{
	Q_ASSERT (m_librarySettings.loadScriptsFromPath() == true);

	m_testScriptsStorage.clear();

	QString errorMsg;

	bool loadResult = m_testScriptsStorage.loadFromPath(m_librarySettings.scriptsPath(), &errorMsg);
	if (loadResult == false)
	{
		writeError(errorMsg);
	}
	else
	{
		writeMessage(tr("Loaded %1 test scripts from \"%2\"").arg(m_testScriptsStorage.count()).arg(m_librarySettings.scriptsPath()));
	}

	return loadResult;
}

bool TestLibrary::loadTestsFromConfiguration()
{
	Q_ASSERT (m_librarySettings.loadScriptsFromPath() == false);

	bool loadResult = true;

	m_testScriptsStorage.clear();

	ConfigSettings configuration = m_configController.configuration();

	for (const QString& sf : configuration.scriptFiles)
	{
		QByteArray data;
		QString errorMsg;

		loadResult &= m_configController.getFileBlocked(sf, &data, &errorMsg);

		if (loadResult == false)
		{
			QString completeErrorMessage = tr("ConfigController::getFileBlocked: Get %1 file error:\n%2").arg(sf).arg(errorMsg);
			writeError(completeErrorMessage);
			break;
		}

		writeMessage("Test file: " + sf);

		TestScript ts;
		ts.fileName = sf;
		ts.script = data;
		m_testScriptsStorage.add(ts);
	}
	return loadResult;
}

void TestLibrary::runTests()
{
	if (state() == TestLibraryState::Running)
	{
		Q_ASSERT(false);
		return;
	}

	if (m_testWorkerThread != nullptr)
	{
		Q_ASSERT(m_testWorkerThread == nullptr);
		return;
	}

	setState(TestLibraryState::Running);

	m_testLog.writeMessage("TestLibrary::execute");

	m_testWorkerThread = new TestWorkerThread(&m_testController, &m_scriptTestLog, this);
	connect(m_testWorkerThread, &TestWorkerThread::finished, this, &TestLibrary::onTestingFinished);
	m_testWorkerThread->worker().setScripts(m_testScriptsStorage.scripts());	// Set all scripts to the worker
	m_testWorkerThread->run();
}

void TestLibrary::stopTests()
{
	if (state() != TestLibraryState::Running)
	{
		Q_ASSERT(false);
		return;
	}

	if (m_testWorkerThread == nullptr)
	{
		Q_ASSERT(m_testWorkerThread == nullptr);
		return;
	}

	m_testWorkerThread->stop();
}

TestLibraryState TestLibrary::state() const
{
	return m_state;
}

void TestLibrary::setState(TestLibraryState state)
{
	m_state = state;
}

void TestLibrary::onConfigurationArrived()
{
	if (state() == TestLibraryState::Running)
	{
		stopTests();
	}

	if (m_librarySettings.loadScriptsFromPath() == false)
	{
		bool ok = loadTestsFromConfiguration();
		if (ok == false)
		{
			setState(TestLibraryState::Error);
		}
	}

	if (state() == TestLibraryState::WaitingForConfiguration)
	{
		execute();
	}

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

void TestLibrary::onTestingFinished(int errorCode)
{
	if (m_testWorkerThread == nullptr)
	{
		Q_ASSERT(m_testWorkerThread);
		return;
	}

	m_testLog.writeMessage("TestLibrary::finished");
	m_testWorkerThread->deleteLater();
	m_testWorkerThread = nullptr;

	setState(TestLibraryState::Idle);

	emit testingFinished(errorCode);
}

