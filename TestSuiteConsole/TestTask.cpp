#include "TestTask.h"

#include <cassert>
#include <iostream>

TestTask::TestTask(QObject* parent) :
	QObject(parent)
{
	QObject::connect(&m_testEngine.testResultLog(), &TestResultLog::newLogItem, this, &TestTask::newLogItem);
	connect(&m_testEngine, &TestEngine::finished, this, &TestTask::finished);
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

