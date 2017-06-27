#include "MainWindow.h"
#include "TuningClientSignalManager.h"
#include "version.h"

TuningClientSignalManager::TuningClientSignalManager()
	:TuningSignalManager(E::SoftwareType::TuningClient, theSettings.instanceStrId(), 0, 1, USED_SERVER_COMMIT_NUMBER)
{

}

void TuningClientSignalManager::writeLogError(const QString& message)
{
	assert(theLogFile);
	theLogFile->writeError(message);
}

void TuningClientSignalManager::writeLogWarning(const QString& message)
{
	assert(theLogFile);
	theLogFile->writeWarning(message);
}

void TuningClientSignalManager::writeLogMessage(const QString& message)
{
	assert(theLogFile);
	theLogFile->writeMessage(message);
}

