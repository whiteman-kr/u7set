#include "MainWindow.h"
#include "TuningClientSignalManager.h"

TuningClientSignalManager::TuningClientSignalManager()
	:TuningSignalManager()
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

