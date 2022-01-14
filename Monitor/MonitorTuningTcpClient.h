#pragma once
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../UtilsLib/LogFile.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "MonitorConfigController.h"

class MonitorTuningTcpClient : public TuningTcpClient, public TcpClientStatistics
{
	Q_OBJECT

public:
	MonitorTuningTcpClient(const SoftwareInfo& softwareInfo, const QString& tuningServiceId, TuningSignalManager* signalManager, ILogFile* logFile);
	virtual ~MonitorTuningTcpClient();

protected:
	virtual void writeLogAlert(const QString& message) override;
	virtual void writeLogError(const QString& message) override;
	virtual void writeLogWarning(const QString& message) override;
	virtual void writeLogMessage(const QString& message) override;

private:
	HasLogFile m_logFile;

};

