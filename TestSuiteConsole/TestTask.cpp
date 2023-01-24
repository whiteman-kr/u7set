#include "TestTask.h"

#include <cassert>
#include <iostream>

TestTask::TestTask(const SoftwareInfo& softwareInfo,
				   HostAddressPort configurationServiceAddress1,
				   HostAddressPort configurationServiceAddress2,
				   QObject* parent) :
	QObject(parent),
	m_LogFile(qAppName(), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + softwareInfo.equipmentID()),
	m_configController(softwareInfo, configurationServiceAddress1, configurationServiceAddress2, &m_LogFile)
{
	QObject::connect(&m_testEngine.testResultLog(), &TestResultLog::newLogItem, this, &TestTask::newLogItem);
	connect(&m_testEngine, &TestEngine::finished, this, &TestTask::finished);

	connect(&m_configController, &TestSuiteConfigController::configurationArrived, this, &TestTask::slot_configurationArrived);
	connect(&m_configController, &TestSuiteConfigController::unknownClient, this, &TestTask::slot_unknownClient);
	connect(&m_configController, &TestSuiteConfigController::wrongClientHostname, this, &TestTask::slot_wrongClientHostname);
	m_configController.start();

	std::cout << "Waiting for connection with Configuration Service...\n";
}

TestSuiteConfigController& TestTask::configController()
{
	return m_configController;
}

const TestSuiteConfigController& TestTask::configController() const
{
	return m_configController;
}

void TestTask::slot_configurationArrived(ConfigSettings configuration)
{
	std::cout<< "slot_configurationArrived\n";

	start();


	return;
}

void TestTask::slot_unknownClient(QString errMsg)
{
	Q_UNUSED(errMsg);

	// CfgService did not find SoftwareID
	//
	std::cout << tr("Configuration Service does not recognize TestSuite EquipmentID %1\n")
						  .arg(m_configController.softwareInfo().equipmentID()).toStdString();
	return;
}

void TestTask::slot_wrongClientHostname(QString errMsg)
{
	Q_UNUSED(errMsg);

	// CfgService did not find SoftwareID
	//
	std::cout << tr("Configuration Service reporting - TestSuite running on computer with wrong hostanme\n").toStdString();
	return;
}

void TestTask::start()
{
	m_testEngine.start();

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
	m_testEngine.stop();
}

bool TestTask::isRunning() const
{
	return m_testEngine.isRunning();
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

