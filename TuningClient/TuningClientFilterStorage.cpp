#include "Stable.h"
#include "MainWindow.h"
#include "TuningClientFilterStorage.h"

TuningClientFilterStorage::TuningClientFilterStorage()
{

}

void TuningClientFilterStorage::writeLogError(const QString& message)
{
	theLogFile->writeError(message);
}

void TuningClientFilterStorage::writeLogWarning(const QString& message)
{
	theLogFile->writeWarning(message);
}

void TuningClientFilterStorage::writeLogMessage(const QString& message)
{
	theLogFile->writeMessage(message);
}
