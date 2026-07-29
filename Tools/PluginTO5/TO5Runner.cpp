#include "TO5Runner.h"

TO5Runner::TO5Runner(Log::LogFile& logFile):
	m_logFile(logFile),
	m_settings(PluginTO5Settings::restore()),
	m_softwareInfo(E::SoftwareType::TestSuite, m_settings.equipmentId),
	m_configController(m_softwareInfo,
					   HostAddressPort(m_settings.ipAddress1, m_settings.port1.toInt()),
					   HostAddressPort(m_settings.ipAddress2, m_settings.port2.toInt()),
					   &m_logFile),
	m_testSuite{m_configController, &m_logFile, &m_testLogOutput}
{

	QObject::connect(&m_testSuite,
					 &TestSuite::MatsTestSuite::finished,
					 [this](int result)
					 {
						 emit done(result);

						 // Save test log to the file
						 //
						 //saveTestLog(m_reportsPath + "\\$default", m_testSuite.testLog(), m_logFile);
					 });

	m_configController.start();
	
	connect(&m_testLogOutput,
			&PluginTestLog::logItem,
			[this](QString msg, TestSuite::TestLogItemType type)
			{
				emit logItem(msg, type);
			});
}

bool TO5Runner::isRunning() const
{ 
	return m_testSuite.isRunning();
}

void TO5Runner::setTestSuiteSettings(const PluginTO5Settings& settings)
{
	m_softwareInfo = SoftwareInfo(E::SoftwareType::TestSuite, settings.equipmentId);

	m_configController.setConnectionParams(settings.equipmentId,
										   HostAddressPort(settings.ipAddress1, settings.port1.toInt()),
										   HostAddressPort(settings.ipAddress2, settings.port2.toInt()));

	return;
}

bool TO5Runner::execute(const QString& testFilter)
{
	// Run tests.
	//
	TestSuite::TestScriptSelection selection {testFilter};

	TestSuite::ControlParams controlParams{{},
										   {} /*reports path*/,
										   selection,
										   {} /* QString::fromStdString(userName)*/,
										   {} /* QString::fromStdString(password)*/};

	bool ok = m_testSuite.execute(controlParams);

	return ok;
}

void TO5Runner::stop()
{ 
	if (m_testSuite.isRunning() == true)
	{
		m_testSuite.stop();
	}
}

TestSuite::ConfigData TO5Runner::configData() const
{ 
	return m_configController.configData(); 
}

const TestSuite::TestLog& TO5Runner::testLog()
{ 
	return m_testSuite.testLog(); 
}

const Tcp::ConnectionState TO5Runner::configConnectionState() const
{
	return m_configController.getConnectionState();
}

const TestSuite::ConfigSettings TO5Runner::configuration() const
{
	return m_configController.configuration();
}

const std::pair<int, int> TO5Runner::getLogIssuesCount() const 
{
	return m_testLogOutput.getLogIssuesCount();
}

void TO5Runner::saveTestLog(QString fileName, TestSuite::TestLog& testLog, Log::LogFile& appLog)
{
	if (fileName.isEmpty() == true)
	{
		Q_ASSERT(false);
		return;
	}

	if (fileName.contains("$default") == true)
	{
		fileName.replace("$default", QString("PluginTO5_%1.tsl").arg(QDateTime::currentDateTime().toString("ddMMyyyy_HHmmss")));
	}

	QString errorMsg;
	bool ok = testLog.saveToCSV(fileName, &errorMsg);
	if (ok == false)
	{
		appLog.writeError(errorMsg);
	}
	else
	{
		appLog.writeMessage(QObject::tr("Test log is saved to the file: '%1'.").arg(fileName));
	}
	return;
}
