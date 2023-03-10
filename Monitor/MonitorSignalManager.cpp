#include "MonitorSignalManager.h"


MonitorSignalManager::MonitorSignalManager(ILogFile* logFile, QObject* parent /*= nullptr*/) :
	ClientLib::AppSignalManager{logFile, parent}
{
}
