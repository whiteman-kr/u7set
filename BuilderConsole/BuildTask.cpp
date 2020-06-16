#include "BuildTask.h"
#include <cassert>
#include <iostream>

BuildTask::BuildTask(QObject* parent) :
	QObject(parent)
{
	QObject::connect(&m_builder.log(), &OutputLog::newLogItem, this, &BuildTask::newLogItem);
	connect(&m_builder, &Builder::Builder::finished, this, &BuildTask::finished);
}

void BuildTask::start()
{
	qDebug() << Q_FUNC_INFO;

	m_builder.start(m_databaseAddress,
				  m_databasePort,
				  m_databaseUserName,
				  m_databasePassword,
				  m_projectName,
				  m_projectUserName,
				  m_projectUserPassword,
				  m_buildOutputPath,
				  false);

	return;
}

void BuildTask::stop()
{
	m_builder.stop();
}

bool BuildTask::isRunning() const
{
	return m_builder.isRunning();
}

void BuildTask::newLogItem(OutputLogItem logItem)
{
	std::cout << logItem.toText().toStdString() << std::endl;
}

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
}

