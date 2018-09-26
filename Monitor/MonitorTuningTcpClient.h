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

protected:
	virtual void writeLogAlert(const QString& message) override;
	virtual void writeLogError(const QString& message) override;
	virtual void writeLogWarning(const QString& message) override;
	virtual void writeLogMessage(const QString& message) override;

};

#endif // MONITORTUNINGTCPCLIENT_H
