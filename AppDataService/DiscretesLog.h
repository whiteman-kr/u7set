#pragma once

#include <queue>

#include <Network.pb.h>

#include "../AppSignalLib/DiscretesLogRecord.h"
#include "../AppSignalLib/SimpleAppSignalState.h"
#include "../UtilsLib/SimpleMutex.h"
#include "../OnlineLib/CircularLogger.h"

class QSqlDatabase;
class QSqlQuery;

class DiscretesLog
{
public:
	DiscretesLog(bool isWriter);
	virtual ~DiscretesLog();

protected:
	void setLogger(CircularLoggerShared logger);

	virtual bool openDatabase() = 0;
	virtual void closeDatabase();

	bool execQuery(QSqlQuery& q, const QString& qStr);

	bool getDbVersion();

	QString getWriterReader() const;

protected:
	bool m_isWriter = false;
	CircularLoggerShared m_log;

	QSqlDatabase* m_db = nullptr;
	bool m_dbIsWorkable = false;
	int m_dbVersion = -1;
};

class DiscretesLogReader : private DiscretesLog
{
public:
	DiscretesLogReader(CircularLoggerShared log);
	virtual ~DiscretesLogReader();

	virtual bool openDatabase() override;
	void getDiscretesLog(Network::GetDiscretesLogReply* reply);

	void setLogChanged();

private:
	CircularLoggerShared m_log;

	static inline int m_instance = 0;

	std::atomic<bool> m_logChanged = true;		// true - is important!

	qint64 m_firstRecordID = 0;
	qint64 m_lastRecordID = 0;

	std::queue<DiscretesLogRecord> m_logRecords;
};

class DiscretesLogWriter : private DiscretesLog
{
public:
	DiscretesLogWriter();
	virtual ~DiscretesLogWriter();

	void start(const QString& equipmentID, int logTimeHours, CircularLoggerShared logger);
	void stop();

	void pushStates(const std::vector<SimpleAppSignalState>& logStates);

	void registerLogReader(DiscretesLogReader* reader);
	void unregisterLogReader(DiscretesLogReader* reader);

	static QString databaseName();

private:
	void run();

	virtual bool openDatabase() override;

	bool checkAndCreateTables();
	void processLogQueue();
	void deleteLogOldRecords();
	qint64 getFreePagesCount();

	void clearLogQueue();

	void notifyReaders();
	void clearReaders();

private:
	int m_logTimeHours = 1;

	SimpleMutex m_logQueueMutex;
	std::queue<SimpleAppSignalState> m_logQueue;
	QString m_requestStr;

	std::mutex m_processingRequiredConditionMutex;
	std::condition_variable m_processingRequiredCondition;

	std::atomic<bool> m_quitRequested = false;

	static inline QString m_databaseName;

	std::thread m_thread;

	const int ONE_HOUR_MS = 60 * 60 * 1000;

	qint64 m_deleteLastTime = 0;

	//
	SimpleMutex m_readersMutex;
	std::set<DiscretesLogReader*> m_readers;
};
