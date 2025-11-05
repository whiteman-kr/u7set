#pragma once

#include "../OnlineLib/CircularLogger.h"
#include "AppDataSource.h"

namespace AppDataSrvTools
{

bool readAppDataSources(const QByteArray& fileData,
						const QString& profile,
						AppDataSources& appDataSources,
						CircularLoggerShared log);

bool readAppSignals(const QByteArray& fileData, AppSignals& appSignals);

void createAndInitSignalStates(const AppSignals& appSignals,
							   DynamicAppSignalStates& appSignalStates,
							   int autoArchivingGroupsCount);

}
