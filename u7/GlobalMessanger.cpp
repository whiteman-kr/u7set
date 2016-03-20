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
