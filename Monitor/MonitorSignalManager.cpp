#include "MonitorSignalManager.h"


MonitorSignalManager::MonitorSignalManager(ILogFile* logFile, QObject* parent /*= nullptr*/) :
	Client::AppSignalManager{logFile, parent}
{
}
