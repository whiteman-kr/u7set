#include "ArchWriteThread.h"

const char* ArchWriteThreadWorker::ARCH_DB_PREFIX = "u7arch_";
const char* ArchWriteThreadWorker::LONG_TERM_TABLE_PREFIX = "lt_";
const char* ArchWriteThreadWorker::SHORT_TERM_TABLE_PREFIX = "st_";

ArchWriteThreadWorker::ArchWriteThreadWorker(const QString& projectID,
											 AppSignalStatesQueue& saveStatesQueue,
											 CircularLoggerShared logger) :
	m_projectID(projectID),
	m_saveStatesQueue(saveStatesQueue),
	m_logger(logger),
	m_timer(this)
{
}

void ArchWriteThreadWorker::onThreadStarted()
{
	DEBUG_LOG_MSG(m_logger, "ArchWriteThread is started");

	connect(&m_timer, &QTimer::timeout, this, &ArchWriteThreadWorker::onTimer);
	connect(&m_saveStatesQueue, &AppSignalStatesQueue::queueNotEmpty, this, &ArchWriteThreadWorker::onSaveStatesQueueIsNotEmpty);

	m_timer.setInterval(1000);
	m_timer.start();

	tryConnectToDb();
}

void ArchWriteThreadWorker::onThreadFinished()
{
	disconnectFromDb();

	DEBUG_LOG_MSG(m_logger, "ArchWriteThread is finished");
}

void ArchWriteThreadWorker::tryConnectToDb()
{
	if (m_db.isOpen() == true)
	{
		return;
	}

	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "infoArchConnection");

	if (db.lastError().isValid() == true)
	{
		DEBUG_LOG_ERR(m_logger, db.lastError().text());
		return;
	}

	db.setHostName("127.0.0.1");
	db.setPort(5432);
	db.setDatabaseName("postgres");
	db.setUserName("u7arch");
	db.setPassword("arch876436");

	bool result = db.open();

	if (result == false)
	{
		DEBUG_LOG_ERR(m_logger, m_db.lastError().text());
		return;
	}

	if (archiveIsExists(db) == false)
	{
		bool res = createArchive(db);

		if (res == false)
		{
			return;
		}
	}

	db.close();		// close connection to 'postgress' database

	// open connection to projectArchive database

	m_db = QSqlDatabase::addDatabase("QPSQL", "writeArchConnection");

	if (m_db.lastError().isValid() == true)
	{
		DEBUG_LOG_ERR(m_logger, m_db.lastError().text());
		return;
	}

	m_db.setHostName("127.0.0.1");
	m_db.setPort(5432);
	m_db.setDatabaseName(projectArchiveDbName());
	m_db.setUserName("u7arch");
	m_db.setPassword("arch876436");

	result = m_db.open();

	if (result == false)
	{
		DEBUG_LOG_ERR(m_logger, m_db.lastError().text());
		return;
	}

	getSignalsTablesList();
}

bool ArchWriteThreadWorker::archiveIsExists(QSqlDatabase& db)
{
	if (db.isOpen() == false)
	{
		assert(false);
		return false;
	}

	QString projectArchDbName = projectArchiveDbName();

	QString queryStr = QString("SELECT datname FROM pg_database WHERE datname = '%1' ORDER BY datname;").
			arg(projectArchDbName);

	QSqlQuery query(db);

	bool result = query.exec(queryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(m_logger, query.lastError().text());
		db.close();
		return false;
	}

	while (query.next() == true)
	{
		QString databaseName = query.value(0).toString();

		if (databaseName == projectArchDbName)
		{
			DEBUG_LOG_MSG(m_logger, QString("Archive database '%1' is exists").arg(projectArchDbName));
			return true;
		}
	}

	DEBUG_LOG_WRN(m_logger, QString("Archive database '%1' is not exists").arg(projectArchDbName));

	return false;
}

bool ArchWriteThreadWorker::createArchive(QSqlDatabase& db)
{
	if (db.isOpen() == false)
	{
		assert(false);
		return false;
	}

	QString projectArchDbName = projectArchiveDbName();

	QString queryStr = QString("CREATE DATABASE %1 WITH ENCODING = 'UTF8' CONNECTION LIMIT = -1;").
							arg(projectArchDbName);

	QSqlQuery query(db);

	bool result = query.exec(queryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(m_logger, query.lastError().text());
		db.close();
		return false;
	}

	DEBUG_LOG_MSG(m_logger, QString("Archive database '%1' is successful created").arg(projectArchDbName));

	return true;
}


bool ArchWriteThreadWorker::getSignalsTablesList()
{
	if (m_db.isOpen() == false)
	{
		assert(false);
		return false;
	}

	m_longTermTables.clear();
	m_shortTermTables.clear();

	QString queryStr = QString(	"SELECT table_name "
								"FROM information_schema.tables "
								"WHERE table_schema = 'public' AND (table_name LIKE 'lt\\_%' OR table_name LIKE 'st\\_%') "
								"ORDER BY table_schema,table_name");

	QSqlQuery query(m_db);

	bool result = query.exec(queryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(m_logger, query.lastError().text());
		m_db.close();
		return false;
	}

	while (query.next() == true)
	{
		QString tableName = query.value(0).toString();

		tableName = tableName.toLower();

		if (tableName.startsWith(LONG_TERM_TABLE_PREFIX) == true)
		{
			appendTable(tableName, SignalStatesTableType::LongTerm);
			continue;
		}

		if (tableName.startsWith(SHORT_TERM_TABLE_PREFIX) == true)
		{
			appendTable(tableName, SignalStatesTableType::ShortTerm);
			continue;
		}

		assert(false);
	}

	return true;
}

void ArchWriteThreadWorker::appendTable(const QString& tableName, SignalStatesTableType tableType)
{
	QString hashStr = tableName.mid(3);			// crop tableName after 'lt_' or 'st_';

	bool result = false;

	Hash signalHash = hashStr.toULongLong(&result, 16);

	if (result == false)
	{
		assert(false);
		return;
	}

	switch(tableType)
	{
	case SignalStatesTableType::LongTerm:
		m_longTermTables.insert(signalHash, tableName);
		break;

	case SignalStatesTableType::ShortTerm:
		m_shortTermTables.insert(signalHash, tableName);
		break;

	default:
		assert(false);
	}
}

bool ArchWriteThreadWorker::createTableIfNotExists(Hash signalHash)
{
	bool result = true;

	if (m_longTermTables.contains(signalHash) == false)
	{
		result &= createSignalStatesTable(signalHash, SignalStatesTableType::LongTerm);
	}

	if (m_shortTermTables.contains(signalHash) == false)
	{
		result &= createSignalStatesTable(signalHash, SignalStatesTableType::ShortTerm);
	}

	return result;
}

bool ArchWriteThreadWorker::createSignalStatesTable(Hash signalHash, SignalStatesTableType tableType)
{
	if (m_db.isOpen() == false)
	{
		return false;
	}

	QString tableName = getTableName(signalHash, tableType);

	if (tableName.isEmpty() == true)
	{
		return false;
	}

	QString queryStr = QString("CREATE TABLE public.%1("
								"uid bigint, "
								"state double precision, "
								"flags integer, "
								"planttime bigint, "
								"systime bigint, "
								"loctime bigint)").arg(tableName);

	QSqlQuery query(m_db);

	bool result = query.exec(queryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(m_logger, query.lastError().text());
		m_db.close();
		return false;
	}

	// table is created successfully

	switch(tableType)
	{
	case SignalStatesTableType::LongTerm:
		m_longTermTables.insert(signalHash, tableName);
		break;

	case SignalStatesTableType::ShortTerm:
		m_shortTermTables.insert(signalHash, tableName);
		break;

	default:
		assert(false);
	}

	return true;
}

QString ArchWriteThreadWorker::getTableName(Hash signalHash, SignalStatesTableType tableType)
{
	QString tableName;

	switch(tableType)
	{
	case SignalStatesTableType::LongTerm:
		tableName = LONG_TERM_TABLE_PREFIX;
		break;

	case SignalStatesTableType::ShortTerm:
		tableName = SHORT_TERM_TABLE_PREFIX;
		break;

	default:
		assert(false);
	}

	if (tableName.isEmpty() == true)
	{
		return tableName;
	}


	tableName += QString().setNum(signalHash, 16);

	return tableName;
}


void ArchWriteThreadWorker::disconnectFromDb()
{
	if (m_db.isOpen() == true)
	{
		m_db.close();
	}
}

void ArchWriteThreadWorker::writeStatesToArchive()
{
	if (m_db.isOpen() == false)
	{
		return;
	}

	if (m_saveStatesQueue.isEmpty() == true)
	{
		return;
	}

	int count = 0;

	int written = 0;

	do
	{
		SimpleAppSignalState state;

		bool res = m_saveStatesQueue.pop(&state);

		if (res == false)
		{
			break;
		}

		res = createTableIfNotExists(state.hash);

		if (res == false)
		{
			count++;
			continue;
		}

		written++;

		count++;
	}
	while(count < 500);

	qDebug() << "Write states " << written;
}

QString ArchWriteThreadWorker::projectArchiveDbName()
{
	QString dbName = QString(ARCH_DB_PREFIX) + m_projectID;

	dbName = dbName.toLower();

	return dbName;
}

void ArchWriteThreadWorker::onTimer()
{
	if (m_db.isOpen() == false)
	{
		tryConnectToDb();
	}

	writeStatesToArchive();
}

void ArchWriteThreadWorker::onSaveStatesQueueIsNotEmpty()
{
	writeStatesToArchive();
}

ArchWriteThread::ArchWriteThread(const QString& projectID, AppSignalStatesQueue& saveStatesQueue, CircularLoggerShared logger)
{
	ArchWriteThreadWorker* worker = new ArchWriteThreadWorker(projectID, saveStatesQueue, logger);

	addWorker(worker);
}
