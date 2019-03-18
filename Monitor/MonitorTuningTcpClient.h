#ifndef MONITORTUNINGTCPCLIENT_H
#define MONITORTUNINGTCPCLIENT_H
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../lib/LogFile.h"
#include "MonitorConfigController.h"
#include "../lib/TcpClientsStatistics.h"

class MonitorTuningTcpClient : public TuningTcpClient, public TcpClientInstance
{
	Q_OBJECT

public:
	MonitorTuningTcpClient(const SoftwareInfo& softwareInfo, TuningSignalManager* signalManager, Log::LogFile* logFile);
	virtual ~MonitorTuningTcpClient();

	int sourceErrorCount() const;
protected:
	virtual void writeLogAlert(const QString& message) override;
	virtual void writeLogError(const QString& message) override;
	virtual void writeLogWarning(const QString& message) override;
	virtual void writeLogMessage(const QString& message) override;

private:
	Log::LogFile* m_logFile = nullptr;

};

#endif // MONITORTUNINGTCPCLIENT_H
