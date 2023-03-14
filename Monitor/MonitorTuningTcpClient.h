#pragma once
#include "../UtilsLib/ILogFile.h"
#include "../OnlineLib/TcpClientStatistics.h"
#include "../ClientLib/TuningUserManager.h"
#include "../ClientLib/TuningTcpClient.h"
#include "../ClientLib/TuningLog.h"
#include "../AppSignalLib/TuningSignalManager.h"


class MonitorTuningTcpClient : public ClientLib::TuningTcpClient, public TcpClientStatistics
{
	Q_OBJECT

public:
	MonitorTuningTcpClient(const SoftwareInfo& softwareInfo,
						   const QString& tuningServiceId,
						   TuningSignalManager& signalManager,
						   ILogFile* logFile,
						   TuningLog::TuningLog* tuningLog,
						   ClientLib::TuningUserManager& tuningUserManager);
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
	ClientLib::TuningUserManager& m_tuningUserManager;
};

