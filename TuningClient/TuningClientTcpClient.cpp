#include "MainWindow.h"
#include "TuningClientTcpClient.h"

TuningClientTcpClient::TuningClientTcpClient(const SoftwareInfo& softwareInfo,
											 const QString& tuningServiceId,
											 int singleLmControlMode,
											 TuningSignalManager& signalManager,
											 Log::LogFile* log,
											 TuningLog::TuningLog* tuningLog,
											 TuningUserManager& userManager) :
	TuningTcpClient(softwareInfo, tuningServiceId, singleLmControlMode, signalManager),
	TcpClientStatistics(this),
	m_log(log),
	m_tuningLog(tuningLog),
	m_userManager(userManager)
{
	setObjectName(tuningServiceId);

	assert(m_log);
	assert(m_tuningLog);
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
	m_tuningLog->write(param, oldValue, newValue, m_userManager.loggedInUser());
}

void TuningClientTcpClient::writeLogSignalChange(const QString& message)
{
	m_tuningLog->write(message, m_userManager.loggedInUser());
}


int TuningClientTcpClient::sourceErrorCount() const
{
	QReadLocker l(&m_tuningSourcesLock);

	int result = 0;

	for (const auto& it : m_tuningSources)
	{
		const TuningSource& ts = it.second;

		for (int i = 0; i < ts.statesCount(); i++)
		{
			if (ts.state(i).isreply() == false && ts.state(i).controlisactive() == true)
			{
				// Control but not valid
				//
				result++;
			}
			else
			{
				result += ts.getErrorsCount(i);
			}
		}
	}

	return result;
}

int TuningClientTcpClient::sourceErrorCount(Hash equipmentHash) const
{
	QReadLocker l(&m_tuningSourcesLock);

	if (m_tuningSources.find(equipmentHash) == m_tuningSources.end())
	{
		return 0;
	}

	const TuningSource& ts = m_tuningSources.at(equipmentHash);

	int result = 0;

	for (int i = 0; i < ts.statesCount(); i++)
	{
		if (ts.state(i).isreply() == false && ts.state(i).controlisactive() == true)
		{
			result++;
		}
		else
		{
			result += ts.getErrorsCount(i);
		}
	}

	return result;
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

	QReadLocker l(&m_tuningSourcesLock);

	for (const auto& it : m_tuningSources)
	{
		const TuningSource& ts = it.second;

		bool sorIsSet = false;

		for (int i = 0; i < ts.statesCount(); i++)
		{
			auto state = ts.state(i);

			if (state.controlisactive() == true)
			{
				*sorActive = true;

				if (state.isreply() == true)
				{
					*sorValid = true;

					if (state.setsor() == true)
					{
						sorIsSet = true;
					}
				}
			}
		}

		if (sorIsSet == true)
		{
			result++;
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

	QReadLocker l(&m_tuningSourcesLock);

	if (m_tuningSources.find(equipmentHash) == m_tuningSources.end())
	{
		return result;
	}

	const TuningSource& ts = m_tuningSources.at(equipmentHash);

	for (int i = 0; i < ts.statesCount(); i++)
	{
		auto state = ts.state(i);

		if (state.controlisactive() == true)
		{
			*sorActive = true;

			if (state.isreply() == true)
			{
				*sorValid = true;

				if (state.setsor() == true)
				{
					result = 1;
				}
			}
		}
	}

	return result;
}

QString TuningClientTcpClient::getStateToolTip() const
{
	HostAddressPort currentConnection = currentServerAddressPort();

	QString result = tr("ID: %1\n").arg(tuningServiceId());
	result += tr("Address (primary): %1\n").arg(serverAddressPort(0).addressPortStr());
	result += tr("Address (secondary): %1\n").arg(serverAddressPort(1).addressPortStr());
	result += tr("Address (current): %1").arg(currentConnection.addressPortStr());

	return result;
}

std::vector<Hash> TuningClientTcpClient::getProcessedHashes(const std::vector<Hash>& hashes)
{
	std::vector<Hash> result;
	result.reserve(hashes.size());

	QReadLocker l(&m_signalHashesLock);

	for (Hash hash : hashes)
	{
		if (m_signalHashesSet.find(hash) != m_signalHashesSet.end())
		{
			result.push_back(hash);
		}
	}

	return result;
}
