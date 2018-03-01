#include "MainWindow.h"
#include "TuningClientTcpClient.h"
#include "version.h"

TuningClientTcpClient::TuningClientTcpClient(const SoftwareInfo& softwareInfo,
											 TuningSignalManager* signalManager,
											 Log::LogFile* log,
											 TuningLog::TuningLog* tuningLog, UserManager* userManager) :
	TuningTcpClient(softwareInfo, signalManager),
	m_log(log),
	m_tuningLog(tuningLog),
	m_userManager(userManager)
{
	assert(m_log);
	assert(m_tuningLog);
	assert(m_userManager);

}

void TuningClientTcpClient::writeLogAlert(const QString& message)
{
	m_log->writeAlert(message);
}

void TuningClientTcpClient::writeLogError(const QString& message)
{
	m_log->writeError(message);
}

void TuningClientTcpClient::writeLogWarning(const QString& message)
{
	m_log->writeWarning(message);
}

void TuningClientTcpClient::writeLogMessage(const QString& message)
{
	m_log->writeMessage(message);
}

void TuningClientTcpClient::writeLogSignalChange(const AppSignalParam& param, const TuningValue& oldValue, const TuningValue& newValue)
{
	m_tuningLog->write(param, oldValue, newValue, m_userManager->loggedInUser());
}

void TuningClientTcpClient::writeLogSignalChange(const QString& message)
{
	m_tuningLog->write(message, m_userManager->loggedInUser());
}


int TuningClientTcpClient::sourceErrorCount() const
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

int TuningClientTcpClient::sourceErrorCount(Hash equipmentHash) const
{
	QMutexLocker l(&m_tuningSourcesMutex);

	if (m_tuningSources.find(equipmentHash) == m_tuningSources.end())
	{
		return 0;
	}

	const TuningSource& ts = m_tuningSources.at(equipmentHash);

	if (ts.state.isreply() == false && ts.state.controlisactive() == true)
	{
		return 1;
	}

	return ts.getErrorsCount();
}

int TuningClientTcpClient::sourceSorCount() const
{
	QMutexLocker l(&m_tuningSourcesMutex);

	int result = 0;

	for (const auto& it : m_tuningSources)
	{
		const TuningSource& ts = it.second;

		if (ts.state.controlisactive() == true &&
				ts.state.isreply() == true &&
				ts.state.setsor() == true)
		{
			result++;
		}
	}

	return result;
}

int TuningClientTcpClient::sourceSorCount(Hash equipmentHash) const
{
	QMutexLocker l(&m_tuningSourcesMutex);

	if (m_tuningSources.find(equipmentHash) == m_tuningSources.end())
	{
		return 0;
	}

	const TuningSource& ts = m_tuningSources.at(equipmentHash);

	if (ts.state.controlisactive() == true &&
			ts.state.isreply() == true &&
			ts.state.setsor() == true)
	{
		return 1;
	}

	return 0;
}

QString TuningClientTcpClient::getStateToolTip() const
{
	Tcp::ConnectionState connectionState = getConnectionState();
	HostAddressPort currentConnection = currentServerAddressPort();

	QString result = tr("Tuning Service connection\r\n\r\n");
	result += tr("Address (primary): %1\r\n").arg(serverAddressPort(0).addressPortStr());
	result += tr("Address (secondary): %1\r\n\r\n").arg(serverAddressPort(1).addressPortStr());
	result += tr("Address (current): %1\r\n").arg(currentConnection.addressPortStr());
	result += tr("Connection: ") + (connectionState.isConnected ? tr("established") : tr("no connection"));

	return result;
}
