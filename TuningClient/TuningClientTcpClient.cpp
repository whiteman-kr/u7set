#include "MainWindow.h"
#include "TuningClientTcpClient.h"
#include "version.h"

TuningClientTcpClient::TuningClientTcpClient(QString equipmentID,
											 int majorVersion,
											 int minorVersion,
											 int commitNo,
											 TuningSignalManager* signalManager)
	:TuningTcpClient(E::SoftwareType::TuningClient, equipmentID, majorVersion, minorVersion, commitNo, signalManager)
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

