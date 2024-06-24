#pragma once

class ArchiveWidget;
class MonitorConfigController;

namespace ClientLib
{
	class AppSignalManager;
}

namespace AppSignalLists
{
	class AppSignalListSet;
}

class MonitorArchive
{
public:
	MonitorArchive() = delete;

public:
	static std::vector<QString> getArchiveList();
	static bool activateWindow(QString archiveName);

	static bool startNewWidget(ClientLib::AppSignalManager& signalManager,
							   MonitorConfigController* configController,
							   const std::vector<AppSignalParam>& appSignals,
							   const AppSignalLists::AppSignalListSet& appSignalListSet,
							   QWidget* parent);

	static bool requestArchiveWithNewWidget(ClientLib::AppSignalManager& signalManager,
											MonitorConfigController* configController,
											const std::vector<AppSignalParam>& appSignals,
											const AppSignalLists::AppSignalListSet& appSignalListSet,
											QDateTime startTime,
											QDateTime endTime,
											E::TimeType timeType,
											QWidget* parent);

	static void registerWindow(QString name, ArchiveWidget* window);
	static void unregisterWindow(QString name);

private:
	static std::map<QString, ArchiveWidget*> s_archiveList;
};
