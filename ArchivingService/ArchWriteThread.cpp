#include "ArchWriteThread.h"
#include "ArchivingService.h"

#include <chrono>

const StringPair ArchWriteThreadWorker::m_upgradeFiles[] =
{
	StringPair(":/Upgrade0001.sql", "Initial upgrade"),
};


QString ArchWriteThreadWorker::m_format1("row(%1,%2,%3,%4,%5)::AppSignalState");
QString ArchWriteThreadWorker::m_format2(",row(%1,%2,%3,%4,%5)::AppSignalState");


ArchWriteThreadWorker::ArchWriteThreadWorker(const HostAddressPort& dbHost,
											 ArchiveShared archive,
											 AppSignalStatesQueue& saveStatesQueue,
											 CircularLoggerShared logger) :
	m_dbHost(dbHost),
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

bool ArchWriteThreadWorker::tryConnectToDb()
{
	if (m_db.isOpen() == true)
	{
		return true;
	}

	QSqlDatabase db;

	bool result = m_archive->openDatabase(Archive::DbType::Postgres, db);

	if (result == false)
	{
		return false;
	}

	bool dbJustCreated = false;

	if (databaseIsExists(db) == false)
	{
		bool res = createDatabase(db);

		if (res == false)
		{
			return false;
		}

		dbJustCreated = true;
	}

	// open connection to projectArchive database

	result = m_archive->openDatabase(Archive::DbType::WriteArchive, m_db);

	if (result == false)
	{
		return false;
	}

	if (dbJustCreated == true)
	{
		result = initDatabase();

		if (result == false)
		{
			return false;
		}
	}

	result = upgradeDatabase();

	if (result == false)
	{
		return false;
	}

	result = checkAndCreateTables();

	return result;
}

bool ArchWriteThreadWorker::databaseIsExists(QSqlDatabase& db)
{
	if (db.isOpen() == false)
	{
		assert(false);
		return false;
	}

	QString dbName = m_archive->archiveDatabaseName();

	QString queryStr = QString("SELECT datname FROM pg_database WHERE datname = '%1';").
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

	QString dbName = m_archive->archiveDatabaseName();

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

	const QHash<Hash, ArchSignal>& archSignals = m_archive->archSignals();

	QHashIterator<Hash, ArchSignal> i(archSignals);

	QString tableName;

	int createdTablesCount = 0;
	int creationErrorCount = 0;

	int ctr = 0;

	while(i.hasNext() == true && quitRequested() == false)
	{
		i.next();

		Hash signalHash = i.key();
		const ArchSignal& archSignal = i.value();

		bool tableIsExists = false;

		tableName = m_archive->getTableName(signalHash);

		if (existingTables.contains(tableName) == false)
		{
			bool res = createTable(tableName);

			if (res == true)
			{
				createdTablesCount++;

				tableIsExists = true;
			}
			else
			{
				creationErrorCount++;
			}
		}
		else
		{
			tableIsExists = true;
		}

		if (tableIsExists == true)
		{
			m_archive->appendExistingTable(tableName);
		}

		m_archive->setCanReadWriteSignal(archSignal.hash, tableIsExists);

		if (createdTablesCount - ctr >= 500)
		{
			DEBUG_LOG_MSG(m_logger, QString("Tables created: %1").arg(createdTablesCount));

			ctr = createdTablesCount;
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
								"WHERE table_schema = 'public' AND (table_name LIKE 'z\\_%') "
								"ORDER BY table_name");

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

bool ArchWriteThreadWorker::createTable(const QString& tableName)
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

	queryStr = QString("CREATE TABLE IF NOT EXISTS  public.%1 ("
							"archid bigint NOT NULL DEFAULT nextval('archid_seq'::regclass), "
							"planttime bigint, "
							"systime bigint, "
							"loctime bigint, "
							"val double precision, "
							"flags integer, "
							"CONSTRAINT %1_pkey PRIMARY KEY (archid)) "
							"WITH (OIDS=FALSE);").arg(tableName);
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

void ArchWriteThreadWorker::writeStatesToArchive(bool writeNow)
{
	if (m_db.isOpen() == false)
	{
		return;
	}

	if (m_saveStatesQueue.isEmpty() == true)
	{
		return;
	}

	const int MAX_STATES_IN_QUERY = 1000;

	if (writeNow == false && m_saveStatesQueue.size() < MAX_STATES_IN_QUERY)
	{
		return;
	}

	int toWriteCount = 0;

	QTime t;

	t.start();

	QString	arrayStr;

	do
	{
		SimpleAppSignalState state;

		bool res = m_saveStatesQueue.pop(&state);

		if (res == false)
		{
			break;
		}

		ArchSignal archSignal = m_archive->getArchSignal(state.hash);

		if (archSignal.canReadWrite == false)
		{
			continue;
		}

		if (archSignal.isAnalog == false && state.flags.smoothAperture == 1)
		{
			assert(false);
			state.flags.smoothAperture = 0;		// hard reset state.flags.smoothAperture for discrete signals
		}

		m_timeFilter.setTimes(state.time);

		if (m_firstState == true)
		{
			writeTimeMark();

			m_firstState = false;
		}

		if (archSignal.isInitialized == false)
		{
			if (state.flags.valid == 1)
			{
				// first received state is valid,
				// write invalid point at time offset -1 ms from received state
				//
				state.flags.valid = 0;
				state.time.plant.timeStamp--;
				state.time.system.timeStamp--;
				state.time.local.timeStamp--;

				appendToArray(state, arrayStr);

				// returns previous values of state fields
				//
				state.flags.valid = 1;
				state.time.plant.timeStamp++;
				state.time.system.timeStamp++;
				state.time.local.timeStamp++;
			}
			else
			{
				// if received state is not valid,
				// the below call of appendToArray(state, archSignal.isAnalog, arrayStr) is writes an invalid point and
				// signal also can be assigned as initialized
				//
			}

			m_archive->setSignalInitialized(state.hash, true);
		}

		appendToArray(state, arrayStr);

		toWriteCount++;
	}
	while(toWriteCount < MAX_STATES_IN_QUERY);

	if (toWriteCount > 0)
	{
		saveAppSignalStatesArrayToArchive(arrayStr);

		int time = t.elapsed();
		qDebug() << C_STR(QString("Write states %1 time %2 (per state %3)").
						  arg(toWriteCount).arg(time).arg(double(time) / toWriteCount));

	}
}

void ArchWriteThreadWorker::appendToArray(const SimpleAppSignalState& state, QString& arrayStr)
{
	qint64 bigintHash = *reinterpret_cast<const qint64*>(&state.hash);

	if (arrayStr.isEmpty() == true)
	{
		arrayStr = QString(m_format1).
				arg(bigintHash).
				arg(state.time.plant.timeStamp).
				arg(state.time.system.timeStamp).
				arg(state.value).
				arg(state.flags.all);
	}
	else
	{
		arrayStr.append(QString(m_format2).
				arg(bigintHash).
				arg(state.time.plant.timeStamp).
				arg(state.time.system.timeStamp).
				arg(state.value).
				arg(state.flags.all));
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
		//assert(false);
		DEBUG_LOG_ERR(m_logger, query.lastError().text());
		m_db.close();
		return false;
	}

	return result;
}


bool ArchWriteThreadWorker::writeTimeMark()
{
	if (m_db.isOpen() == false)
	{
		assert(false);
		return false;
	}

	Times times;
	qint64 serverTime;

	m_timeFilter.getTimes(times, serverTime);

	QString queryStr = QString("INSERT INTO timemarks (planttime, systime, servertime) "
							   "VALUES (%1::bigint, %2::bigint, %3::bigint);").
							arg(times.plant.timeStamp).
							arg(times.system.timeStamp).
							arg(serverTime);

	QSqlQuery query(m_db);

	bool result = query.exec(queryStr);

	if (result == false)
	{
		assert(false);
		DEBUG_LOG_ERR(m_logger, query.lastError().text());
		m_db.close();
		return false;
	}

	return true;
}


void ArchWriteThreadWorker::onTimer()
{
	if (m_db.isOpen() == false)
	{
		bool res = tryConnectToDb();

		if (res == false)
		{
			return;
		}
	}

	QTime t = QTime::currentTime();

	int currMinute = t.minute();
	int currSecond = t.second();

	bool writeNow = (currSecond % 5) == 0 ? true : false;

	writeStatesToArchive(writeNow);

	if (currMinute != m_prevMinute)
	{
		m_prevMinute = currMinute;
		writeTimeMark();
	}
}

void ArchWriteThreadWorker::onSaveStatesQueueIsNotEmpty()
{
	writeStatesToArchive(false);
}

ArchWriteThread::ArchWriteThread(const HostAddressPort& dbHost,
								 ArchiveShared archive,
								 AppSignalStatesQueue& saveStatesQueue,
								 CircularLoggerShared logger)
{
	ArchWriteThreadWorker* worker = new ArchWriteThreadWorker(dbHost,
															  archive,
															  saveStatesQueue,
															  logger);

	addWorker(worker);
}
