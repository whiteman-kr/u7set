#include <algorithm>
#include <chrono>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

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

void DiscretesLog::setLogger(CircularLoggerShared logger)
{
	TEST_PTR_RETURN(logger);
	m_log = logger;
}

void DiscretesLog::closeDatabase()
{
	if (m_db != nullptr)
	{
		if (m_db->isOpen() == true)
		{
			m_db->close();
		}

		delete m_db;
		m_db = nullptr;
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

	m_databaseName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
					 QString("/DiscretesLog_%1_%2.sqlite").arg(project).arg(equipmentID);

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

	m_requestStr.clear();

	QSqlQuery q(*m_db);

	m_condVarMutex.lock();

	qint64 recordTime = 0;
	bool firstRecord = false;
	bool notifyReadersFlag = false;

	while(m_logQueue.empty() == false)
	{
		if (m_requestStr.isEmpty())
		{
			m_requestStr = QStringLiteral("INSERT INTO DiscretesLog (recordTime, plantTime, systemTime, localTime, hash, value, flags) VALUES ");

			recordTime = QDateTime::currentMSecsSinceEpoch();
			firstRecord = true;
		}

		if (firstRecord == false)
		{
			m_requestStr.append(QStringLiteral(", "));
		}
		else
		{
			firstRecord = false;
		}

		const SimpleAppSignalState& s = m_logQueue.front();

		m_requestStr.append(QString("(%1, %2, %3, %4, '%5', %6, %7)").
										arg(recordTime).
										arg(s.plantTime()).
										arg(s.systemTime()).
										arg(s.localTime()).
										arg(QString::number(s.hash)).
										arg(s.value == 0.0 ? 0 : 1).
										arg(s.flags.all));
		m_logQueue.pop();

		if (m_requestStr.length() >= 950000)
		{
			m_condVarMutex.unlock();

			execQuery(q, m_requestStr);

			m_requestStr.clear();

			m_condVarMutex.lock();

			notifyReadersFlag = true;
		}
	}

	m_condVarMutex.unlock();

	if (m_requestStr.isEmpty() == false)
	{
		execQuery(q, m_requestStr);

		m_requestStr.clear();

		notifyReadersFlag = true;
	}

	if (notifyReadersFlag == true)
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

	m_requestStr = QString("DELETE FROM DiscretesLog WHERE plantTime<=%1").arg(ackUpToPlantTime);

	execQuery(q, m_requestStr);

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

	execQuery(q, QString("DELETE FROM DiscretesLog WHERE recordTime < %1").arg(recordTime));

	DEBUG_LOG_MSG(m_log, QString("DiscretesLogWriter: delete %1 old records").arg(q.numRowsAffected()));

	qint64 freePagesCount = getFreePagesCount();

	DEBUG_LOG_MSG(m_log, QString("DiscretesLogWriter: database free pages count - %1").arg(freePagesCount));

	notifyReaders(true);
}

qint64 DiscretesLogWriter::getFreePagesCount()
{
	TEST_PTR_RETURN_VALUE(m_db, 0);

	QSqlQuery q(*m_db);

	execQuery(q, "PRAGMA freelist_count");

	if (q.next() == true)
	{
		return q.value(0).toLongLong();
	}

	Q_ASSERT(false);

	return 0;
}
void DiscretesLogWriter::clearLogQueue()
{
	std::lock_guard lock(m_condVarMutex);

	std::queue<SimpleAppSignalState> empty;

	m_logQueue.swap(empty);
}

void DiscretesLogWriter::notifyReaders(bool logTruncated)
{
	m_readersMutex.lock();

	for(DiscretesLogReader* reader : m_readers)
	{
		TEST_PTR_CONTINUE(reader);

		reader->setLogChanged(logTruncated);
	}

	m_readersMutex.unlock();
}

void DiscretesLogWriter::clearReaders()
{
	m_readersMutex.lock();

	m_readers.clear();

	m_readersMutex.unlock();
}

// ------------------------------------------------------------------------------------------
//
// DiscretesLogReader class implementation
//
// ------------------------------------------------------------------------------------------

DiscretesLogReader::DiscretesLogReader(CircularLoggerShared log) :
	DiscretesLog(false)
{
	m_instance++;

	DEBUG_LOG_MSG(log, QString("DiscretesLogReader_%1 created!").arg(m_instance));

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

	m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", QString("DiscretesLogReader_%1").arg(m_instance)));

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
		return;
	}

	static const int MAX_RECORDS_COUNT = 5000;

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

	if (execQuery(q, QString("SELECT id, recordTime, "
									"plantTime, systemTime, localTime, hash, value, flags, "
									"acknowledged, ackTime, ackSource, ackUser "
									"FROM DiscretesLog WHERE id > %1 ORDER BY id").arg(m_lastRecordID)) == false)
	{
		return;
	}

	static const int	COL_ID = 0,
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

	DiscretesLogRecord r;

	while(q.next() == true)
	{
		if (protoRecordsCount >= MAX_RECORDS_COUNT)
		{
			m_lastRecordID = r.recordID = q.value(COL_ID).toLongLong();
			r.recordTime = q.value(COL_RECORD_TIME).toLongLong();
			r.plantTime = q.value(COL_PLANT_TIME).toLongLong();
			r.systemTime = q.value(COL_SYSTEM_TIME).toLongLong();
			r.localTime = q.value(COL_LOCAL_TIME).toLongLong();
			r.signalHash = q.value(COL_HASH).toULongLong();
			r.value = q.value(COL_VALUE).toDouble();
			r.flags = q.value(COL_FLAGS).toUInt();
			r.acknowledged = q.value(COL_ACKNOWLEDGED).toBool();
			r.ackTime = q.value(COL_ACK_TIME).toLongLong();
			r.ackSource = q.value(COL_ACK_SOURCE).toString();
			r.ackUser = q.value(COL_ACK_USER).toString();

			m_logRecords.push(r);
		}
		else
		{
			Network::DiscretesLogRecord* dlr = reply->add_discreteslogrecord();

			m_lastRecordID = q.value(COL_ID).toLongLong();

			dlr->set_recordid(m_lastRecordID);
			dlr->set_recordtime(q.value(COL_RECORD_TIME).toLongLong());
			dlr->set_planttime(q.value(COL_PLANT_TIME).toLongLong());
			dlr->set_systemtime(q.value(COL_SYSTEM_TIME).toLongLong());
			dlr->set_localtime(q.value(COL_LOCAL_TIME).toLongLong());
			dlr->set_signalhash(q.value(COL_HASH).toULongLong());
			dlr->set_value(q.value(COL_VALUE).toDouble());
			dlr->set_flags(q.value(COL_FLAGS).toUInt());
			dlr->set_acknowledged(q.value(COL_ACKNOWLEDGED).toBool());
			dlr->set_acktime(q.value(COL_ACK_TIME).toLongLong());
			dlr->set_acksource(q.value(COL_ACK_SOURCE).toString().toStdString());
			dlr->set_ackuser(q.value(COL_ACK_USER).toString().toStdString());

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
				m_firstRecordID = q.value(0).toLongLong();
				reply->set_logfirstrecordid(m_firstRecordID);
			}
		}
	}

	qDebug() << "=========== Send Discretes log records" << reply->discreteslogrecord_size();

	//

	m_logChanged = false;
	m_logTruncated = false;
}

void DiscretesLogReader::setLogChanged(bool logTruncated)
{
	m_logChanged = true;
	m_logTruncated = logTruncated;
}
