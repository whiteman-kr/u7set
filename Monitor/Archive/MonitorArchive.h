#pragma once
#include "../../AppSignalLib/AppSignalParam.h"

class ArchiveWidget;
class MonitorSignalManager;
class MonitorConfigController;


class MonitorArchive
{
public:
	MonitorArchive() = delete;

public:
	static std::vector<QString> getArchiveList();
	static bool activateWindow(QString archiveName);

	static bool startNewWidget(MonitorSignalManager* signalManager,
							   MonitorConfigController* configController,
							   const std::vector<AppSignalParam>& appSignals,
							   QWidget* parent);

	static bool requestArchiveWithNewWidget(MonitorSignalManager* signalManager,
											MonitorConfigController* configController,
											const std::vector<AppSignalParam>& appSignals,
											QDateTime startTime,
											QDateTime endTime,
											E::TimeType timeType,
											QWidget* parent);

	static void registerWindow(QString name, ArchiveWidget* window);
	static void unregisterWindow(QString name);

private:
	static std::map<QString, ArchiveWidget*> s_archiveList;
};
