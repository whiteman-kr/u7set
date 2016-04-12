#include "GlobalMessanger.h"


GlobalMessanger* GlobalMessanger::m_instance = nullptr;

GlobalMessanger::GlobalMessanger() :
	QObject()
{
}

GlobalMessanger* GlobalMessanger::instance()
{
	if (m_instance == nullptr)
	{
		m_instance = new GlobalMessanger();
	}

	return m_instance;
}

void GlobalMessanger::free()
{
	delete m_instance;
}

void GlobalMessanger::fireProjectOpened(DbProject project)
{
	emit projectOpened(project);
}

void GlobalMessanger::fireProjectClosed()
{
	emit projectClosed();
}

void GlobalMessanger::fireShowDeviceApplicationSignals(QStringList deviceStrIds, bool refreshSignalList)
{
	qDebug() << "Show Application signals for devices " << deviceStrIds;
	emit showDeviceApplicationSignals(deviceStrIds, refreshSignalList);
}

void GlobalMessanger::fireBuildStarted()
{
	emit buildStarted();
}

void GlobalMessanger::fireBuildFinished()
{
	emit buildFinished();
}

void GlobalMessanger::clearBuildSchemaIssues()
{
	QMutexLocker ml(&m_buildResultMutex);
	m_buildSchemaIssues.clear();
}

void GlobalMessanger::swapSchemaIssues(std::map<QUuid, OutputMessageLevel>& data)
{
	QMutexLocker ml(&m_buildResultMutex);
	std::swap(m_buildSchemaIssues, data);
}

OutputMessageLevel GlobalMessanger::issueForSchemaItem(const QUuid itemId) const
{
	QMutexLocker ml(&m_buildResultMutex);

	auto it = m_buildSchemaIssues.find(itemId);

	if (it == m_buildSchemaIssues.end())
	{
		// Either Success or did not take part in build
		//
		return OutputMessageLevel::Success;
	}

	return it->second;
}

void GlobalMessanger::fireChangeCurrentTab(QWidget* tab)
{
	emit changeCurrentTab(tab);
}
