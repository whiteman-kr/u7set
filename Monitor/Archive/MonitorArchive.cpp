#include "MonitorArchive.h"
#include "ArchiveWidget.h"
#include "../ClientLib/AppSignalManager.h"

//
//
// MonitorArchive
//
//
std::map<QString, ArchiveWidget*> MonitorArchive::s_archiveList;

std::vector<QString> MonitorArchive::getArchiveList()
{
	std::vector<QString> result;
	result.reserve(s_archiveList.size());

	for (std::pair<QString, ArchiveWidget*> p : s_archiveList)
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

	ArchiveWidget* widget = s_archiveList[archiveName];
	Q_ASSERT(widget);

	widget->activateWindow();
	widget->ensureVisible();

	return true;
}

bool MonitorArchive::startNewWidget(ClientLib::AppSignalManager& signalManager,
									MonitorConfigController* configController,
									const std::vector<AppSignalParam>& appSignals,
									QWidget* parent)
{
	Q_ASSERT(configController);

	ArchiveWidget* window = new ArchiveWidget(signalManager, configController, parent);
	window->setSignals(appSignals);
	window->show();

	return false;
}

bool MonitorArchive::requestArchiveWithNewWidget(ClientLib::AppSignalManager& signalManager,
												 MonitorConfigController* configController,
												 const std::vector<AppSignalParam>& appSignals,
												 QDateTime startTime,
												 QDateTime endTime,
												 E::TimeType timeType,
												 QWidget* parent)
{
	Q_ASSERT(configController);

	ArchiveWidget* window = new ArchiveWidget(signalManager, configController, parent);

	window->setSignals(appSignals);
	window->setTime(startTime, endTime, timeType);

	window->show();

	window->requestData();

	return false;
}

void MonitorArchive::registerWindow(QString name, ArchiveWidget* window)
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

