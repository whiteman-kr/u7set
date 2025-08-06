#pragma once

#include <queue>

#include "../AppSignalLib/SimpleAppSignalState.h"
#include "../UtilsLib/SimpleMutex.h"
#include "../OnlineLib/CircularLogger.h"

class QSqlDatabase;

class DiscretesLog
{
public:
	DiscretesLog();
	virtual ~DiscretesLog();

	void start(CircularLoggerShared logger);
	void stop();

	void pushStates(const std::vector<SimpleAppSignalState>& logStates);

private:
	void run();

	bool openDatabase(QSqlDatabase& db);
	void closeDatabase(QSqlDatabase& db);
	bool checkAndCreateTables(QSqlDatabase& db);

private:
	CircularLoggerShared m_log;

	SimpleMutex m_logQueueMutex;
	std::queue<SimpleAppSignalState> m_logQueue;

	std::mutex m_processingRequiredConditionMutex;
	std::condition_variable m_processingRequiredCondition;

	std::atomic<bool> m_quitRequested = false;

	int m_dbVersion = -1;

	std::thread m_thread;
};
