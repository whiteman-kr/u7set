#include "MonitorTuningTcpClient.h"

MonitorTuningTcpClient::MonitorTuningTcpClient(const SoftwareInfo& softwareInfo, TuningSignalManager* signalManager, Log::LogFile* logFile) :
	TuningTcpClient(softwareInfo, signalManager),
	m_logFile(logFile)
{
	setAutoApply(true);

	if (m_logFile == nullptr)
	{
		assert(m_logFile);
	}
}

MonitorTuningTcpClient::~MonitorTuningTcpClient()
{
}

int MonitorTuningTcpClient::sourceErrorCount() const
{
	QMutexLocker l(&m_tuningSourcesMutex);

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

	if (m_logFile != nullptr)
	{
		m_logFile->writeAlert(message);
	}
}

void MonitorTuningTcpClient::writeLogError(const QString& message)
{
	TuningTcpClient::writeLogError(message);

	if (m_logFile != nullptr)
	{
		m_logFile->writeError(message);
	}
}

void MonitorTuningTcpClient::writeLogWarning(const QString& message)
{
	TuningTcpClient::writeLogWarning(message);

	if (m_logFile != nullptr)
	{
		m_logFile->writeWarning(message);
	}
}

void MonitorTuningTcpClient::writeLogMessage(const QString& message)
{
	TuningTcpClient::writeLogMessage(message);

	if (m_logFile != nullptr)
	{
		m_logFile->writeMessage(message);
	}
}
