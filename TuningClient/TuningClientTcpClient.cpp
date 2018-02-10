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

