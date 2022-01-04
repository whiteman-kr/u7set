#include "ScriptMonitorApplication.h"

ScriptMonitorApplication::ScriptMonitorApplication() :
	QObject(nullptr)
{
}

void ScriptMonitorApplication::showArchive(QStringList signalsList, QDateTime startTime, QDateTime endTime, int timeType)
{
	emit signal_showArchive(std::move(signalsList), startTime, endTime, timeType);
}

void ScriptMonitorApplication::showSnapshot(QStringList signalsList)
{
	emit signal_showSnapshot(std::move(signalsList));
}

void ScriptMonitorApplication::showSnapshotByMask(QStringList masks)
{
	emit signal_showSnapshotByMask(std::move(masks));
}

void ScriptMonitorApplication::showSnapshotByTag(QStringList tags)
{
	emit signal_showSnapshotByTag(std::move(tags));
}

QString ScriptMonitorApplication::equipmentId() const
{
	if (m_mainWindow == nullptr)
	{
		Q_ASSERT(m_mainWindow);
		return {};
	}

	return m_mainWindow->configController().softwareInfo().equipmentID();
}

void ScriptMonitorApplication::setMainWindow(MonitorMainWindow* mainWindow)
{
	m_mainWindow = mainWindow;

	if (m_mainWindow != nullptr)
	{
		connect(this, &ScriptMonitorApplication::signal_showArchive, m_mainWindow, qOverload<QStringList, QDateTime, QDateTime, int>(&MonitorMainWindow::slot_archive), Qt::QueuedConnection);
		connect(this, &ScriptMonitorApplication::signal_showSnapshot, m_mainWindow, qOverload<QStringList>(&MonitorMainWindow::slot_signalSnapshot), Qt::QueuedConnection);
		connect(this, &ScriptMonitorApplication::signal_showSnapshotByMask, m_mainWindow, &MonitorMainWindow::slot_signalSnapshotByMask, Qt::QueuedConnection);
		connect(this, &ScriptMonitorApplication::signal_showSnapshotByTag, m_mainWindow, &MonitorMainWindow::slot_signalSnapshotByTag, Qt::QueuedConnection);
	}

	return;
}

MonitorMainWindow* ScriptMonitorApplication::mainWindow()
{
	return m_mainWindow;
}

const MonitorMainWindow* ScriptMonitorApplication::mainWindow() const
{
	return m_mainWindow;
}
