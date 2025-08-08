#include <algorithm>
#include <chrono>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

#include "DiscretesLog.h"
#include "../UtilsLib/WUtils.h"

DiscretesLogWriter::DiscretesLogWriter()
{
}

DiscretesLogWriter::~DiscretesLogWriter()
{
}

void DiscretesLogWriter::start(int logTimeHours, CircularLoggerShared logger)
{
	m_logTimeHours = std::clamp(logTimeHours, 1, 10 * 24);
	m_log = logger;

	m_quitRequested = false;

	m_thread = std::thread(&DiscretesLogWriter::run, this);
}

void DiscretesLogWriter::stop()
{
	if (m_thread.joinable() == false)
	{
		return;
	}

	m_quitRequested = true;

	m_processingRequiredCondition.notify_one();

	m_thread.join();
}

void DiscretesLogWriter::pushStates(const std::vector<SimpleAppSignalState>& logStates)
{
	if (m_dbIsWorkable == false)
	{
		return;
	}

	m_logQueueMutex.lock();

	for(const SimpleAppSignalState& logState : logStates)
	{
		m_logQueue.push(logState);
	}

	m_logQueueMutex.unlock();

	m_processingRequiredCondition.notify_one();
}

void DiscretesLogWriter::run()
{
	DEBUG_LOG_WRN(m_log, "DiscretesLog: started");

	QSqlDatabase db;

	openDatabase(db);

	deleteLogOldRecords(db);

	std::unique_lock ul(m_processingRequiredConditionMutex, std::defer_lock);

	while(true)
	{
		ul.lock();

		m_processingRequiredCondition.wait_for(
							ul,
							std::crono::milliseconds(ONE_HOUR_MS),
							[this]() -> bool
							{
								m_logQueueMutex.lock();
								bool logQueueEmpty = m_logQueue.empty();
								m_logQueueMutex.unlock();

								return m_timeout || m_quitRequested || logQueueEmpty == false;
							});

		// here ul is LOCKED!

		if (m_quitRequested == true)
		{
			ul.unlock();
			break;
		}

		ul.unlock();

		if (m_timeout == true)
		{
			m_timeout = false;
			deleteLogOldRecords(db);
		}

		processLogQueue(db);
	}

	closeDatabase(db);

	DEBUG_LOG_WRN(m_log, "DiscretesLog: finished");
}

bool DiscretesLogWriter::openDatabase(QSqlDatabase& db)
{
	m_dbIsWorkable = false;

	db = QSqlDatabase::addDatabase("QSQLITE", "DiscretesLogWriter");

	QString appDataLoacation =  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

	db.setDatabaseName(appDataLoacation + "/DiscretesLog.sqlite");  // путь к файлу базы данных

	if (db.open() == true)
	{
		DEBUG_LOG_MSG(m_log, QString("DiscretesLog: database %1 opened").arg(db.databaseName()));

		bool res = checkAndCreateTables(db);

		if (res == false)
		{
			return false;
		}
	}
	else
	{
		DEBUG_LOG_ERR(m_log, "DiscretesLog: ERROR database not opened");
		return false;
	}

	m_dbIsWorkable = true;

	return m_dbIsWorkable;
}

void DiscretesLogWriter::closeDatabase(QSqlDatabase& db)
{
	if (db.isOpen() == true)
	{
		db.close();
	}
}

bool DiscretesLogWriter::checkAndCreateTables(QSqlDatabase& db)
{
	QSqlQuery q(db);

	if (execQuery(q, "PRAGMA journal_mode=WAL;") == false)
	{
		Q_ASSERT(false);
		DEBUG_LOG_ERR(m_log, "DiscretesLog: ERROR set journal_mode");
		return false;
	}

	if (db.tables().contains("Version") == false)
	{
		if (execQuery(q, "CREATE TABLE IF NOT EXISTS Version (VersionNo INTEGER, CreationDate TEXT)") == false)
		{
			DEBUG_LOG_ERR(m_log, "DiscretesLog: ERROR create Version table");
			return false;
		}
		else
		{
			DEBUG_LOG_MSG(m_log, "DiscretesLog: Version table created");
		}

		const int VERSION_NO = 1;

		QDateTime localTime = QDateTime::currentDateTime();
		localTime.setTimeZone(QTimeZone::UTC);

		if (execQuery(q, QString("INSERT INTO Version(VersionNo, CreationDate) VALUES (%1, '%2')").
						arg(VERSION_NO).
						arg(formatTime_YYYY_MM_DD(localTime.toMSecsSinceEpoch()))) == true)
		{
			DEBUG_LOG_MSG(m_log, QString("DiscretesLog: database version set to %1").arg(VERSION_NO));
		}
		else
		{
			DEBUG_LOG_ERR(m_log, "DiscretesLog: ERROR set database version!");
			return false;
		}
	}

	if (execQuery(q, "SELECT MAX(VersionNo) FROM Version") == false)
	{
		DEBUG_LOG_ERR(m_log, "DiscretesLog: ERROR get database version!");
		return false;
	}

	bool res = q.next();

	Q_ASSERT(res == true);

	m_dbVersion = q.value(0).toInt();

	DEBUG_LOG_MSG(m_log, QString("DiscretesLog: database version = %1").arg(m_dbVersion));

	//

	if (db.tables().contains("DiscretesLog") == false)
	{
		QString s = R"(
						CREATE TABLE IF NOT EXISTS DiscretesLog (
								id INTEGER PRIMARY KEY AUTOINCREMENT,
								recordTime INTEGER,

								plantTime INTEGER,
								systemTime INTEGER,
								localTime INTEGER,
								hash INTEGER,
								value INTEGER,
								flags INTEGER,

								acknowleged INTEGER DEFAULT 0,
								ackTime INTEGER DEFAULT NULL,
								ackSource TEXT DEFAULT NULL,
								ackUser TEXT DEFAULT NULL)
					)";

		if (execQuery(q, s) == false)
		{
			DEBUG_LOG_ERR(m_log, "DiscretesLog: ERROR create DiscretesLog table");
			return false;
		}
		else
		{
			DEBUG_LOG_MSG(m_log, "DiscretesLog: DiscretesLog table created");
		}
	}

	return true;
}

void DiscretesLogWriter::processLogQueue(QSqlDatabase& db)
{
	if (m_dbIsWorkable == false)
	{
		clearLogQueue();
		return;
	}

	m_requestStr.clear();

	QSqlQuery q(db);

	m_logQueueMutex.lock();

	qint64 recordTime = 0;
	bool firstRecord = false;

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

		m_requestStr.append(QString("(%1, %2, %3, %4, %5, %6, %7)").
										arg(recordTime).
										arg(s.plantTime()).
										arg(s.systemTime()).
										arg(s.localTime()).
										arg(s.hash).
										arg(s.value == 0.0 ? 0 : 1).
										arg(s.flags.all));
		m_logQueue.pop();

		if (m_requestStr.length() >= 950000)
		{
			m_logQueueMutex.unlock();

			execQuery(q, m_requestStr);

			m_requestStr.clear();

			m_logQueueMutex.lock();
		}
	}

	m_logQueueMutex.unlock();

	if (m_requestStr.isEmpty() == false)
	{
		execQuery(q, m_requestStr);

		m_requestStr.clear();
	}
}

bool DiscretesLogWriter::execQuery(QSqlQuery& q, const QString& qStr)
{
	if (q.exec(qStr) == false)
	{
		QSqlError err = q.lastError();

		DEBUG_LOG_ERR(m_log, QString("DiscretesLog: query '%1' exec ERROR - '%2'").
							 arg(qStr).arg(err.text()));

		Q_ASSERT(false);

		return false;
	}

	return true;
}

void DiscretesLogWriter::deleteLogOldRecords(QSqlDatabase& db)
{
	if (m_dbIsWorkable == false)
	{
		return;
	}

	qint64 recordTime = QDateTime::currentMSecsSinceEpoch();

	recordTime -= m_logTimeHours * ONE_HOUR_MS;

	QSqlQuery q(db);

	execQuery(q, QString("DELETE FROM DiscretesLog WHERE recordTime < %1").arg(recordTime));

	DEBUG_LOG_MSG(m_log, QString("DiscretesLog: delete %1 old records").arg(q.numRowsAffected()));
}

void DiscretesLogWriter::clearLogQueue()
{
	m_logQueueMutex.lock();

	while(m_logQueue.empty() == false)
	{
		m_logQueue.pop();
	}

	m_logQueueMutex.unlock();
}

void DiscretesLogWriter::onTimer()
{
	m_timeout = true;
	m_processingRequiredCondition.notify_one();
}
