#pragma once

#include "../../OnlineLib/CircularLogger.h"
#include "../../OnlineLib/BuildInfo.h"
#include "../../OnlineLib/SoftwareSettings.h"
#include "../../AppDataService/AppDataSource.h"
#include "../../AppDataService/AppDataReceiver.h"
#include "../../AppDataService/DiscretesLog.h"
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

extern std::shared_ptr<AppDataReceiver> appDataReceiver;

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

void createAndStartAppDataReceiver();
void stopAppDataReceiver();

std::shared_ptr<DiscretesLogWriter> startDiscretesLogWriter(const QString& project, const QString& equipmentID);
void stopDiscretesLogWriter(std::shared_ptr<DiscretesLogWriter> dsLogWriter);

void logMsg(const QString& msg);
