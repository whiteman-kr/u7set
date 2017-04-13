#include "Stable.h"
#include "MainWindow.h"
#include "ClientFilterStorage.h"

ClientFilterStorage::ClientFilterStorage()
{

}

void ClientFilterStorage::writeLogError(const QString& message)
{
	theLogFile->writeError(message);
}

void ClientFilterStorage::writeLogWarning(const QString& message)
{
	theLogFile->writeWarning(message);
}

void ClientFilterStorage::writeLogMessage(const QString& message)
{
	theLogFile->writeMessage(message);
}
