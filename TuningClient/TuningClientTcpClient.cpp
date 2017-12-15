#include "MainWindow.h"
#include "TuningClientTcpClient.h"
#include "version.h"

TuningClientTcpClient::TuningClientTcpClient(const SoftwareInfo& softwareInfo,
											 TuningSignalManager* signalManager) :
	TuningTcpClient(softwareInfo, signalManager)
{

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

