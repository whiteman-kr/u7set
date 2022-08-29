#pragma once
#include "../lib/Tuning/TuningTcpClient.h"
#include "../lib/Tuning/TuningSignalManager.h"
#include "../UtilsLib/LogFile.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "MonitorConfigController.h"
#include "../lib/Tuning/TuningLog.h"
#include "../lib/Tuning/TuningUserManager.h"

class MonitorTuningTcpClient : public TuningTcpClient, public TcpClientStatistics
{
	Q_OBJECT

public:
    MonitorTuningTcpClient(const SoftwareInfo& softwareInfo, const QString& tuningServiceId, TuningSignalManager* signalManager, ILogFile* logFile,
                           TuningLog::TuningLog* tuningLog, TuningUserManager* tuningUserManager);
	virtual ~MonitorTuningTcpClient() = default;

protected:
	virtual void writeLogAlert(const QString& message) override;
	virtual void writeLogError(const QString& message) override;
	virtual void writeLogWarning(const QString& message) override;
	virtual void writeLogMessage(const QString& message) override;

    virtual void writeLogSignalChange(const AppSignalParam& param, const TuningValue& oldValue, const TuningValue& newValue) override;

private:
	HasLogFile m_logFile;
    TuningLog::TuningLog* m_tuningLog = nullptr;
    TuningUserManager* m_tuningUserManager = nullptr;

};

