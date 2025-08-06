#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

#include "DiscretesLog.h"
#include "../UtilsLib/WUtils.h"

DiscretesLog::DiscretesLog()
{
}

DiscretesLog::~DiscretesLog()
{
}

void DiscretesLog::start(CircularLoggerShared logger)
{
	m_log = logger;

	m_quitRequested = false;

	m_thread = std::thread(&DiscretesLog::run, this);
}

void DiscretesLog::stop()
{
	if (m_thread.joinable() == false)
	{
		return;
	}

	m_quitRequested = true;

	m_processingRequiredCondition.notify_one();

	m_thread.join();
}

void DiscretesLog::pushStates(const std::vector<SimpleAppSignalState>& logStates)
{
	m_logQueueMutex.lock();

	for(const SimpleAppSignalState& logState : logStates)
	{
		m_logQueue.push(logState);
	}

	m_logQueueMutex.unlock();

	m_processingRequiredCondition.notify_one();
}

void DiscretesLog::run()
{
	DEBUG_LOG_WRN(m_log, "DiscretesLog started");

	QSqlDatabase db;

	openDatabase(db);

	std::unique_lock ul(m_processingRequiredConditionMutex, std::defer_lock);

	while(true)
	{
		ul.lock();

		m_processingRequiredCondition.wait(ul, [this]() -> bool
						   {
								m_logQueueMutex.lock();
								bool logQueueEmpty = m_logQueue.empty();
								m_logQueueMutex.unlock();

								return m_quitRequested || logQueueEmpty == false;
						   });

		// here ul is LOCKED!

		if (m_quitRequested == true)
		{
			ul.unlock();
			break;
		}

		ul.unlock();
	}

	closeDatabase(db);

	DEBUG_LOG_WRN(m_log, "DiscretesLog finished");
}

bool DiscretesLog::openDatabase(QSqlDatabase& db)
{
	db = QSqlDatabase::addDatabase("QSQLITE");

	QString appDataLoacation =  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

	db.setDatabaseName(appDataLoacation + "/DiscretesLog.sqlite");  // путь к файлу базы данных

	if (db.open() == true)
	{
		DEBUG_LOG_MSG(m_log, QString("DiscretesLog database %1 opened").arg(db.databaseName()));

		bool res = checkAndCreateTables(db);

		if (res == false)
		{
			return false;
		}
	}
	else
	{
		DEBUG_LOG_ERR(m_log, "DiscretesLog database NOT opened");
	}

	return db.isOpen();
}

void DiscretesLog::closeDatabase(QSqlDatabase& db)
{
	if (db.isOpen() == true)
	{
		db.close();
	}
}

bool DiscretesLog::checkAndCreateTables(QSqlDatabase& db)
{
	if (db.tables().contains("Version") == false)
	{
		QSqlQuery q(db);

		if (q.exec("CREATE TABLE IF NOT EXISTS Version (VersionNo INTEGER, CreationDate TEXT)") == false)
		{
			DEBUG_LOG_ERR(m_log, "Error create Version table");
			return false;
		}
		else
		{
			DEBUG_LOG_MSG(m_log, "Version table created");
		}

		const int VERSION_NO = 1;

		QDateTime localTime = QDateTime::currentDateTime();
		localTime.setTimeZone(QTimeZone::UTC);

		if (q.exec(QString("INSERT INTO Version(VersionNo, CreationDate) VALUES (%1, '%2')").
						arg(VERSION_NO).
						arg(formatTime_YYYY_MM_DD(localTime.toMSecsSinceEpoch()))) == true)
		{
			DEBUG_LOG_MSG(m_log, QString("DiscretesLog database version set to %1").arg(VERSION_NO));
		}
		else
		{
			DEBUG_LOG_ERR(m_log, "Error set DiscretesLog database version!");
			return false;
		}
	}

	QSqlQuery q(db);

	if (q.exec("SELECT MAX(VersionNo) FROM Version") == false)
	{
		DEBUG_LOG_ERR(m_log, "Error get DiscretesLog database version!");
		return false;
	}

	q.next();

	m_dbVersion = q.value("VersionNo").toInt();

	DEBUG_LOG_MSG(m_log, QString("DiscretesLog database version = %1").arg(m_dbVersion));

	//

	if (db.tables().contains("DiscretesLog") == false)
	{
		QString s = R"(
						CREATE TABLE IF NOT EXISTS DiscretesLog (
								id INTEGER PRIMARY KEY AUTOINCREMENT,
								recordTime INTEGER,
								hash INTEGER,
								value INTEGER,
								plantTime INTEGER,
								serverTime INTEGER,
								acknowleged INTEGER,
								ackTime INTEGER,
								ackSource TEXT)
					)";

		if (q.exec(s) == false)
		{
			DEBUG_LOG_ERR(m_log, "Error create DiscretesLog table");
			return false;
		}
		else
		{
			DEBUG_LOG_MSG(m_log, "DiscretesLog table created");
		}
	}

	return true;
}



