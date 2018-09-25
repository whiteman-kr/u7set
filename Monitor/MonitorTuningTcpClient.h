#ifndef MONITORTUNINGTCPCLIENT_H
#define MONITORTUNINGTCPCLIENT_H
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "MonitorConfigController.h"


class MonitorTuningTcpClient : public TuningTcpClient
{
	Q_OBJECT

public:
	MonitorTuningTcpClient(const SoftwareInfo& softwareInfo, TuningSignalManager* signalManager);
	virtual ~MonitorTuningTcpClient();
};

#endif // MONITORTUNINGTCPCLIENT_H
