#include "MainWindow.h"
#include "TuningClientTcpClient.h"

TuningClientTcpClient::TuningClientTcpClient(const SoftwareInfo& softwareInfo,
											 TuningSignalManager* signalManager,
											 Log::LogFile* log,
											 TuningLog::TuningLog* tuningLog, TuningUserManager* userManager) :
	TuningTcpClient(softwareInfo, signalManager),
	TcpClientStatistics(this),
	m_log(log),
	m_tuningLog(tuningLog),
	m_userManager(userManager)
{
	setObjectName("TuningClientTcpClient");

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

int TuningClientTcpClient::sourceSorCount(bool* sorActive, bool* sorValid) const
{
	if (sorValid == nullptr || sorActive == nullptr)
	{
		assert(sorValid);
		assert(sorActive);
		return 0;
	}

	int result = 0;

	*sorActive = false;
	*sorValid = false;

	QMutexLocker l(&m_tuningSourcesMutex);

	for (const auto& it : m_tuningSources)
	{
		const TuningSource& ts = it.second;

		if (ts.state.controlisactive() == true)
		{
			*sorActive = true;

			if (ts.state.isreply() == true)
			{
				*sorValid = true;

				if (ts.state.setsor() == true)
				{
					result++;
				}
			}
		}
	}

	return result;
}

int TuningClientTcpClient::sourceSorCount(Hash equipmentHash, bool* sorActive, bool* sorValid) const
{
	if (sorValid == nullptr || sorActive == nullptr)
	{
		assert(sorValid);
		assert(sorActive);
		return 0;
	}

	*sorActive = false;
	*sorValid = false;

	int result = 0;

	QMutexLocker l(&m_tuningSourcesMutex);

	if (m_tuningSources.find(equipmentHash) == m_tuningSources.end())
	{
		return result;
	}

	const TuningSource& ts = m_tuningSources.at(equipmentHash);

	if (ts.state.controlisactive() == true)
	{
		*sorActive = true;

		if (ts.state.isreply() == true)
		{
			*sorValid = true;

			if (ts.state.setsor() == true)
			{
				result = 1;
			}
		}
	}

	return result;
}

QString TuningClientTcpClient::getStateToolTip() const
{
	Tcp::ConnectionState connectionState = getConnectionState();
	HostAddressPort currentConnection = currentServerAddressPort();

	QString result = tr("Tuning Service connection\n\n");
	result += tr("Address (primary): %1\n").arg(serverAddressPort(0).addressPortStr());
	result += tr("Address (secondary): %1\n\n").arg(serverAddressPort(1).addressPortStr());
	result += tr("Address (current): %1\n").arg(currentConnection.addressPortStr());
	result += tr("Connection: ") + (connectionState.isConnected ? tr("established") : tr("no connection"));

	return result;
}

bool TuningClientTcpClient::takeClientControl(QWidget* parentWidget)
{
	if (m_simulationMode == false)
	{
		if (activeTuningSourceCount() == 0)
		{
			QMessageBox::critical(parentWidget, qAppName(),	 tr("No tuning sources with control enabled found."));

			return false;
		}
	}

	if (singleLmControlMode() == true && clientIsActive() == false)
	{
		QString equipmentId = singleActiveTuningSource();

		if (QMessageBox::warning(parentWidget, qAppName(),
								 tr("Warning!\n\nCurrent client is not selected as active now.\n\nAre you sure you want to take control and activate the source %1?").arg(equipmentId),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return false;
		}

		activateTuningSourceControl(equipmentId, true, true);
	}

	return true;
}


bool TuningClientTcpClient::writingIsEnabled(const TuningSignalState& state) const
{
	if (lmStatusFlagMode() == LmStatusFlagMode::AccessKey)
	{
		return state.writingIsEnabled();
	}
	else
	{
		return true;
	}
}
