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

class LoggerGuard
{
public:
	LoggerGuard()
	{
		Q_ASSERT(logger == nullptr);
		logger = std::make_shared<CircularLogger>();
		LOGGER_INIT(logger, QString(), "");
		logger->setLogCodeInfo(false);
	}

	~LoggerGuard()
	{
		LOGGER_SHUTDOWN(logger);
	}
};

bool isGTestDeathChild(const QStringList& args);

bool loadConfiguration();
bool loadAppDataSources();
bool loadAppSignals();
void createAndInitSignalStates();

void logMsg(const QString& msg);
