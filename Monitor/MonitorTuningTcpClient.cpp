#include "MonitorTuningTcpClient.h"

MonitorTuningTcpClient::MonitorTuningTcpClient(const SoftwareInfo& softwareInfo, const QString& tuningServiceId, TuningSignalManager& signalManager, ILogFile* logFile,
											   TuningLog::TuningLog* tuningLog, TuningUserManager& tuningUserManager) :
	TuningTcpClient(softwareInfo, tuningServiceId, false/*singleLmControlMode*/, signalManager),
	TcpClientStatistics(this),
    m_logFile(logFile, "TuningTcpClient"),
    m_tuningLog(tuningLog),
    m_tuningUserManager(tuningUserManager)
{
	setObjectName("MonitorTuningTcpClient");

	Q_ASSERT(logFile);

	setAutoApply(true);
}

void MonitorTuningTcpClient::writeLogAlert(const QString& message)
{
	TuningTcpClient::writeLogAlert(message);

	if (m_logFile.logFile() != nullptr)
	{
		m_logFile.writeAlert(message);
	}
}

void MonitorTuningTcpClient::writeLogError(const QString& message)
{
	TuningTcpClient::writeLogError(message);

	if (m_logFile.logFile() != nullptr)
	{
		m_logFile.writeError(message);
	}
}

void MonitorTuningTcpClient::writeLogWarning(const QString& message)
{
	TuningTcpClient::writeLogWarning(message);

	if (m_logFile.logFile() != nullptr)
	{
		m_logFile.writeWarning(message);
	}
}

void MonitorTuningTcpClient::writeLogMessage(const QString& message)
{
	TuningTcpClient::writeLogMessage(message);

	if (m_logFile.logFile() != nullptr)
	{
		m_logFile.writeMessage(message);
	}
}

void MonitorTuningTcpClient::writeLogSignalChange(const AppSignalParam& param, const TuningValue& oldValue, const TuningValue& newValue)
{
	m_tuningLog->write(param, oldValue, newValue, m_tuningUserManager.loggedInUser());
}
