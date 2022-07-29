#include "MonitorArchive.h"
#include "MonitorArchiveWidget.h"

//
//
// MonitorArchive
//
//
std::map<QString, MonitorArchiveWidget*> MonitorArchive::s_archiveList;

std::vector<QString> MonitorArchive::getArchiveList()
{
	std::vector<QString> result;
	result.reserve(s_archiveList.size());

	for (std::pair<QString, MonitorArchiveWidget*> p : s_archiveList)
	{
		result.push_back(p.first);
	}

	return result;
}

bool MonitorArchive::activateWindow(QString archiveName)
{
	if (s_archiveList.count(archiveName) != 1)
	{
		Q_ASSERT(s_archiveList.count(archiveName) != 1);
		return false;
	}

	MonitorArchiveWidget* widget = s_archiveList[archiveName];
	Q_ASSERT(widget);

	widget->activateWindow();
	widget->ensureVisible();

	return true;
}

bool MonitorArchive::startNewWidget(MonitorSignalManager* signalManager,
									MonitorConfigController* configController,
									const std::vector<AppSignalParam>& appSignals,
									QWidget* parent)
{
	Q_ASSERT(signalManager);
	Q_ASSERT(configController);

	MonitorArchiveWidget* window = new MonitorArchiveWidget(signalManager, configController, parent);
	window->setSignals(appSignals);
	window->show();

	return false;
}

bool MonitorArchive::requestArchiveWithNewWidget(MonitorSignalManager* signalManager,
												 MonitorConfigController* configController,
												 const std::vector<AppSignalParam>& appSignals,
												 QDateTime startTime,
												 QDateTime endTime,
												 E::TimeType timeType,
												 QWidget* parent)
{
	Q_ASSERT(signalManager);
	Q_ASSERT(configController);

	MonitorArchiveWidget* window = new MonitorArchiveWidget(signalManager, configController, parent);

	window->setSignals(appSignals);
	window->setTime(startTime, endTime, timeType);

	window->show();

	window->requestDataOnConnection();

	return false;
}

void MonitorArchive::registerWindow(QString name, MonitorArchiveWidget* window)
{
	Q_ASSERT(s_archiveList.count(name) == 0);
	s_archiveList[name] = window;

	return;
}

void MonitorArchive::unregisterWindow(QString name)
{
	Q_ASSERT(s_archiveList.count(name) == 1);
	s_archiveList.erase(name);

	return;
}

