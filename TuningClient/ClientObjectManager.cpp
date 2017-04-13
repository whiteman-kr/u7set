#include "Stable.h"
#include "MainWindow.h"
#include "ClientObjectManager.h"

TuningClientObjectManager::TuningClientObjectManager()
	:TuningObjectManager()
{

}

void TuningClientObjectManager::writeLogError(const QString& message)
{
	assert(theLogFile);
	theLogFile->writeError(message);
}

void TuningClientObjectManager::writeLogWarning(const QString& message)
{
	assert(theLogFile);
	theLogFile->writeWarning(message);
}

void TuningClientObjectManager::writeLogMessage(const QString& message)
{
	assert(theLogFile);
	theLogFile->writeMessage(message);
}

