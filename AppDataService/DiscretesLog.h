#pragma once

#include <queue>

#include "../AppSignalLib/SimpleAppSignalState.h"
#include "../UtilsLib/SimpleMutex.h"
#include "../OnlineLib/CircularLogger.h"

class QSqlDatabase;
class QSqlQuery;

class DiscretesLogWriter : public QObject
{
	Q_OBJECT

public:
	DiscretesLogWriter();
	virtual ~DiscretesLogWriter();

	void start(int logTimeHours, CircularLoggerShared logger);
	void stop();

	void pushStates(const std::vector<SimpleAppSignalState>& logStates);

private:
	void run();

	bool openDatabase(QSqlDatabase& db);
	void closeDatabase(QSqlDatabase& db);
	bool checkAndCreateTables(QSqlDatabase& db);
	void processLogQueue(QSqlDatabase& db);
	bool execQuery(QSqlQuery& q, const QString& qStr);
	void deleteLogOldRecords(QSqlDatabase& db);

	void clearLogQueue();

private slots:
	void onTimer();

private:
	int m_logTimeHours = 1;
	CircularLoggerShared m_log;

	SimpleMutex m_logQueueMutex;
	std::queue<SimpleAppSignalState> m_logQueue;
	QString m_requestStr;

	std::mutex m_processingRequiredConditionMutex;
	std::condition_variable m_processingRequiredCondition;

	std::atomic<bool> m_quitRequested = false;
	bool m_timeout = false;

	bool m_dbIsWorkable = false;
	int m_dbVersion = -1;

	std::thread m_thread;

	//const int ONE_HOUR_MS = 60 * 60 * 1000;
	const int ONE_HOUR_MS = 60 * 1000;
};
