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
	qDebug() << "GlobalMessanger::free()";
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
	m_buildIssues.clear();
}

void GlobalMessanger::swapSchemaIssues(Builder::BuildIssues* buildIssues)
{
	if (buildIssues == nullptr)
	{
		assert(buildIssues);
		return;
	}

	QMutexLocker ml(&m_buildResultMutex);
	m_buildIssues.swap(buildIssues);
	return;
}

OutputMessageLevel GlobalMessanger::issueForSchemaItem(const QUuid& itemId) const
{
	QMutexLocker ml(&m_buildResultMutex);

	auto it = m_buildIssues.m_items.find(itemId);

	if (it == m_buildIssues.m_items.end())
	{
		// Either Success or did not take part in build
		//
		return OutputMessageLevel::Success;
	}

	return it->second;
}

Builder::BuildIssues::Counter GlobalMessanger::issueForSchema(const QString& schemeId) const
{
	QMutexLocker ml(&m_buildResultMutex);

	auto it = m_buildIssues.m_schemas.find(schemeId);

	if (it == m_buildIssues.m_schemas.end())
	{
		// Either Success or did not take part in build
		//
		return Builder::BuildIssues::Counter();
	}

	return it->second;
}

void GlobalMessanger::clearSchemaItemRunOrder()
{
	QMutexLocker ml(&m_buildResultMutex);
	m_runOrder.clear();
}

void GlobalMessanger::setRunOrder(const QString& equipmentId, std::map<QUuid, int>& data)
{
	QMutexLocker ml(&m_buildResultMutex);

	RunOrderDebugInfo ro = {equipmentId, data};
	m_runOrder.push_back(ro);

	return;
}

int GlobalMessanger::schemaItemRunOrder(const QString& equipmentId, const QUuid& itemId) const
{
	QMutexLocker ml(&m_buildResultMutex);

	for (const RunOrderDebugInfo& ro : m_runOrder)
	{
		if (ro.equipmentId == equipmentId)
		{
			auto it = ro.schemaItemsRunOrder.find(itemId);

			if (it == ro.schemaItemsRunOrder.end())
			{
				return -1;
			}

			return it->second;
		}
	}

	return -1;
}

void GlobalMessanger::fireChangeCurrentTab(QWidget* tab)
{
	emit changeCurrentTab(tab);
}
