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

void GlobalMessanger::fireBuildStarted()
{
	emit buildStarted();
}

void GlobalMessanger::fireBuildFinished()
{
	emit buildFinished();
}

void GlobalMessanger::clearBuildSchemeIssues()
{
	QMutexLocker ml(&m_buildResultMutex);
	m_buildSchemeIssues.clear();
}

void GlobalMessanger::swapSchemeIssues(std::map<QUuid, OutputMessageLevel>& data)
{
	QMutexLocker ml(&m_buildResultMutex);
	std::swap(m_buildSchemeIssues, data);
}

OutputMessageLevel GlobalMessanger::issueForSchemeItem(const QUuid itemId) const
{
	QMutexLocker ml(&m_buildResultMutex);

	auto it = m_buildSchemeIssues.find(itemId);

	if (it == m_buildSchemeIssues.end())
	{
		// Either Success or did not take part in build
		//
		return OutputMessageLevel::Success;
	}

	return it->second;
}
