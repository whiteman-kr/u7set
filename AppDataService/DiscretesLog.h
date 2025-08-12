#pragma once

#include <queue>

#include <Network.pb.h>

#include "../AppSignalLib/SimpleAppSignalState.h"
#include "../UtilsLib/SimpleMutex.h"
#include "../OnlineLib/CircularLogger.h"
#include "../OnlineLib/CircularLogger.h"

class QSqlDatabase;
class QSqlQuery;

class DiscretesLogWriter
{
public:
	DiscretesLogWriter();
	virtual ~DiscretesLogWriter();

	void start(int logTimeHours, CircularLoggerShared logger);
	void stop();

	void pushStates(const std::vector<SimpleAppSignalState>& logStates);

	static QString databaseName();

private:
	void run();

	bool openDatabase();
	void closeDatabase();
	bool checkAndCreateTables();
	void processLogQueue();
	bool execQuery(QSqlQuery& q, const QString& qStr);
	void deleteLogOldRecords();
	qint64 getFreePagesCount();

	void clearLogQueue();

private:
	int m_logTimeHours = 1;
	CircularLoggerShared m_log;

	SimpleMutex m_logQueueMutex;
	std::queue<SimpleAppSignalState> m_logQueue;
	QString m_requestStr;

	std::mutex m_processingRequiredConditionMutex;
	std::condition_variable m_processingRequiredCondition;

	std::atomic<bool> m_quitRequested = false;

	static inline QString m_databaseName;
	QSqlDatabase* m_db = nullptr;
	bool m_dbIsWorkable = false;
	int m_dbVersion = -1;

	std::thread m_thread;

	//const int ONE_HOUR_MS = 60 * 60 * 1000;
	const int ONE_HOUR_MS = 60 * 1000;

	qint64 m_deleteLastTime = 0;
};

struct DiscretesLogRecord
{
	qint64 recordID;
	qint64 recordTime;
	qint64 plantTime;
	qint64 systemTime;
	qint64 localTime;
	Hash signalHash;
	double value;
	quint32 flags;
	bool acknowledged;
	qint64 ackTime;
	QString ackSource;
	QString ackUser;

	void saveToProto(Network::DiscretesLogRecord* dlr);
	void loadFromProto(const Network::DiscretesLogRecord& dlr);
};

class DiscretesLogReader
{
public:
	DiscretesLogReader(CircularLoggerShared log);
	virtual ~DiscretesLogReader();

	bool openDatabase();
	void getDiscretesLog(Network::GetDiscretesLogReply* reply);

private:
	bool execQuery(QSqlQuery& q, const QString& qStr);

private:
	CircularLoggerShared m_log;

	static inline int m_instance = 0;

	QSqlDatabase* m_db = nullptr;
	bool m_dbIsWorkable = false;

	qint64 m_lastRecordID = 0;

	std::queue<DiscretesLogRecord> m_logRecords;
};
