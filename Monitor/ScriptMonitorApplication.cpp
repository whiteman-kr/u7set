#include "ScriptMonitorApplication.h"
#include "MonitorMainWindow.h"

ScriptMonitorApplication::ScriptMonitorApplication() :
	QObject(nullptr)
{
}

void ScriptMonitorApplication::showArchive(QStringList signalList, QDateTime startTime, QDateTime endTime, int timeType)
{
	emit signal_showArchive(std::move(signalList), startTime, endTime, timeType);
}

void ScriptMonitorApplication::showTrendArchive(QStringList appSignalIds,
												QDateTime startTime,
												QDateTime endTime,
												E::TimeType timeType,
												int laneCount,
												E::TrendViewMode viewMode,
												E::TrendScaleType scaleType)
{
	emit signal_showTrendArchive(std::move(appSignalIds), startTime, endTime, timeType, laneCount, viewMode, scaleType);
}

void ScriptMonitorApplication::showSnapshot(QStringList signalList)
{
	emit signal_showSnapshot(std::move(signalList));
}

void ScriptMonitorApplication::showSnapshotByMask(QStringList masks)
{
	emit signal_showSnapshotByMask(std::move(masks));
}

void ScriptMonitorApplication::showSnapshotByTag(QStringList tags)
{
	emit signal_showSnapshotByTag(std::move(tags));
}

void ScriptMonitorApplication::setVisibleTabBar(bool visible)
{
	emit signal_setVisibleTabBar(visible);
}

void ScriptMonitorApplication::setVisibleSchemaTree(bool visible)
{
	emit signal_setVisibleSchemaTree(visible);
}

void ScriptMonitorApplication::toggleSchemaTree()
{
	emit signal_toggleSchemaTree();
}

void ScriptMonitorApplication::setVisibleToolBar(bool visible)
{
	emit signal_setVisibleToolBar(visible);
}

void ScriptMonitorApplication::setVisibleStatusBar(bool visible)
{
	emit signal_setVisibleStatusBar(visible);
}

void ScriptMonitorApplication::setVisibleMenu(bool visible)
{
	emit signal_setVisibleMenu(visible);
}

void ScriptMonitorApplication::setFullScreen(bool fullScreen)
{
	emit signal_setFullScreen(fullScreen);
}

bool ScriptMonitorApplication::start(QString program, QString arguments, QString workDir)
{
	return QProcess::startDetached(program, arguments.split(';', Qt::SkipEmptyParts), workDir);
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
		// clang-format off
		connect(this, &ScriptMonitorApplication::signal_showArchive, m_mainWindow, qOverload<QStringList, QDateTime, QDateTime, int>(&MonitorMainWindow::slot_archive), Qt::QueuedConnection);

		connect(this, &ScriptMonitorApplication::signal_showTrendArchive, m_mainWindow, &MonitorMainWindow::slot_trendArchive, Qt::QueuedConnection);

		connect(this, &ScriptMonitorApplication::signal_showSnapshot, m_mainWindow, qOverload<QStringList>(&MonitorMainWindow::slot_signalSnapshot), Qt::QueuedConnection);
		connect(this, &ScriptMonitorApplication::signal_showSnapshotByMask, m_mainWindow, &MonitorMainWindow::slot_signalSnapshotByMask, Qt::QueuedConnection);
		connect(this, &ScriptMonitorApplication::signal_showSnapshotByTag, m_mainWindow, &MonitorMainWindow::slot_signalSnapshotByTag, Qt::QueuedConnection);

		connect(this, &ScriptMonitorApplication::signal_toggleSchemaTree, m_mainWindow, &MonitorMainWindow::toggleSchemaTree, Qt::QueuedConnection);
		connect(this, &ScriptMonitorApplication::signal_setVisibleSchemaTree, m_mainWindow, &MonitorMainWindow::setVisibleSchemaTree, Qt::QueuedConnection);
		connect(this, &ScriptMonitorApplication::signal_setVisibleTabBar, m_mainWindow, &MonitorMainWindow::setVisibleTabBar, Qt::QueuedConnection);
		connect(this, &ScriptMonitorApplication::signal_setVisibleToolBar, m_mainWindow, &MonitorMainWindow::setVisibleToolBar, Qt::QueuedConnection);
		connect(this, &ScriptMonitorApplication::signal_setVisibleStatusBar, m_mainWindow, &MonitorMainWindow::setVisibleStatusBar, Qt::QueuedConnection);
		connect(this, &ScriptMonitorApplication::signal_setVisibleMenu, m_mainWindow, &MonitorMainWindow::setVisibleMenu, Qt::QueuedConnection);
		connect(this, &ScriptMonitorApplication::signal_setFullScreen, m_mainWindow, &MonitorMainWindow::setFullScreen, Qt::QueuedConnection);
		// clang-format on
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
