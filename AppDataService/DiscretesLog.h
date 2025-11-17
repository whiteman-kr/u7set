#pragma once

#include <queue>
#include <set>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <Network.pb.h>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "../AppSignalLib/DiscretesLogRecord.h"
#include "../AppSignalLib/SimpleAppSignalState.h"
#include "../OnlineLib/CircularLogger.h"

class DiscretesLog
{
public:
	DiscretesLog(bool isWriter);
	virtual ~DiscretesLog();

	static bool readDiscretesLogRecord(const QSqlQuery& q, DiscretesLogRecord& r);

	static QString hashToHex(Hash v);
	static Hash hexToHash(const QString& s);

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

	QSqlDatabase m_db;
	bool m_dbIsWorkable = false;
	int m_dbVersion = -1;
};

class DiscretesLogReader : private DiscretesLog, public std::enable_shared_from_this<DiscretesLogReader>
{
public:
	DiscretesLogReader(CircularLoggerShared log);
	virtual ~DiscretesLogReader();

	virtual bool openDatabase() override;
	void getDiscretesLog(Network::GetDiscretesLogReply* reply);

	void setLogChanged(bool logTruncated);

private:
	static bool selectLastNRecords(QSqlQuery& q, int N);
	static bool selectNextAfterNRecords(QSqlQuery& q, qint64 lastRecordId, int N);

private:
	QString m_dbName;
	std::atomic<bool> m_logChanged = true;		// true - is important!
	std::atomic<bool> m_logTruncated = false;

	qint64 m_firstRecordID = 0;
	qint64 m_lastRecordID = 0;

	std::queue<DiscretesLogRecord> m_logRecords;
};

class DiscretesLogWriter : private DiscretesLog
{
public:
	DiscretesLogWriter();
	virtual ~DiscretesLogWriter();

	void start(const QString& project, const QString& equipmentID, int logTimeHours, CircularLoggerShared logger);
	void stop();

	void pushStates(const std::vector<SimpleAppSignalState>& logStates);
	bool logQueueIsEmpty() const;

	void registerLogReader(const std::shared_ptr<DiscretesLogReader>& reader);
	void unregisterLogReader(const std::shared_ptr<DiscretesLogReader>& reader);

	void ackDiscretesLog(const Network::AckDiscretesLogRequest& ackRequest);

	static QString databaseName();

	// functions for testing purposes ONLY!
	//
	bool clearLog();
	bool deleteDbFiles();
	void waitWhileLogQueueIsEmpty();

	//

private:
	void run();

	virtual bool openDatabase() override;

	bool checkAndCreateTables();
	void processLogQueue();
	void ackLog(const Network::AckDiscretesLogRequest& ackRequest);
	void deleteLogOldRecords();

	void clearLogQueue();

	void notifyReaders(bool logTruncated);
	void clearReaders();

private:
	int m_logTimeHours = 1;
	std::atomic_bool m_started = false;

	std::queue<SimpleAppSignalState> m_logQueue;
	bool m_logQueueIsEmpty = true;

	std::queue<Network::AckDiscretesLogRequest> m_ackRequestQueue;

	mutable std::mutex m_condVarMutex;
	std::condition_variable m_condVar;

	std::atomic<bool> m_quitRequested = false;

	static inline QString m_databaseName;

	std::thread m_thread;

	const qint64 ONE_HOUR_MS = 3600LL * 1000LL;

	qint64 m_deleteLastTime = 0;

	//

	std::mutex m_readersMutex;

	using ReaderWPtr = std::weak_ptr<DiscretesLogReader>;

	std::set<ReaderWPtr, std::owner_less<ReaderWPtr>> m_readers;
};
