#pragma once

#include "../../OnlineLib/CircularLogger.h"
#include "../../OnlineLib/BuildInfo.h"
#include "../../OnlineLib/SoftwareSettings.h"
#include "../../AppDataService/AppDataSource.h"
#include "../../AppSignalLib/AppSignal.h"

extern CircularLoggerShared logger;

extern QString buildPath;
extern QString profileName;

extern OnlineLib::BuildInfo buildInfo;
extern SoftwareSettingsSet settingsSet;
extern AppDataServiceSettings appDataSrvSettings;

extern AppDataSources appDataSources;
extern AppSignals appSignals;

extern DynamicAppSignalStates appSignalStates;

//

bool loadConfiguration();
bool loadAppDataSources();
bool loadAppSignals();
void createAndInitSignalStates();
