#pragma once

#include "../../OnlineLib/CircularLogger.h"
#include "../../OnlineLib/BuildInfo.h"
#include "../../OnlineLib/SoftwareSettings.h"
#include "../../AppDataService/AppDataSource.h"
#include "../../AppDataService/AppDataReceiver.h"
#include "../../AppDataService/DiscretesLog.h"
#include "../../AppSignalLib/AppSignal.h"

#include <ArchV3Lib/Core.h>

extern CircularLoggerShared logger;

extern QString buildPath;
extern QString profileName;

extern OnlineLib::BuildInfo buildInfo;
extern SoftwareSettingsSet settingsSet;

extern AppSignals appSignals;

extern std::unique_ptr<QByteArray> achInfoV3Data;

inline const QString TEST_PROJECT = "TEST_PROJECT";
inline const QString TEST_PROJECT_DB_PATTERN = "u7arch_TEST_PROJECT_%";
inline const QString COMPILER_TESTS_PROJECT_DB_PATTERN = "u7arch_compiler_tests_%";

inline ArchV3::DbConnectionInfo dbConnInfo = {.host = "127.0.0.1", .port = 5433, .user = "u7arch", .password = "P2ssw0rd"};

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

bool loadArchInfoV3Data();
void cleanup();
void dropDatabases(const QString& patternName);

bool loadConfiguration();
bool loadAppSignals();
void createAndInitSignalStates();
void logMsg(const QString& msg);
