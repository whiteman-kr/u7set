#include "MainWindow.h"
#include "TuningClientTcpClient.h"
#include "version.h"

TuningClientTcpClient::TuningClientTcpClient(const SoftwareInfo& softwareInfo,
											 TuningSignalManager* signalManager) :
	TuningTcpClient(softwareInfo, signalManager)
{

}

bool TuningClientTcpClient::tuningSourceCounters(const QString& equipmentID, TuningFilterCounters* result) const
{
	if (result == nullptr)
	{
		assert(result);
		return false;
	}

	result->errorCounter = 0;
	result->sorCounter = 0;

	QMutexLocker l(&m_tuningSourcesMutex);

	for (auto it : m_tuningSources)
	{
		TuningSource& ts = it.second;

		if (ts.info.equipmentid().c_str() == equipmentID)
		{

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
	}

	return false;
}

void TuningClientTcpClient::writeLogError(const QString& message)
{
	assert(theLogFile);
	theLogFile->writeError(message);
}

void TuningClientTcpClient::writeLogWarning(const QString& message)
{
	assert(theLogFile);
	theLogFile->writeWarning(message);
}

void TuningClientTcpClient::writeLogMessage(const QString& message)
{
	assert(theLogFile);
	theLogFile->writeMessage(message);
}

