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

bool TuningClientTcpClient::tuningSourceCounters(const Hash& equipmentHash, TuningFilterCounters* result) const
{
	if (result == nullptr)
	{
		assert(result);
		return false;
	}

	result->errorCounter = 0;
	result->sorCounter = 0;

	TuningSource ts;

	if (tuningSourceInfoByHash(equipmentHash, &ts) == false)
	{
		return false;
	}

	if (ts.state.isreply() == true && ts.state.fotipflagsetsor() > 0)
	{
		result->sorCounter++;
	}

	if (ts.state.isreply() == true && ts.state.errfotipuniqueid() > 0)
	{
		result->errorCounter++;
	}

	return true;
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

