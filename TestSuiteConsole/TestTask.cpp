#include "TestTask.h"

#include <cassert>
#include <iostream>

TestTask::TestTask(const SoftwareInfo& softwareInfo,
				   HostAddressPort configurationServiceAddress1,
				   HostAddressPort configurationServiceAddress2,
				   const QString& scriptsPath,
				   QObject* parent) :
	QObject(parent),
	m_LogFile(qAppName(), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + softwareInfo.equipmentID()),
	m_testLibrary(softwareInfo, configurationServiceAddress1, configurationServiceAddress2, &m_LogFile)
{
	connect(&m_testLibrary.testResultLog(), &TestLog::newLogItem, this, &TestTask::newLogItem);
	connect(&m_testLibrary, &TestLibrary::readyForTesting, this, &TestTask::slot_startTests);
	connect(&m_testLibrary, &TestLibrary::finished, this, &TestTask::finished);

	connect(&m_testLibrary, &TestLibrary::logMessage, this, &TestTask::slot_logMessage);
	connect(&m_testLibrary, &TestLibrary::logError, this, &TestTask::slot_logError);
	connect(&m_testLibrary.configController(), &TestSuiteConfigController::logMessage, this, &TestTask::slot_logMessage);
	connect(&m_testLibrary.configController(), &TestSuiteConfigController::logError, this, &TestTask::slot_logError);


	// Load test scripts from files if specified
	//
	if (scriptsPath.isEmpty() == false)
	{
		QString errorMsg;

		bool result = m_testLibrary.testScriptsStorage().loadFromPath(scriptsPath, &errorMsg);
		if (result == false)
		{
			std::cout << errorMsg.toStdString() << std::endl;
		}
		else
		{
			std::cout << "Loaded " << m_testLibrary.testScriptsStorage().count()
					  << " test scripts from \"" << scriptsPath.toStdString() << "\"." << std::endl;
		}
	}

	std::cout << "Waiting for connection with Configuration Service...\n";
}

void TestTask::slot_startTests()
{
	start();

	return;
}

void TestTask::slot_logMessage(const QString& msg)
{
	std::cout << msg.toStdString() << std::endl;
	return;
}

void TestTask::slot_logError(const QString& errMsg)
{
	std::cout << "[ERR] " << errMsg.toStdString() << std::endl;
	return;
}


void TestTask::start()
{
	m_testLibrary.execute();

	/*
	m_builder.start(m_databaseAddress,
				  m_databasePort,
				  m_databaseUserName,
				  m_databasePassword,
				  m_projectName,
				  m_projectUserName,
				  m_projectUserPassword,
				  m_buildOutputPath,
				  false);
*/
	return;
}

void TestTask::stop()
{
	m_testLibrary.stop();
}

bool TestTask::isRunning() const
{
	return m_testLibrary.isRunning();
}

void TestTask::newLogItem(const TestLogItem& logItem)
{
	std::cout << logItem.toText().toStdString() << std::endl;
}

/*
void BuildTask::setDatabaseAddress(QString value)
{
	m_databaseAddress = value;
}

void BuildTask::setDatabasePort(int value)
{
	m_databasePort = value;
}

void BuildTask::setDatabaseUserName(QString value)
{
	m_databaseUserName = value;
}

void BuildTask::setDatabasePassword(QString value)
{
	m_databasePassword = value;
}

void BuildTask::setProjectName(QString value)
{
	m_projectName = value;
}

void BuildTask::setProjectUserName(QString value)
{
	m_projectUserName = value;
}

void BuildTask::setProjectUserPassword(QString value)
{
	m_projectUserPassword = value;
}

void BuildTask::setBuildOutputPath(QString value)
{
	m_buildOutputPath = value;
}*/

