#include "MonitorTuningTcpClient.h"

MonitorTuningTcpClient::MonitorTuningTcpClient(const SoftwareInfo& softwareInfo, TuningSignalManager* signalManager) :
	TuningTcpClient(softwareInfo, signalManager)
{
	setAutoApply(true);
}

MonitorTuningTcpClient::~MonitorTuningTcpClient()
{
}

