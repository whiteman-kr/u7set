#include <algorithm>
#include <chrono>
#include <optional>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QDir>

#include "DiscretesLog.h"
#include "../UtilsLib/WUtils.h"

// ------------------------------------------------------------------------------------------
//
// DiscretesLog class implementation
//
// ------------------------------------------------------------------------------------------

DiscretesLog::DiscretesLog(bool isWriter) :
	m_isWriter(isWriter)
{
}

DiscretesLog::~DiscretesLog()
{
}

bool DiscretesLog::readDiscretesLogRecord(const QSqlQuery& q, DiscretesLogRecord& r)
{
	if (q.isValid() == false)
	{
		return false;
	}

	constexpr int	COL_ID = 0,
					COL_RECORD_TIME = 1,
					COL_PLANT_TIME = 2,
					COL_SYSTEM_TIME = 3,
					COL_LOCAL_TIME = 4,
					COL_HASH = 5,
					COL_VALUE = 6,
					COL_FLAGS = 7,
					COL_ACKNOWLEDGED = 8,
					COL_ACK_TIME = 9,
					COL_ACK_SOURCE = 10,
					COL_ACK_USER = 11;

	r.recordID = q.value(COL_ID).toLongLong();
	r.recordTime = q.value(COL_RECORD_TIME).toLongLong();
	r.plantTime = q.value(COL_PLANT_TIME).toLongLong();
	r.systemTime = q.value(COL_SYSTEM_TIME).toLongLong();
	r.localTime = q.value(COL_LOCAL_TIME).toLongLong();
	r.signalHash = hexToHash(q.value(COL_HASH).toString());
	r.value = q.value(COL_VALUE).toDouble();
	r.flags = q.value(COL_FLAGS).toUInt();
	r.acknowledged = q.value(COL_ACKNOWLEDGED).toBool();
	r.ackTime = q.value(COL_ACK_TIME).toLongLong();
	r.ackSource = q.value(COL_ACK_SOURCE).toString();
	r.ackUser = q.value(COL_ACK_USER).toString();

	return true;
}

QString DiscretesLog::hashToHex(Hash v)
{
	QString s = QString::number(v, 16).toUpper();
	return s.rightJustified(16, QChar('0'));
}

Hash DiscretesLog::hexToHash(const QString& s)
{
	bool ok = false;

	Hash v = s.toULongLong(&ok, 16);

	Q_ASSERT(ok);

	return v;
}

void DiscretesLog::setLogger(CircularLoggerShared logger)
{
	TEST_PTR_RETURN(logger);
	m_log = logger;
}

void DiscretesLog::closeDatabase()
{
	if (m_db != nullptr)
	{
		const QString connName = m_db->connectionName();

		if (m_db->isOpen() == true)
		{
			m_db->close();
		}

		delete m_db;
		m_db = nullptr;

		QSqlDatabase::removeDatabase(connName);
	}
}

bool DiscretesLog::execQuery(QSqlQuery& q, const QString& qStr)
{
	if (q.exec(qStr) == false)
	{
		QSqlError err = q.lastError();

		QString errStr = QString("%1: query '%2' exec ERROR - '%3'").
						 arg(getWriterReader()).
						 arg(qStr).arg(err.text());

		DEBUG_LOG_ERR(m_log, errStr);

		Q_ASSERT(false);

		return false;
	}
	else
	{
		// DEBUG_LOG_MSG(m_log, QString("%1: query '%2' executed. RowsAffected = %3").
		// 					 arg(getWriterReader()).
		// 					 arg(qStr).arg(q.numRowsAffected()));
	}

	return true;
}

bool DiscretesLog::getDbVersion()
{
	TEST_PTR_RETURN_FALSE(m_db);

	QSqlQuery q(*m_db);

	if (execQuery(q, "SELECT MAX(VersionNo) FROM Version") == false)
	{
		DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR get database version!");
		return false;
	}

	bool res = q.next();

	if (res == false)
	{
		Q_ASSERT(false);
		return false;
	}

	m_dbVersion = q.value(0).toInt();

	DEBUG_LOG_MSG(m_log, QString("%1: database version = %2").
						 arg(getWriterReader()).arg(m_dbVersion));

	return true;
}

QString DiscretesLog::getWriterReader() const
{
	return (m_isWriter == true ? QStringLiteral("DiscretesLogWriter") : QStringLiteral("DiscretesLogReader"));
}

// ------------------------------------------------------------------------------------------
//
// DiscretesLogReader class implementation
//
// ------------------------------------------------------------------------------------------

DiscretesLogReader::DiscretesLogReader(CircularLoggerShared log) :
	DiscretesLog(false)
{
	m_dbName = QString("DiscretesLogReader_%1").arg(QUuid::createUuid().toString(QUuid::Id128));

	DEBUG_LOG_MSG(log, QString("%1 created").arg(m_dbName));

	setLogger(log);

	openDatabase();
}

DiscretesLogReader::~DiscretesLogReader()
{
	closeDatabase();
}

bool DiscretesLogReader::openDatabase()
{
	m_dbIsWorkable = false;

	m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", m_dbName));

	m_db->setDatabaseName(DiscretesLogWriter::databaseName());

	if (m_db->open() == true)
	{
		DEBUG_LOG_MSG(m_log, QString("DiscretesLogReader: database %1 opened").arg(m_db->databaseName()));

		if (m_db->tables().contains("Version") == false)
		{
			DEBUG_LOG_ERR(m_log, "DiscretesLogReader: ERROR table Varsion don't exist");
			return false;
		}

		if (m_db->tables().contains("DiscretesLog") == false)
		{
			DEBUG_LOG_ERR(m_log, "DiscretesLogReader: ERROR table DiscretesLog don't exist");
			return false;
		}

		QSqlQuery q(*m_db);

		if (execQuery(q, "PRAGMA busy_timeout = 3000") == false)
		{
			Q_ASSERT(false);
			DEBUG_LOG_ERR(m_log, "DiscretesLogReader: ERROR set busy_timeout");
			return false;
		}

		if (getDbVersion() == false)
		{
			return false;
		}
	}
	else
	{
		DEBUG_LOG_ERR(m_log, "DiscretesLogReader: ERROR database not opened");
		return false;
	}

	m_dbIsWorkable = true;

	return m_dbIsWorkable;
}

void DiscretesLogReader::getDiscretesLog(Network::GetDiscretesLogReply* reply)
{
	TEST_PTR_RETURN(reply);

	if (m_dbIsWorkable == false || m_db == nullptr)
	{
		reply->set_logisworkable(false);
		return;
	}

	reply->set_logisworkable(true);

	constexpr int MAX_RECORDS_COUNT = 5000;

	int protoRecordsCount = 0;

	reply->set_pendingrecordscount(0);

	while(m_logRecords.empty() == false)
	{
		Network::DiscretesLogRecord* dlr = reply->add_discreteslogrecord();

		m_logRecords.front().saveToProto(dlr);
		m_logRecords.pop();

		protoRecordsCount++;

		if (protoRecordsCount >= MAX_RECORDS_COUNT)
		{
			break;
		}
	}

	if (m_logChanged == false)
	{
		reply->set_pendingrecordscount(TO_INT(m_logRecords.size()));
		reply->set_logfirstrecordid(m_firstRecordID);
		return;
	}

	QSqlQuery q(*m_db);

	constexpr int N = 10000;

	if (m_lastRecordID == 0)
	{
		if (selectLastNRecords(q, N) == false)
		{
			return;
		}
	}
	else
	{
		if (selectNextAfterRecords(q, m_lastRecordID) == false)
		{
			return;
		}
	}

	DiscretesLogRecord r;

	while(q.next() == true)
	{
		if (readDiscretesLogRecord(q, r) == false)
		{
			continue;
		}

		m_lastRecordID = r.recordID;

		if (protoRecordsCount >= MAX_RECORDS_COUNT)
		{
			m_logRecords.push(r);
		}
		else
		{
			Network::DiscretesLogRecord* dlr = reply->add_discreteslogrecord();

			dlr->set_recordid(r.recordID);
			dlr->set_recordtime(r.recordTime);
			dlr->set_planttime(r.plantTime);
			dlr->set_systemtime(r.systemTime);
			dlr->set_localtime(r.localTime);
			dlr->set_signalhash(r.signalHash);
			dlr->set_value(r.value);
			dlr->set_flags(r.flags);
			dlr->set_acknowledged(r.acknowledged);
			dlr->set_acktime(r.ackTime);
			dlr->set_acksource(r.ackSource.toStdString());
			dlr->set_ackuser(r.ackUser.toStdString());

			protoRecordsCount++;
		}
	}

	reply->set_pendingrecordscount(TO_INT(m_logRecords.size()));

	//

	execQuery(q, QString("SELECT MIN(id) FROM DiscretesLog"));

	if (q.next() == true)
	{
		if (q.value(0).isNull() == false)
		{
			m_firstRecordID = q.value(0).toLongLong();
			reply->set_logfirstrecordid(m_firstRecordID);
		}
		else
		{
			// if table DiscretesLog is empty
			//
			execQuery(q, QString("SELECT IFNULL((SELECT seq FROM sqlite_sequence WHERE name='DiscretesLog'), 0) AS seqValue"));

			if (q.next() == true)
			{
				m_firstRecordID = q.value(0).toLongLong() + 1;
				reply->set_logfirstrecordid(m_firstRecordID);
			}
		}
	}

	//

	m_logChanged = false;
	m_logTruncated = false;
}

void DiscretesLogReader::setLogChanged(bool logTruncated)
{
	m_logChanged = true;
	m_logTruncated = logTruncated;
}

bool DiscretesLogReader::selectLastNRecords(QSqlQuery& q, int N)
{
	if (!q.prepare(
			"SELECT id, recordTime, plantTime, systemTime, localTime, hash, value, flags, "
			"acknowledged, ackTime, ackSource, ackUser "
			"FROM DiscretesLog "
			"WHERE id > (SELECT IFNULL(MAX(id) - ? + 1, 0) FROM DiscretesLog) "
			"ORDER BY id"))
	{
		return false;
	}

	q.bindValue(0, N);

	return q.exec();
}

bool DiscretesLogReader::selectNextAfterRecords(QSqlQuery& q, qint64 lastRecordId)
{
	if (!q.prepare(
			"SELECT id, recordTime, plantTime, systemTime, localTime, hash, value, flags, "
			"acknowledged, ackTime, ackSource, ackUser "
			"FROM DiscretesLog "
			"WHERE id > ? ORDER BY id"))
	{
		return false;
	}

	q.bindValue(0, lastRecordId);

	return q.exec();
}


// ------------------------------------------------------------------------------------------
//
// DiscretesLogWriter class implementation
//
// ------------------------------------------------------------------------------------------

DiscretesLogWriter::DiscretesLogWriter() :
	DiscretesLog(true)
{
}

DiscretesLogWriter::~DiscretesLogWriter()
{
}

void DiscretesLogWriter::start(const QString& project, const QString& equipmentID, int logTimeHours, CircularLoggerShared logger)
{
	m_started = false;

	const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

	QDir().mkpath(baseDir);

	m_databaseName = baseDir + QString("/DiscretesLog_%1_%2.sqlite").arg(project).arg(equipmentID);

	m_logTimeHours = std::clamp(logTimeHours, 1, 10 * 24);

	setLogger(logger);

	m_quitRequested = false;

	clearReaders();

	m_thread = std::thread(&DiscretesLogWriter::run, this);

	while(!m_started)
	{
		QThread::msleep(5);
	}
}

void DiscretesLogWriter::stop()
{
	clearReaders();

	if (m_thread.joinable() == false)
	{
		return;
	}

	m_quitRequested = true;

	m_condVar.notify_one();

	m_thread.join();
}

void DiscretesLogWriter::pushStates(const std::vector<SimpleAppSignalState>& logStates)
{
	if (m_dbIsWorkable == false)
	{
		return;
	}

	{
		std::lock_guard lock(m_condVarMutex);

		for(const SimpleAppSignalState& logState : logStates)
		{
			m_logQueue.push(logState);
		}
	}

	m_condVar.notify_one();
}

void DiscretesLogWriter::registerLogReader(DiscretesLogReader* reader)
{
	std::lock_guard lock(m_readersMutex);

	Q_ASSERT(m_readers.contains(reader) == false);

	m_readers.insert(reader);

	reader->setLogChanged(false);
}

void DiscretesLogWriter::unregisterLogReader(DiscretesLogReader* reader)
{
	std::lock_guard lock(m_readersMutex);

	m_readers.erase(reader);
}

void DiscretesLogWriter::ackDiscretesLog(const Network::AckDiscretesLogRequest& ackRequest)
{
	{
		std::lock_guard lock(m_condVarMutex);

		m_ackRequestQueue.push(ackRequest);
	}

	m_condVar.notify_one();
}

bool DiscretesLogWriter::clearLog()
{
	TEST_PTR_RETURN_FALSE(m_db);

	if (m_dbIsWorkable == false)
	{
		return false;
	}

	QSqlQuery q(*m_db);

	return execQuery(q, QString("DELETE FROM DiscretesLog"));
}

QString DiscretesLogWriter::databaseName()
{
	return m_databaseName;
}

void DiscretesLogWriter::run()
{
	DEBUG_LOG_WRN(m_log, "DiscretesLogWriter: started");

	openDatabase();

	deleteLogOldRecords();

	std::unique_lock ul(m_condVarMutex);

	m_started = true;

	bool logQueueEmpty = false;
	std::optional<Network::AckDiscretesLogRequest> ackRequest;

	while(true)
	{
		bool signaled = m_condVar.wait_for(
							ul,
							std::chrono::milliseconds(ONE_HOUR_MS),
							[&]() -> bool
							{
								logQueueEmpty = m_logQueue.empty();

								ackRequest.reset();

								if (m_ackRequestQueue.empty() == false)
								{
									ackRequest = m_ackRequestQueue.front();
									m_ackRequestQueue.pop();
								}

								return m_quitRequested ||
									   logQueueEmpty == false ||
									   ackRequest.has_value();
							});

		// here ul is LOCKED!

		if (m_quitRequested == true)
		{
			ul.unlock();
			break;
		}

		ul.unlock();

		if (signaled == true)
		{
			if (logQueueEmpty == false)
			{
				processLogQueue();
			}

			if (ackRequest.has_value() == true)
			{
				ackLog(ackRequest.value());
				ackRequest.reset();
			}
		}

		deleteLogOldRecords();

		ul.lock();
	}

	closeDatabase();

	DEBUG_LOG_WRN(m_log, "DiscretesLogWriter: finished");
}

bool DiscretesLogWriter::openDatabase()
{
	m_dbIsWorkable = false;

	m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "DiscretesLogWriter"));

	m_db->setDatabaseName(m_databaseName);

	if (m_db->open() == true)
	{
		DEBUG_LOG_MSG(m_log, QString("DiscretesLogWriter: database %1 opened").arg(m_db->databaseName()));

		bool res = checkAndCreateTables();

		if (res == false)
		{
			return false;
		}
	}
	else
	{
		DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR database not opened");
		return false;
	}

	m_dbIsWorkable = true;

	return m_dbIsWorkable;
}

bool DiscretesLogWriter::checkAndCreateTables()
{
	TEST_PTR_RETURN_FALSE(m_db);

	QSqlQuery q(*m_db);

	if (execQuery(q, "PRAGMA journal_mode=WAL") == false)
	{
		Q_ASSERT(false);
		DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR set journal_mode");
		return false;
	}

	if (execQuery(q, "PRAGMA wal_autocheckpoint = 1024") == false)
	{
		Q_ASSERT(false);
		DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR set wal_autocheckpoint");
		return false;

	}

	if (execQuery(q, "PRAGMA busy_timeout = 3000") == false)
	{
		Q_ASSERT(false);
		DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR set busy_timeout");
		return false;

	}

	if (execQuery(q, "PRAGMA synchronous = NORMAL") == false)
	{
		Q_ASSERT(false);
		DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR set synchronous");
		return false;

	}

	if (m_db->tables().contains("Version") == false)
	{
		if (execQuery(q, "CREATE TABLE IF NOT EXISTS Version (VersionNo INTEGER, CreationDate TEXT)") == false)
		{
			DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR create Version table");
			return false;
		}
		else
		{
			DEBUG_LOG_MSG(m_log, "DiscretesLogWriter: Version table created");
		}

		const int VERSION_NO = 1;

		QDateTime localTime = QDateTime::currentDateTime();
		localTime.setTimeZone(QTimeZone::UTC);

		if (execQuery(q, QString("INSERT INTO Version(VersionNo, CreationDate) VALUES (%1, '%2')").
						arg(VERSION_NO).
						arg(formatTime_YYYY_MM_DD(localTime.toMSecsSinceEpoch()))) == true)
		{
			DEBUG_LOG_MSG(m_log, QString("DiscretesLogWriter: database version set to %1").arg(VERSION_NO));
		}
		else
		{
			DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR set database version!");
			return false;
		}
	}

	if (getDbVersion() == false)
	{
		return false;
	}

	//

	if (m_db->tables().contains("DiscretesLog") == false)
	{
		QString s = R"(
						CREATE TABLE IF NOT EXISTS DiscretesLog (
								id INTEGER PRIMARY KEY AUTOINCREMENT,
								recordTime INTEGER,

								plantTime INTEGER,
								systemTime INTEGER,
								localTime INTEGER,
								hash TEXT,
								value INTEGER,
								flags INTEGER,

								acknowledged INTEGER DEFAULT 0,
								ackTime INTEGER DEFAULT NULL,
								ackSource TEXT DEFAULT NULL,
								ackUser TEXT DEFAULT NULL)
					)";

		if (execQuery(q, s) == false)
		{
			DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR create DiscretesLog table");
			return false;
		}
		else
		{
			DEBUG_LOG_MSG(m_log, "DiscretesLogWriter: DiscretesLog table created");
		}
	}

	if (execQuery(q, "CREATE INDEX IF NOT EXISTS idx_DiscretesLog_recordTime ON DiscretesLog(recordTime)") == false)
	{
		DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR create idx_DiscretesLog_recordTime index");
	}

	if (execQuery(q, "CREATE INDEX IF NOT EXISTS idx_DiscretesLog_plantTime ON DiscretesLog(plantTime)") == false)
	{
		DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR create idx_DiscretesLog_plantTime index");
	}

	return true;
}

void DiscretesLogWriter::processLogQueue()
{
	TEST_PTR_RETURN(m_db);

	if (m_dbIsWorkable == false)
	{
		clearLogQueue();
		return;
	}

	std::queue<SimpleAppSignalState> localQueue;

	{
		std::lock_guard lock(m_condVarMutex);

		if (m_logQueue.empty())
		{
			return;
		}

		m_logQueue.swap(localQueue);
	}

	QSqlQuery q(*m_db);

	if (!m_db->transaction())
	{
		DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR begin transaction");
	}

	const qint64 recordTime = QDateTime::currentMSecsSinceEpoch();

	const QString insertSql =
		QStringLiteral(	"INSERT INTO DiscretesLog "
						"(recordTime, plantTime, systemTime, localTime, hash, value, flags) "
						"VALUES (?, ?, ?, ?, ?, ?, ?)");

	if (!q.prepare(insertSql))
	{
		DEBUG_LOG_ERR(m_log, QString("DiscretesLogWriter: ERROR prepare insert: %1").arg(q.lastError().text()));

		if (m_db->isOpen())
		{
			m_db->rollback();
		}
		return;
	}

	int inserted = 0;
	constexpr int BATCH_COMMIT = 3000;

	while (!localQueue.empty())
	{
		const SimpleAppSignalState s = localQueue.front();
		localQueue.pop();

		q.bindValue(0, recordTime);
		q.bindValue(1, s.plantTime());
		q.bindValue(2, s.systemTime());
		q.bindValue(3, s.localTime());
		q.bindValue(4, hashToHex(s.hash));
		q.bindValue(5, (s.value == 0.0 ? 0 : 1));
		q.bindValue(6, s.flags.all);

		if (!q.exec())
		{
			DEBUG_LOG_ERR(m_log, QString("DiscretesLogWriter: INSERT failed: %1").arg(q.lastError().text()));

			if (m_db->isOpen())
			{
				m_db->rollback();
			}
			return;
		}

		inserted++;

		if ((inserted % BATCH_COMMIT) == 0)
		{
			if (!m_db->commit() || !m_db->transaction())
			{
				DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR mid-commit/transaction");
				return;
			}
		}
	}

	if (!m_db->commit())
	{
		DEBUG_LOG_ERR(m_log, "DiscretesLogWriter: ERROR commit");
		return;
	}

	if (inserted > 0)
	{
		notifyReaders(false);
	}
}

void DiscretesLogWriter::ackLog(const Network::AckDiscretesLogRequest& ackRequest)
{
	TEST_PTR_RETURN(m_db);

	if (m_dbIsWorkable == false)
	{
		return;
	}

	// QString ackSource = QString::fromStdString(ackRequest.acksource());
	// QString ackUser = QString::fromStdString(ackRequest.ackuser());
	// qint64 ackTime = QDateTime::currentMSecsSinceEpoch();

	qint64 ackUpToPlantTime = ackRequest.ackuptoplanttime();

	QSqlQuery q(*m_db);

	if (!q.prepare("DELETE FROM DiscretesLog WHERE plantTime <= ?"))
	{
		DEBUG_LOG_ERR(m_log, QString("DiscretesLogWriter: ack prepare failed: %1").arg(q.lastError().text()));
		return;
	}

	q.bindValue(0, ackUpToPlantTime);

	if (q.exec() == false)
	{
		DEBUG_LOG_ERR(m_log, QString("DiscretesLogWriter: ack DELETE failed: %1").arg(q.lastError().text()));
		return;
	}

	notifyReaders(true);
}

void DiscretesLogWriter::deleteLogOldRecords()
{
	TEST_PTR_RETURN(m_db);

	if (m_dbIsWorkable == false)
	{
		return;
	}

	qint64 now = QDateTime::currentMSecsSinceEpoch();

	if (now - m_deleteLastTime < ONE_HOUR_MS)
	{
		return;
	}

	m_deleteLastTime = now;

	qint64 recordTime = now - m_logTimeHours * ONE_HOUR_MS;

	QSqlQuery q(*m_db);

	if (execQuery(q, QString("DELETE FROM DiscretesLog WHERE recordTime < %1").arg(recordTime)) == false)
	{
		DEBUG_LOG_ERR(m_log, QString("DiscretesLogWriter: error DELETE"));
	}

	DEBUG_LOG_MSG(m_log, QString("DiscretesLogWriter: delete %1 old records").arg(q.numRowsAffected()));

	if (execQuery(q, "PRAGMA wal_checkpoint(TRUNCATE)") == false)
	{
		DEBUG_LOG_ERR(m_log, QString("DiscretesLogWriter: error PRAGMA wal_checkpoint"));
	}

	notifyReaders(true);
}

void DiscretesLogWriter::clearLogQueue()
{
	std::lock_guard lock(m_condVarMutex);

	std::queue<SimpleAppSignalState> empty;

	m_logQueue.swap(empty);
}

void DiscretesLogWriter::notifyReaders(bool logTruncated)
{
	std::lock_guard lg(m_readersMutex);

	for(DiscretesLogReader* reader : m_readers)
	{
		TEST_PTR_CONTINUE(reader);

		reader->setLogChanged(logTruncated);
	}
}

void DiscretesLogWriter::clearReaders()
{
	std::lock_guard lg(m_readersMutex);

	m_readers.clear();
}

