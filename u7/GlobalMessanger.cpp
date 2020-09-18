#include "GlobalMessanger.h"


GlobalMessanger* GlobalMessanger::m_instance = nullptr;

GlobalMessanger::GlobalMessanger() :
	QObject()
{

static int CompareDataTypeId = -1;
	if (CompareDataTypeId == -1)
	{
		CompareDataTypeId= qRegisterMetaType<CompareData>();
	}

}

GlobalMessanger& GlobalMessanger::instance()
{
	static GlobalMessanger inst;
	return inst;
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

void GlobalMessanger::fireAddLogicSchema(QStringList deviceStrIds, QString lmDescriptionFile)
{
	qDebug() << "Add Logic Schema to LogicModule(s) " << deviceStrIds;
	emit addLogicSchema(deviceStrIds, lmDescriptionFile);
}

void GlobalMessanger::fireSearchSchemaForLm(QString equipmentId)
{
	emit searchSchemaForLm(equipmentId);
}

void GlobalMessanger::fireBuildStarted()
{
	emit buildStarted();
}

void GlobalMessanger::fireBuildFinished(int errorCount)
{
	emit buildFinished(errorCount);
}

void GlobalMessanger::clearBuildSchemaIssues()
{
	m_buildIssues.clear();
}

OutputMessageLevel GlobalMessanger::issueForSchemaItem(const QUuid& itemId) const
{
	return m_buildIssues.issueForSchemaItem(itemId);
}

Builder::BuildIssues::Counter GlobalMessanger::issueForSchema(const QString& schemaId) const
{
	return m_buildIssues.issueForSchema(schemaId);
}

void GlobalMessanger::clearSchemaItemRunOrder()
{
	QMutexLocker ml(&m_buildResultMutex);
	m_runOrder.clear();
}

std::pair<int, int> GlobalMessanger::schemaItemRunOrder(const QString& equipmentId, const QUuid& itemId) const
{
	QMutexLocker ml(&m_buildResultMutex);
	return m_runOrder.schemaItemRunOrder(equipmentId, itemId);
}

void GlobalMessanger::fireChangeCurrentTab(QWidget* tab)
{
	emit changeCurrentTab(tab);
}

void GlobalMessanger::fireCompareObject(DbChangesetObject object, CompareData compareData)
{
	emit compareObject(object, compareData);
}

void GlobalMessanger::fireCompareProject(CompareData compareData)
{
	emit compareProject(compareData);
}

void GlobalMessanger::runOrderReady(Builder::RunOrder runOrder)
{
	QMutexLocker ml(&m_buildResultMutex);
	m_runOrder = std::move(runOrder);
}

Builder::BuildIssues& GlobalMessanger::buildIssues()
{
	return m_buildIssues;
}

const Builder::BuildIssues& GlobalMessanger::buildIssues() const
{
	return m_buildIssues;
}
