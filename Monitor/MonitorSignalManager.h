#pragma once

#include "../ClientLib/AppSignalManager.h"

class MonitorSignalManager : public ClientLib::AppSignalManager
{
	Q_OBJECT

public:
	explicit MonitorSignalManager(ILogFile* logFile, QObject* parent = nullptr);
};
