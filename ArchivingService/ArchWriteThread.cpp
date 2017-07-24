#include "ArchWriteThread.h"
#include "ArchivingService.h"

#include <chrono>

const char* ArchWriteThreadWorker::ARCH_DB_PREFIX = "u7arch_";
const char* ArchWriteThreadWorker::LONG_TERM_TABLE_PREFIX = "lt_";
const char* ArchWriteThreadWorker::SHORT_TERM_TABLE_PREFIX = "st_";

const StringPair ArchWriteThreadWorker::m_upgradeFiles[] =
{
	StringPair(":/Upgrade0001.sql", "Initial upgrade"),
};


ArchWriteThreadWorker::ArchWriteThreadWorker(const QString& projectID,
											 AppSignalStatesQueue& saveStatesQueue,
											 const ArchSignals &archSignals,
											 CircularLoggerShared logger) :
	m_projectID(projectID),
	m_saveStatesQueue(saveStatesQueue),
	m_archSignals(archSignals),
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

	bool dbJustCreated = false;

	if (databaseIsExists(db) == false)
	{
		bool res = createDatabase(db);

		if (res == false)
		{
			return;
		}

		dbJustCreated = true;
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

	if (dbJustCreated == true)
	{
		initDatabase();
	}

	upgradeDatabase();

	getSignalsTablesList();
}

bool ArchWriteThreadWorker::databaseIsExists(QSqlDatabase& db)
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

bool ArchWriteThreadWorker::createDatabase(QSqlDatabase& db)
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

bool ArchWriteThreadWorker::initDatabase()
{
	QString queryStr = QString("CREATE TABLE public.version("
								"versionid serial, "
								"date timestamp with time zone NOT NULL DEFAULT now(), "
								"reasone text NOT NULL, "
								"CONSTRAINT version_pkey PRIMARY KEY (versionid)) "
								"WITH (OIDS=FALSE);");
	QSqlQuery query(m_db);

	bool result = query.exec(queryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(m_logger, query.lastError().text());
		m_db.close();
		return false;
	}

	DEBUG_LOG_MSG(m_logger, QString("Table 'Version' is successful created"));

	return true;
}

bool ArchWriteThreadWorker::upgradeDatabase()
{
	int version = getDatabaseVersion();

	int filesArraySize = sizeof(m_upgradeFiles) / sizeof(StringPair);

	if (version == filesArraySize)
	{
		return true;				// nothing to upgrade
	}

	assert(version >= 0 && version < filesArraySize);

	for(int v = version; v < filesArraySize; v++)
	{
		QString fileName = m_upgradeFiles[v].first;

		QFile upgradeFile(fileName);

		bool result = upgradeFile.open(QIODevice::ReadOnly | QIODevice::Text);

		if (result == false)
		{
			DEBUG_LOG_ERR(m_logger, QString("Can't open database upgrade file '%1'").arg(fileName));
			return false;
		}

		QString upgradeScript = upgradeFile.readAll();

		QSqlQuery query(m_db);

		result = query.exec(upgradeScript);

		if (result == false)
		{
			DEBUG_LOG_ERR(m_logger, QString("Execution error of upgrade file '%1'").arg(fileName));
			DEBUG_LOG_ERR(m_logger, query.lastError().text());
			m_db.close();
			return false;
		}

		result = updateVersion(m_upgradeFiles[v].second);

		if (result == false)
		{
			break;
		}
	}

	DEBUG_LOG_MSG(m_logger, QString("Archive database is successful upgraded to version %1").arg(getDatabaseVersion()))

	return true;
}

int ArchWriteThreadWorker::getDatabaseVersion()
{
	QString queryStr = QString("SELECT MAX(versionId) FROM version;");

	QSqlQuery query(m_db);

	bool result = query.exec(queryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(m_logger, query.lastError().text());
		m_db.close();
		return 0;
	}

	int versionNo = 0;

	while(query.next() == true)
	{
		versionNo = query.value(0).toInt();
		break;
	}

	return versionNo;
}

bool ArchWriteThreadWorker::updateVersion(const QString& reasone)
{
	QString queryStr = QString("INSERT INTO version (reasone) VALUES('%1');").arg(reasone);

	QSqlQuery query(m_db);

	bool result = query.exec(queryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(m_logger, query.lastError().text());
		m_db.close();
		return 0;
	}

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

bool ArchWriteThreadWorker::createTableIfNotExists(Hash signalHash, bool isAnalogSignal)
{
	bool result = true;

	if (m_longTermTables.contains(signalHash) == false)
	{
		result &= createSignalStatesTable(signalHash, SignalStatesTableType::LongTerm);
	}

	if (isAnalogSignal == true)
	{
		// short term tables is creates for analog signals only
		//
		if (m_shortTermTables.contains(signalHash) == false)
		{
			result &= createSignalStatesTable(signalHash, SignalStatesTableType::ShortTerm);
		}
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

	QString queryStr;

	switch(tableType)
	{
	case SignalStatesTableType::LongTerm:
		queryStr = QString("CREATE TABLE public.%1 ("
							"archid bigint NOT NULL, "
							"planttime bigint, "
							"systime bigint, "
							"loctime bigint, "
							"val double precision, "
							"flags integer, "
							"CONSTRAINT %1_pkey PRIMARY KEY (archid)) "
							"WITH (OIDS=FALSE);").arg(tableName);
		break;

	case SignalStatesTableType::ShortTerm:

		queryStr = QString("CREATE TABLE public.%1 ("
							"archid bigint NOT NULL DEFAULT nextval('archid_seq'::regclass), "
							"planttime bigint, "
							"systime bigint, "
							"loctime bigint, "
							"val double precision, "
							"flags integer, "
							"CONSTRAINT %1_pkey PRIMARY KEY (archid)) "
							"WITH (OIDS=FALSE);").arg(tableName);
		break;

	default:
		assert(false);
		return false;
	}

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

	DEBUG_LOG_MSG(m_logger, QString("Archive table '%1' is successful created").arg(tableName));

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

	tableName += QString().setNum(signalHash, 16).rightJustified(sizeof(qint64) * 2, '0', false);

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

	const int MAX_STATES_IN_QUERY = 2000;

	if (m_saveStatesQueue.size() < MAX_STATES_IN_QUERY)
	{
		return;
	}

	int count = 0;

	int toWriteCount = 0;

	QTime t;

	t.start();

	QString	arrayStr;

	//arrayStr.reserve(2000000);		// reserve 2 000 000, usual arrayStr size is ~1 500 000

	QString format1("row(%1,%2,%3,%4,%5,%6,%7,%8)::AppSignalState");
	QString format2(",row(%1,%2,%3,%4,%5,%6,%7,%8)::AppSignalState");

	do
	{
		SimpleAppSignalState state;

		bool res = m_saveStatesQueue.pop(&state);

		if (res == false)
		{
			break;
		}

		if (m_archSignals.contains(state.hash) == false)
		{
			assert(false);
			count++;
			continue;
		}

		bool isAnalogSignal = m_archSignals.value(state.hash);

		res = createTableIfNotExists(state.hash, isAnalogSignal);

		if (res == false)
		{
			count++;
			continue;
		}

/*
 *		res = saveAppSignalStateToArchive(state, isAnalogSignal);

		if (res == false)
		{
			m_saveErrors++;
		}
		else
		{
			written++;
		} */

		qint64 bigintHash = *reinterpret_cast<qint64*>(&state.hash);

		bool writeToShortTimeArchiveOnly = state.flags.hasShortTermArchivingReasonOnly();

		if (toWriteCount == 0)
		{
			arrayStr = QString(format1).
					arg(bigintHash).
					arg(state.time.plant.timeStamp).
					arg(state.time.system.timeStamp).
					arg(state.time.local.timeStamp).
					arg(state.value).
					arg(state.flags.all).
					arg(isAnalogSignal == true ? "TRUE" : "FALSE").
					arg(writeToShortTimeArchiveOnly == true ? "TRUE" : "FALSE");
		}
		else
		{
			arrayStr.append(QString(format2).
					arg(bigintHash).
					arg(state.time.plant.timeStamp).
					arg(state.time.system.timeStamp).
					arg(state.time.local.timeStamp).
					arg(state.value).
					arg(state.flags.all).
					arg(isAnalogSignal == true ? "TRUE" : "FALSE").
					arg(writeToShortTimeArchiveOnly == true ? "TRUE" : "FALSE"));
		}

		toWriteCount++;

		count++;
	}
	while(count < MAX_STATES_IN_QUERY);

	if (toWriteCount > 0)
	{
		saveAppSignalStatesArrayToArchive(arrayStr);
		//DEBUG_LOG_MSG(m_logger, arrayStr);
	}

	if (toWriteCount != 0)
	{
		int time = t.elapsed();

		qDebug() << "Write states " << toWriteCount << "time " << time << "(per write =" << double(time) / toWriteCount << ")";
	}
}

bool ArchWriteThreadWorker::saveAppSignalStateToArchive(SimpleAppSignalState& state, bool isAnalogSignal)
{
	if (m_db.isOpen() == false)
	{
		assert(false);
		return false;
	}

	// convert unsigned int 64 hash to signet int 64 (equivalent to pgsql ::bigint)
	//
	qint64 bigintHash = *reinterpret_cast<qint64*>(&state.hash);

	bool writeToShortTimeArchiveOnly = state.flags.hasShortTermArchivingReasonOnly();

	QString queryStr = QString("SELECT * FROM saveAppSignalState(%1::bigint,%2::bigint,%3::bigint,%4::bigint,%5::double precision,%6::integer,%7,%8)").
						arg(bigintHash).
						arg(state.time.plant.timeStamp).
						arg(state.time.system.timeStamp).
						arg(state.time.local.timeStamp).
						arg(state.value).
						arg(state.flags.all).
						arg(isAnalogSignal == true ? "TRUE" : "FALSE").
						arg(writeToShortTimeArchiveOnly == true ? "TRUE" : "FALSE");

	QSqlQuery query(m_db);

	bool result = query.exec(queryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(m_logger, query.lastError().text());
		m_db.close();
		return false;
	}

	return result;
}


bool ArchWriteThreadWorker::saveAppSignalStatesArrayToArchive(const QString& arrayStr)
{
	if (m_db.isOpen() == false)
	{
		assert(false);
		return false;
	}

	QString queryStr = QString("SELECT * FROM saveAppSignalStatesArray(ARRAY[%1])").arg(arrayStr);

	QSqlQuery query(m_db);

	bool result = query.exec(queryStr);

	if (result == false)
	{
		assert(false);
		DEBUG_LOG_ERR(m_logger, query.lastError().text());
		m_db.close();
		return false;
	}

	return result;
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

ArchWriteThread::ArchWriteThread(const QString& projectID,
								 AppSignalStatesQueue& saveStatesQueue,
								 const ArchSignals& archSignals,
								 CircularLoggerShared logger)
{
	ArchWriteThreadWorker* worker = new ArchWriteThreadWorker(projectID, saveStatesQueue, archSignals, logger);

	addWorker(worker);
}
