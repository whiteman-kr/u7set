#include "MonitorTuningTcpClient.h"

MonitorTuningTcpClient::MonitorTuningTcpClient(const SoftwareInfo& softwareInfo, const QString& tuningServiceId, TuningSignalManager* signalManager, ILogFile* logFile) :
	TuningTcpClient(softwareInfo, tuningServiceId, signalManager),
	TcpClientStatistics(this),
	m_logFile(logFile, "TuningTcpClient")
{
	setObjectName("MonitorTuningTcpClient");

	Q_ASSERT(logFile);

	setAutoApply(true);
}

MonitorTuningTcpClient::~MonitorTuningTcpClient()
{
}

int MonitorTuningTcpClient::sourceErrorCount() const
{
	QReadLocker l(&m_tuningSourcesLock);

	int result = 0;

	for (const auto& it : m_tuningSources)
	{
		const TuningSource& ts = it.second;

		if (ts.state.isreply() == false && ts.state.controlisactive() == true)
		{
			result++;
			continue;
		}

		result += ts.getErrorsCount();
	}

	return result;
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
