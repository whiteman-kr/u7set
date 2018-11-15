#include "Archive.h"
#include "../lib/WUtils.h"

// ----------------------------------------------------------------------------------------------------------------------
//
// ArchSignal class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

ArchSignal::ArchSignal(Archive* archive, const Proto::ArchSignal& protoArchSignal) :
	archFile(archive, this)
{
	hash = protoArchSignal.hash();
	appSignalID = QString::fromStdString(protoArchSignal.appsignalid());
	isAnalog = protoArchSignal.isanalog();

	lastState.flags.valid = 0;
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
	m_log(logger),
	m_dbSaveStatesQueue(1024 * 1024)
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
		DEBUG_LOG_MSG(m_log, QString("Database '%1' is invalid").arg(db.connectionName()));
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

	DEBUG_LOG_ERR(m_log, db.lastError().text());
	return false;
}

void Archive::initArchSignals(const Proto::ArchSignals& archSignals)
{
	assert(m_archSignals.count() == 0);
	assert(m_archFiles.count() == 0);

	int signalsCount = archSignals.archsignals_size();

	m_archSignals.reserve(static_cast<int>(signalsCount * 1.2));
	m_archFiles.resize(signalsCount);

	for(int i = 0; i < signalsCount; i++)
	{
		const Proto::ArchSignal& protoArchSignal = archSignals.archsignals(i);

		ArchSignal* archSignal = new ArchSignal(this, protoArchSignal);

		m_archSignals.insert(archSignal->hash, archSignal);

		m_archFiles[i] = &archSignal->archFile;
	}
}

QString Archive::getSignalID(Hash signalHash)
{
	ArchSignal* archSignal = m_archSignals.value(signalHash, nullptr);

	if (archSignal == nullptr)
	{
		assert(false);
		return QString();
	}

	return archSignal->appSignalID;
}

bool Archive::canReadWriteSignal(Hash signalHash)
{
	ArchSignal* archSignal = m_archSignals.value(signalHash, nullptr);

	if (archSignal == nullptr)
	{
		assert(false);
		return false;
	}

	return archSignal->canReadWrite;
}

void Archive::setCanReadWriteSignal(Hash signalHash, bool canReadWrite)
{
	ArchSignal* archSignal = m_archSignals.value(signalHash, nullptr);

	if (archSignal == nullptr)
	{
		assert(false);
		return;
	}

	archSignal->canReadWrite = canReadWrite;
}

void Archive::setSignalInitialized(Hash signalHash, bool initilaized)
{
	ArchSignal* archSignal = m_archSignals.value(signalHash, nullptr);

	if (archSignal == nullptr)
	{
		assert(false);
		return;
	}

	archSignal->isInitialized = initilaized;
}

void Archive::getArchSignalStatus(Hash signalHash, bool* canReadWrite, bool* isInitialized, bool* isAnalog)
{
	TEST_PTR_RETURN(canReadWrite);
	TEST_PTR_RETURN(isInitialized);
	TEST_PTR_RETURN(isAnalog);

	ArchSignal* archSignal = m_archSignals.value(signalHash, nullptr);

	if (archSignal == nullptr)
	{
		*canReadWrite = false;
		*isInitialized = false;
		*isAnalog = false;
		return;
	}

	*canReadWrite = archSignal->canReadWrite;
	*isInitialized = archSignal->isInitialized;
	*isAnalog = archSignal->isAnalog;
}


void Archive::getSignalsHashes(QVector<Hash>* hashes)
{
	TEST_PTR_RETURN(hashes);

	hashes->resize(m_archSignals.count());

	int i = 0;

	for(ArchSignal* archSignal : m_archSignals)
	{
		(*hashes)[i] = archSignal->hash;
		i++;
	}
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

void Archive::saveState(const SimpleAppSignalState& state)
{
	m_dbSaveStatesQueue.push(&state);

	//

	ArchSignal* archSignal = m_archSignals.value(state.hash, nullptr);

	if (archSignal == nullptr)
	{
		assert(false);
		return;
	}

	archSignal->lastState = state;

	m_archID++;

	archSignal->archFile.pushState(m_archID, state);

	if (archSignal->archFile.isEmergency() == true)
	{
		addEmergencyFile(&archSignal->archFile);
	}
}

void Archive::addEmergencyFile(ArchFile* file)
{
	QMutexLocker locker(&m_emergencyFilesMutex);

	if (m_emergencyFilesInQueue.contains(file) == false)
	{
		m_emergencyFilesQueue.append(file);
		m_emergencyFilesInQueue.insert(file, true);
	}
}

ArchFile* Archive::getNextEmergencyFile()
{
	ArchFile* emergencyFile = nullptr;

	QMutexLocker locker(&m_emergencyFilesMutex);

	if (m_emergencyFilesQueue.isEmpty() == false)
	{
		emergencyFile = m_emergencyFilesQueue.first();
		m_emergencyFilesQueue.removeFirst();
		m_emergencyFilesInQueue.remove(emergencyFile);
	}

	return emergencyFile;
}

bool Archive::flushImmediately(Hash signalHash)
{
	ArchFile* archFile = getArchFile(signalHash);

	TEST_PTR_RETURN_FALSE(archFile);

	m_immedaitelyFlushingMutex.lock();

	archFile->setRequiredImmediatelyFlushing(true);

	m_requiredImmediatelyFlushing.append(archFile);

	m_immedaitelyFlushingMutex.unlock();

	return true;
}

bool Archive::waitingForImmediatelyFlushing(Hash signalHash, int waitTimeoutSeconds)
{
	ArchFile* archFile = getArchFile(signalHash);

	TEST_PTR_RETURN_FALSE(archFile);

	bool result = false;

	const int WAIT_TIME_MCS = 500;											// 500 microseconds
	int maxWaitCount = waitTimeoutSeconds * 1000 * 1000 / WAIT_TIME_MCS;

	int waitCount = 0;

	do
	{
		QThread::usleep(WAIT_TIME_MCS);

		if (archFile->isRequiredImmediatelyFlushing() == false)
		{
			result = true;
			break;
		}

		waitCount++;

		if (waitCount >= maxWaitCount)
		{
			result = false;
			break;
		}
	}
	while(1);

	return result;

}


ArchFile* Archive::getNextRequredImediatelyFlushing()
{
	ArchFile* archFile = nullptr;

	QMutexLocker locker(&m_immedaitelyFlushingMutex);

	if (m_requiredImmediatelyFlushing.isEmpty() == false)
	{
		archFile = m_requiredImmediatelyFlushing.first();

		m_requiredImmediatelyFlushing.removeFirst();
	}

	return archFile;
}

ArchFile* Archive::getNextRegularFile()
{
	int archFilesCount = m_archFiles.count();

	if (archFilesCount == 0)
	{
		return nullptr;
	}

	QMutexLocker locker(&m_regularFilesMutex);

	if (m_regularFileIndex >= archFilesCount)
	{
		m_regularFileIndex = 0;
	}

	ArchFile* nextRegularFile = m_archFiles[m_regularFileIndex];

	m_regularFileIndex++;

	return nextRegularFile;
}

bool Archive::checkAndCreateArchiveDirs()
{
	bool result = archDirIsWritableChecking();

	if (result == false)
	{
		return false;
	}

	result = createGroupDirs();

	return result;
}

bool Archive::archDirIsWritableChecking()
{
	int pass = 1;

	bool result = false;

	QString archDir;
	do
	{
		if (pass == 1)
		{
			archDir = m_archDir;
		}
		else
		{
			archDir = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppDataLocation);
		}

		m_archFullPath = QDir(QString("%1/%2-archive/%3").arg(archDir).arg(m_projectID).arg(m_equipmentID)).absolutePath();

		QDir d(m_archFullPath);

		DEBUG_LOG_MSG(m_log, QString("Archive directory %1 checking...").arg(m_archFullPath));

		if (d.exists() == false)
		{
			if (d.mkpath(m_archFullPath) == false)
			{
				DEBUG_LOG_ERR(m_log, QString("Archive directory %1 creation error").arg(m_archFullPath));
				pass++;
				continue;
			}
			else
			{
				DEBUG_LOG_MSG(m_log, QString("Archive directory %1 is created successfully").arg(m_archFullPath));
			}
		}
		else
		{
			DEBUG_LOG_MSG(m_log, QString("Archive directory %1 allready exists").arg(m_archFullPath));
		}

		QFileInfo fi(m_archFullPath);

		if (fi.isDir() == false)
		{
			DEBUG_LOG_ERR(m_log, QString("Path %1 is not a directory!").arg(m_archFullPath));
			pass++;
			continue;
		}

		if (fi.isWritable() == false)
		{
			DEBUG_LOG_ERR(m_log, QString("Directory %1 is not writable!").arg(m_archFullPath));
			pass++;
			continue;
		}

		qint64 time = QDateTime::currentMSecsSinceEpoch();

		QString testDir = QString("%1/test_dir_%2").arg(m_archFullPath).arg(time);

		if (d.mkpath(testDir) == false)
		{
			DEBUG_LOG_ERR(m_log, QString("Test directory %1 creation error!").arg(testDir));
			pass++;
			continue;
		}

		DEBUG_LOG_MSG(m_log, QString("Test directory %1 is created successfully").arg(testDir));

		d.rmdir(testDir);

		QString testFile = QString("%1/test_file_%2.dat").arg(m_archFullPath).arg(time);

		QFile f(testFile);

		if (f.open(QIODevice::ReadWrite) == false)
		{
			DEBUG_LOG_ERR(m_log, QString("Test file %1 creation error!").arg(testFile));
			pass++;
			continue;
		}

		DEBUG_LOG_MSG(m_log, QString("Test file %1 is created successfully").arg(testFile));

		f.remove();

		DEBUG_LOG_MSG(m_log, QString("Archive directory %1 checking succesfully completed").arg(m_archFullPath));

		result = true;

		break;
	}
	while(pass <= 2);

	return result;
}

bool Archive::createGroupDirs()
{
	bool result = true;

	for(int i = 0; i < 256; i++)
	{
		QString dir = QString("%1/%2").arg(m_archFullPath).arg(QString().sprintf("%02X", i));

		QDir d;

		bool res = d.mkpath(dir);

		if (res == false)
		{
			DEBUG_LOG_ERR(m_log, QString("Directory %1 creation error!").arg(dir));

			result = false;
		}
	}

	return result;
}


bool Archive::shutdown()
{
/*	if (m_lastState.flags.valid == 1)
	{
		qint64 dTime = system - m_lastState.time.system;

		if (dTime > 0)
		{
			m_lastState.time.system = systemTime;
			m_lastState.time.local += dTime;
			m_lastState.time.plant += dTime;

			pushState()

		}
	}

	ArchFile* firstArchFile = nullptr;

	// shutting down all archive files
	//
	do
	{
		ArchFile* archFile = m_archive->getNextRegularFile();

		if (archFile == nullptr)
		{
			break;
		}

		if (firstArchFile == nullptr)
		{
			firstArchFile = archFile;
		}
		else
		{
			if (firstArchFile == archFile)
			{
				break;
			}
		}

		archFile->shutdown(m_curPartition, &m_totalFlushedStatesCount);
	}
	while(1);
}
*/
	assert(false);

	return true;
}


void Archive::clear()
{
	m_projectID.clear();
	m_existingTables.clear();

	m_archFiles.clear();

	for(ArchSignal* archSignal : m_archSignals)
	{
		delete archSignal;
	}

	m_archSignals.clear();

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

ArchFile* Archive::getArchFile(Hash signalHash)
{
	ArchSignal* archSignal = m_archSignals.value(signalHash, nullptr);

	if (archSignal == nullptr)
	{
		return nullptr;
	}

	return &archSignal->archFile;
}





