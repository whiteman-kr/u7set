#include "Archive.h"
#include "../lib/WUtils.h"

const char* Archive::ARCH_DB_PREFIX = "u7arch_";

const char* Archive::FIELD_PLANT_TIME = "plantTime";
const char* Archive::FIELD_SYSTEM_TIME = "sysTime";
const char* Archive::FIELD_ARCH_ID = "archID";
const char* Archive::FIELD_VALUE = "val";
const char* Archive::FIELD_FLAGS = "flags";


Archive::Archive(const QString& projectID, const HostAddressPort& dbHost, CircularLoggerShared logger) :
	m_projectID(projectID),
	m_dbHost(dbHost),
	m_logger(logger)
{
	m_dbUser = "u7arch";
	m_dbPassword =  "arch876436";
}

Archive::~Archive()
{
	clear();
}

bool Archive::openDatabase(DbType dbType, QSqlDatabase& destDb)
{
	QSqlDatabase db = getDatabase(dbType);

	if (db.isValid() == false)
	{
		DEBUG_LOG_MSG(m_logger, QString("Database '%1' is invalid").arg(db.connectionName()));
		return false;
	}

	if (db.isOpen() == true)
	{
		destDb = db;
		return true;
	}

	QString databaseName = postgresDatabaseName();

	if (dbType != DbType::Postgres)
	{
		databaseName = archiveDatabaseName();
	}

	db.setHostName(m_dbHost.addressStr());
	db.setPort(m_dbHost.port());
	db.setDatabaseName(databaseName);
	db.setUserName(m_dbUser);
	db.setPassword(m_dbPassword);

	bool res = db.open();

	if (res == true)
	{
		destDb = db;
		return true;
	}

	DEBUG_LOG_ERR(m_logger, db.lastError().text());
	return false;
}

void Archive::initArchSignals(int count)
{
	m_archSignals.clear();
	m_signalIDs.clear();

	/*m_archSignals.reserve(static_cast<int>(count * 1.2));
	m_signalIDs.reserve(static_cast<int>(count * 1.2));*/
}

void Archive::appendArchSignal(const QString& appSignalID, const ArchSignal& archSignal)
{
	if (m_archSignals.contains(archSignal.hash) == true)
	{
		assert(false);
	}
	else
	{
		m_archSignals.insert(archSignal.hash, archSignal);
		m_signalIDs.insert(archSignal.hash, appSignalID);
	}
}

ArchSignal Archive::getArchSignal(Hash signalHash)
{
	return m_archSignals.value(signalHash, ArchSignal());
}

QString Archive::getSignalID(Hash signalHash)
{
	return m_signalIDs.value(signalHash, QString());
}

bool Archive::canReadWriteSignal(Hash signalHash)
{
	if (m_archSignals.contains(signalHash) == false)
	{
		return false;
	}

	return m_archSignals[signalHash].canReadWrite;
}

void Archive::setCanReadWriteSignal(Hash signalHash, bool canReadWrite)
{
	if (m_archSignals.contains(signalHash) == false)
	{
		assert(false);
		return;
	}

	m_archSignals[signalHash].canReadWrite = canReadWrite;
}

QString Archive::postgresDatabaseName()
{
	return "postgres";
}

QString Archive::archiveDatabaseName()
{
	assert(m_projectID.isEmpty() == false);

	QString dbName = QString(ARCH_DB_PREFIX) + m_projectID;

	dbName = dbName.toLower();

	return dbName;
}

QString Archive::getTableName(Hash signalHash)
{
	return QString("z_%1").arg(QString().setNum(signalHash, 16).rightJustified(sizeof(qint64) * 2, '0', false));
/*
	QString tableName;

	switch(tableType)
	{
	case TableType::LongTerm:
		tableName = LONG_TERM_TABLE_PREFIX;
		break;

	case TableType::ShortTerm:
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

	return tableName;*/
}

void Archive::appendExistingTable(const QString& tableName)
{
	if (m_existingTables.contains(tableName) == true)
	{
		assert(false);
		return;
	}

	m_existingTables.insert(tableName, tableName);
}

bool Archive::tableIsExists(const QString& tableName)
{
	return m_existingTables.contains(tableName);
}

QString Archive::timeTypeStr(TimeType timeType)
{
	switch(timeType)
	{
	case TimeType::Plant:
		return QString("Plant");

	case TimeType::System:
		return QString("System");

	case TimeType::Local:
		return QString("Local");

	case TimeType::ArchiveId:
		return QString("ArchiveId");

	default:
		assert(false);
	}

	return QString("???");
}


qint64 Archive::localTimeOffsetFromUtc()
{
	QDateTime local(QDateTime::currentDateTime());

	qint64 offset = local.offsetFromUtc() * 1000;

	return offset;
}

QString Archive::getCmpField(TimeType timeType)
{
	QString cmpField;

	switch(timeType)
	{
	case TimeType::Plant:
		cmpField = FIELD_PLANT_TIME;
		break;

	case TimeType::System:
	case TimeType::Local:						// local time search also use systemtime field in requests
		cmpField = FIELD_SYSTEM_TIME;
		break;

	case TimeType::ArchiveId:
		cmpField = FIELD_ARCH_ID;
		break;

	default:
		assert(false);
	}

	return cmpField;
}

void Archive::setSignalInitialized(Hash signalHash, bool initilaized)
{
	if (m_archSignals.contains(signalHash) == false)
	{
		assert(false);
		return;
	}

	m_archSignals[signalHash].isInitialized = initilaized;
}

void Archive::clear()
{
	m_projectID.clear();
	m_archSignals.clear();
	m_signalIDs.clear();
	m_existingTables.clear();

	removeDatabases();
}

QSqlDatabase Archive::getDatabase(DbType dbType)
{
	QString connectionName;

	switch(dbType)
	{
	case DbType::Postgres:
		connectionName = "PostgresConnection";
		break;

	case DbType::WriteArchive:
		connectionName = "WriteArchiveConnection";
		break;

	case DbType::ReadArchive:
		connectionName = "ReadArchiveConnection";
		break;

	default:
		assert(false);
	}

	AUTO_LOCK(m_dbMutex);

	if (QSqlDatabase::contains(connectionName) == true)
	{
		return QSqlDatabase::database(connectionName, false);
	}

	return QSqlDatabase::addDatabase("QPSQL", connectionName);
}

void Archive::removeDatabases()
{
	AUTO_LOCK(m_dbMutex);

	QStringList connectionNames = QSqlDatabase::connectionNames();

	for(QString& connectionName : connectionNames)
	{
		QSqlDatabase::removeDatabase(connectionName);
	}
}




