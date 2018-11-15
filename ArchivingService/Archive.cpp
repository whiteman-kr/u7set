#include "Archive.h"
#include "../lib/WUtils.h"

// ----------------------------------------------------------------------------------------------------------------------
//
// Archive::Signal class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

void Archive::Signal::Signal(const Proto::ArchSignal& protoArchSignal)
{
	hash = protoArchSignal.hash();
	appSignalID = QString::fromStdString(protoArchSignal.appsignalid());
	isAnalog = protoArchSignal.isanalog();
}

// ----------------------------------------------------------------------------------------------------------------------
//
// Archive class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

const char* Archive::ARCH_DB_PREFIX = "u7arch_";

const char* Archive::FIELD_PLANT_TIME = "plantTime";
const char* Archive::FIELD_SYSTEM_TIME = "sysTime";
const char* Archive::FIELD_ARCH_ID = "archID";
const char* Archive::FIELD_VALUE = "val";
const char* Archive::FIELD_FLAGS = "flags";

Archive::Archive(const QString& projectID,
				 const QString& equipmentID,
				 const QString& archDir,
				 const HostAddressPort& dbHost,
				 CircularLoggerShared logger) :
	m_projectID(projectID),
	m_equipmentID(equipmentID),
	m_archDir(archDir),
	m_dbHost(dbHost),
	m_logger(logger),
	m_dbSaveStatesQueue(1024 * 1024),
	m_saveStatesQueue(1024 * 1024)
{
	m_archFullPath = QString("%1/%2-archive/%3").arg(m_archDir).arg(m_projectID).arg(m_equipmentID);

	//

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

void Archive::initArchSignals(const Proto::ArchSignals& archSignals)
{
	assert(m_archiveSignals.count() == 0);
	assert(m_archFiles.count() == 0);

	int signalsCount = archSignals.archsignals_size();

	m_archiveSignals.reserve(static_cast<int>(signalsCount * 1.2));
	m_archFiles.resize(signalsCount);

	for(int i = 0; i < signalsCount; i++)
	{
		const Proto::ArchSignal& protoArchSignal = archSignals.archsignals(i);

		Archive::Signal* archSignal = new Archive::Signal(protoArchSignal);

		archSignal.archFile = new ArchFile(this, archSignal);

		m_archiveSignals.insert(archSignal.hash, archSignal);

		m_archFiles[i] = archSignal.archFile;
	}
}

QString Archive::getSignalID(Hash signalHash)
{
	return m_signalIDs.value(signalHash, QString());
}

bool Archive::canReadWriteSignal(Hash signalHash)
{
	if (m_archiveSignals.contains(signalHash) == false)
	{
		return false;
	}

	return m_archiveSignals[signalHash].canReadWrite;
}

void Archive::setCanReadWriteSignal(Hash signalHash, bool canReadWrite)
{
	if (m_archiveSignals.contains(signalHash) == false)
	{
		assert(false);
		return;
	}

	m_archiveSignals[signalHash].canReadWrite = canReadWrite;
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

QString Archive::timeTypeStr(E::TimeType timeType)
{
	switch(timeType)
	{
	case E::TimeType::Plant:
		return QString("Plant");

	case E::TimeType::System:
		return QString("System");

	case E::TimeType::Local:
		return QString("Local");

	case E::TimeType::ArchiveId:
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

QString Archive::getCmpField(E::TimeType timeType)
{
	QString cmpField;

	switch(timeType)
	{
	case E::TimeType::Plant:
		cmpField = FIELD_PLANT_TIME;
		break;

	case E::TimeType::System:
	case E::TimeType::Local:						// local time search also use systemtime field in requests
		cmpField = FIELD_SYSTEM_TIME;
		break;

	case E::TimeType::ArchiveId:
		cmpField = FIELD_ARCH_ID;
		break;

	default:
		assert(false);
	}

	return cmpField;
}

void Archive::setSignalInitialized(Hash signalHash, bool initilaized)
{
	if (m_archiveSignals.contains(signalHash) == false)
	{
		assert(false);
		return;
	}

	m_archiveSignals[signalHash].isInitialized = initilaized;
}

void Archive::saveState(const SimpleAppSignalState& state)
{
	m_dbSaveStatesQueue.push(&state);
	m_saveStatesQueue.push(&state);
}

void Archive::clear()
{
	m_projectID.clear();
	m_archiveSignals.clear();
	m_existingTables.clear();
	m_archiveSignals.clear();

	for(ArchFile* archFile : m_archFiles)
	{
		delete archFile;
	}

	m_archFiles.clear();

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




