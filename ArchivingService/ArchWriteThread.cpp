#include "ArchWriteThread.h"
#include "ArchivingService.h"

#include <chrono>

const StringPair ArchWriteThreadWorker::m_upgradeFiles[] =
{
	StringPair(":/Upgrade0001.sql", "Initial upgrade"),
};


ArchWriteThreadWorker::ArchWriteThreadWorker(Archive& archive,
											 AppSignalStatesQueue& saveStatesQueue,
											 CircularLoggerShared logger) :
	m_archive(archive),
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
	m_db.setDatabaseName(m_archive.dbName());
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

	result &= upgradeDatabase();

	result &= checkAndCreateTables();
}

bool ArchWriteThreadWorker::databaseIsExists(QSqlDatabase& db)
{
	if (db.isOpen() == false)
	{
		assert(false);
		return false;
	}

	QString dbName = m_archive.dbName();

	QString queryStr = QString("SELECT datname FROM pg_database WHERE datname = '%1' ORDER BY datname;").
			arg(dbName);

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

		if (databaseName == dbName)
		{
			DEBUG_LOG_MSG(m_logger, QString("Archive database '%1' is exists").arg(dbName));
			return true;
		}
	}

	DEBUG_LOG_WRN(m_logger, QString("Archive database '%1' is not exists").arg(dbName));

	return false;
}

bool ArchWriteThreadWorker::createDatabase(QSqlDatabase& db)
{
	if (db.isOpen() == false)
	{
		assert(false);
		return false;
	}

	QString dbName = m_archive.dbName();

	QString queryStr = QString("CREATE DATABASE %1 WITH ENCODING = 'UTF8' CONNECTION LIMIT = -1;").
							arg(dbName);

	QSqlQuery query(db);

	bool result = query.exec(queryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(m_logger, query.lastError().text());
		db.close();
		return false;
	}

	DEBUG_LOG_MSG(m_logger, QString("Archive database '%1' is successful created").arg(dbName));

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

bool ArchWriteThreadWorker::checkAndCreateTables()
{
	QHash<QString, QString> existingTables;

	bool result = getExistingTables(existingTables);

	if (result == false)
	{
		return false;
	}

	const QHash<Hash, ArchSignal>& archSignals = m_archive.archSignals();

	QHashIterator<Hash, ArchSignal> i(archSignals);

	QString tableName;

	int createdTablesCount = 0;
	int creationErrorCount = 0;

	while(i.hasNext() == true)
	{
		i.next();

		Hash signalHash = i.key();
		const ArchSignal& archSignal = i.value();

		bool stTableExists = false;
		bool ltTableExists = false;

		if (archSignal.isAnalog == true)
		{
			// analog signal should have table st_*
			//
			tableName = m_archive.getTableName(signalHash, Archive::TableType::ShortTerm);

			if (existingTables.contains(tableName) == false)
			{
				bool res = createTable(tableName, Archive::TableType::ShortTerm);

				if (res == true)
				{
					createdTablesCount++;

					stTableExists = true;
				}
				else
				{
					creationErrorCount++;
				}
			}
			else
			{
				stTableExists = true;
			}

			if (stTableExists)
			{
				m_archive.appendExistingTable(tableName);
			}
		}

		// analog and discrete signals should have table  lt_*
		//
		tableName = m_archive.getTableName(signalHash, Archive::TableType::LongTerm);

		if (existingTables.contains(tableName) == false)
		{
			bool res = createTable(tableName, Archive::TableType::LongTerm);

			if (res == true)
			{
				createdTablesCount++;

				ltTableExists = true;
			}
			else
			{
				creationErrorCount++;
			}
		}
		else
		{
			ltTableExists = true;
		}

		if (ltTableExists)
		{
			m_archive.appendExistingTable(tableName);
		}

		if (archSignal.isAnalog == true)
		{
			m_archive.setCanReadWriteSignal(archSignal.hash, stTableExists && ltTableExists);
		}
		else
		{
			m_archive.setCanReadWriteSignal(archSignal.hash, ltTableExists);
		}
	}

	if (createdTablesCount > 0)
	{
		DEBUG_LOG_MSG(m_logger, QString("Tables has been created: %1").arg(createdTablesCount));
	}

	if (creationErrorCount > 0)
	{
		DEBUG_LOG_ERR(m_logger, QString("Tables creation errors: %1").arg(creationErrorCount));
		return false;
	}

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

bool ArchWriteThreadWorker::getExistingTables(QHash<QString, QString>& existingTables)
{
	if (m_db.isOpen() == false)
	{
		assert(false);
		return false;
	}

	existingTables.clear();

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

		existingTables.insert(tableName, tableName);
	}

	return true;
}

bool ArchWriteThreadWorker::createTable(const QString& tableName, Archive::TableType tableType)
{
	if (m_db.isOpen() == false)
	{
		return false;
	}

	if (tableName.isEmpty() == true)
	{
		assert(false);
		return false;
	}

	QString queryStr;

	switch(tableType)
	{
	case Archive::TableType::LongTerm:
		queryStr = QString("CREATE TABLE IF NOT EXISTS public.%1 ("
							"archid bigint NOT NULL, "
							"planttime bigint, "
							"systime bigint, "
							"loctime bigint, "
							"val double precision, "
							"flags integer, "
							"CONSTRAINT %1_pkey PRIMARY KEY (archid)) "
							"WITH (OIDS=FALSE);").arg(tableName);
		break;

	case Archive::TableType::ShortTerm:

		queryStr = QString("CREATE TABLE IF NOT EXISTS  public.%1 ("
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

	// DEBUG_LOG_MSG(m_logger, QString("Archive table '%1' is successful created").arg(tableName));

	return true;
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

		ArchSignal archSignal = m_archive.getArchSignal(state.hash);

		if (archSignal.canReadWrite == false)
		{
			QString appSignalID = m_archive.getSignalID(state.hash);
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
					arg(archSignal.isAnalog == true ? "TRUE" : "FALSE").
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
					arg(archSignal.isAnalog == true ? "TRUE" : "FALSE").
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

ArchWriteThread::ArchWriteThread(Archive& archive,
								 AppSignalStatesQueue& saveStatesQueue,
								 CircularLoggerShared logger)
{
	ArchWriteThreadWorker* worker = new ArchWriteThreadWorker(archive,
															  saveStatesQueue,
															  logger);

	addWorker(worker);
}
