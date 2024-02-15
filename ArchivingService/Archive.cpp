#include "Archive.h"
#include "../Proto/ArchSignal.pb.h"
#include "../UtilsLib/WUtils.h"
#include "../lib/ConstStrings.h"
#include "ArchFile.h"
#include "ArchRequest.h"
#include "ArchWriterThread.h"
#include <QDir>
#include <QStandardPaths>

// ----------------------------------------------------------------------------------------------------------------------
//
// ArchRequestParam class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

ArchRequestParam::ArchRequestParam(quint32 requestID, E::TimeType timeType, qint64 startTime,
								   qint64 endTime, const QVector<Hash>& signalHashes) :
	m_requestID(requestID),
	m_timeType(timeType),
	m_startTime(startTime),
	m_endTime(endTime),
	m_signalHashes(signalHashes)
{
}

void ArchRequestParam::expandTimes(qint64 expandTime)
{
	m_startTime -= expandTime;

	if (m_startTime < 0)
	{
		assert(false);
		m_startTime = 0;
	}

	m_endTime += expandTime;
}

QString ArchRequestParam::print()
{
	return QString("ID=%1, timeType=%2, startTime=%3, endTime=%4, signals=%5").
				arg(m_requestID).
				arg(Archive::timeTypeStr(m_timeType)).
				arg(Archive::formatTime(m_startTime)).
				arg(Archive::formatTime(m_endTime)).
				arg(m_signalHashes.count());
}

// ----------------------------------------------------------------------------------------------------------------------
//
// Archive class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

std::atomic<quint32> Archive::m_nextRequestID = { 1 };

const int Archive::MIN_SHORT_TERM_PERIOD_DAYS;

const int Archive::MIN_MAINTENANCE_DELAY_MINUTES;
const int Archive::MAX_MAINTENANCE_DELAY_MINUTES;

const int Archive::MIN_QUEUE_SIZE_FOR_FLUSHING;
const int Archive::MAX_QUEUE_SIZE_FOR_FLUSHING;

QString Archive::formatTime(qint64 time)
{
	return QDateTime::fromMSecsSinceEpoch(time, Qt::TimeSpec::UTC).toString("yyyy-MM-dd HH:mm:ss");
}

Archive::Archive(const QString& projectID,
				 const QString& equipmentID,
				 const QString& archDir,
				 QByteArray& archFileInfoData,
				 int shortTermPeriod,
				 int longTermPeriod,
				 int maintenanceDelayMinutes,
				 int minQueueSizeForFlushing,
				 CircularLoggerShared logger) :
	m_readOnlyArchive(false),
	m_projectID(projectID),
	m_equipmentID(equipmentID),
	m_archDir(archDir),
	m_maintenanceDelayMinutes(maintenanceDelayMinutes),
	m_minQueueSizeForFlushing(minQueueSizeForFlushing),
	m_log(logger),
	m_archInfoFileData(new QByteArray)
{
	m_archInfoFileData->swap(archFileInfoData);

	// shortTermPeriod and longTermPeriod limitation
	//
	shortTermPeriod = std::max(shortTermPeriod, MIN_SHORT_TERM_PERIOD_DAYS);
	longTermPeriod = std::max(shortTermPeriod + 1, longTermPeriod);

	// m_maintenanceDelayMinutes limitation
	//
	m_maintenanceDelayMinutes = std::max(m_maintenanceDelayMinutes, MIN_MAINTENANCE_DELAY_MINUTES);
	m_maintenanceDelayMinutes = std::min(m_maintenanceDelayMinutes, MAX_MAINTENANCE_DELAY_MINUTES);

	// m_minQueueSizeForFlushing limitation
	//
	m_minQueueSizeForFlushing = std::max(m_minQueueSizeForFlushing, MIN_QUEUE_SIZE_FOR_FLUSHING);
	m_minQueueSizeForFlushing = std::min(m_minQueueSizeForFlushing, MAX_QUEUE_SIZE_FOR_FLUSHING);

	// period from days to milliseconds conversation
	//
	m_msShortTermPeriod = shortTermPeriod * PARTITION_PERIOD_MS;
	m_msLongTermPeriod = longTermPeriod * PARTITION_PERIOD_MS;
}

Archive::Archive(const QString& projectID,					// Read only archive constructor
				 const QString& equipmentID,
				 const QString& readOnlyArchFullPath,
				 QByteArray& archFileInfoData,
				 CircularLoggerShared logger) :
	m_readOnlyArchive(true),
	m_projectID(projectID),
	m_equipmentID(equipmentID),
	m_archInfoFileData(new QByteArray),
	m_log(logger)
{
	m_archInfoFileData->swap(archFileInfoData);

	m_archFullPath = QDir::fromNativeSeparators(readOnlyArchFullPath);
}

Archive::~Archive()
{
	clear();
}

void Archive::start()
{
	m_isWorkable = false;

	if (m_readOnlyArchive == false)
	{
		bool result = checkAndCreateArchiveDirs();		// can set m_readOnlyArchive to true
														// if "readonly" file exists in archive directory!
		if (result == false)
		{
			DEBUG_LOG_ERR(m_log, "Archive directories creation error");
			return;
		}
	}

	if (initArchFiles() == false)
	{
		return;
	}

	if (m_readOnlyArchive == false)
	{
		saveArchInfoProtoFile();

		Q_ASSERT(m_archWriterThread == nullptr);

		m_archWriterThread = new ArchWriterThread(this, m_log);
		m_archWriterThread->start();

		Q_ASSERT(m_archMaintenanceThread == nullptr);

		m_archMaintenanceThread = new ArchMaintenanceThread(*this, m_log);
		m_archMaintenanceThread->start();

		DEBUG_LOG_MSG(m_log, QString("Archive running in READ/WRITE mode!"));
	}
	else
	{
		DEBUG_LOG_MSG(m_log, QString("Archive running in READ ONLY mode!"));
	}

	DELETE_IF_NOT_NULL(m_archInfoFileData);		// no more required

	m_isWorkable = true;
}

void Archive::stop()
{
	DEBUG_LOG_MSG(m_log, QString("Archive is shutting down..."));

	stopAllRequests();
	stopMaintenanceThread();
	stopWriteThread();

	m_isWorkable = false;
}

std::shared_ptr<ArchRequest> Archive::startNewRequest(E::TimeType timeType,
													  qint64 startTime,
													  qint64 endTime,
													  const QVector<Hash>& signalHashes,
													  std::shared_ptr<Network::GetAppSignalStatesFromArchiveNextReply> getNextReply)
{
	m_requestsMutex.lock();

	ArchRequestParam param(getNewRequestID(), timeType, startTime, endTime, signalHashes);

	ArchRequestShared archRequest = std::make_shared<ArchRequest>(*this, param, getNextReply, m_log);

	m_requests.insert(param.requestID(), archRequest);

	m_requestsMutex.unlock();

	archRequest->start();

	return archRequest;
}

void Archive::finalizeRequest(quint32 requestID)
{
	m_requestsMutex.lock();

	ArchRequestShared archRequest = m_requests.value(requestID, nullptr);

	if (archRequest != nullptr)
	{
		archRequest->quitAndWait();

		m_requests.remove(requestID);
	}

	m_requestsMutex.unlock();
}

QString Archive::getSignalID(Hash signalHash)
{
	ArchFile* archFile = m_archFiles.value(signalHash, nullptr);

	if (archFile == nullptr)
	{
		assert(false);
		return QString();
	}

	return archFile->appSignalID();
}

void Archive::getSignalsHashes(QVector<Hash>* hashes)
{
	TEST_PTR_RETURN(hashes);

	hashes->resize(m_archFiles.count());

	int i = 0;

	for(ArchFile* archFile : m_archFiles)
	{
		(*hashes)[i] = archFile->hash();
		i++;
	}
}

void Archive::saveState(const SimpleAppSignalState& state)
{
	if (m_readOnlyArchive == true)
	{
		Q_ASSERT(false);
		return;
	}

	ArchFile* archFile = m_archFiles.value(state.hash, nullptr);

	if (archFile == nullptr)
	{
		assert(false);
		return;
	}

	archFile->pushState(state);

	if (archFile->isEmergency() == true)
	{
		appendEmergencyFile(archFile);
	}
}

bool Archive::shutdown(ArchFileRecord* buffer, int bufferSize, const QThread* thread)
{
	if (m_readOnlyArchive == true)
	{
		return true;
	}

	// shutting down all archive files
	//
	qint64 totalFlushed = 0;

	for(ArchFile* archFile : m_archFilesArray)
	{
		if (archFile == nullptr)
		{
			assert(false);
			continue;
		}

		qint64 curPartition = getCurrentPartition();
		archFile->shutdown(curPartition, &totalFlushed, buffer, bufferSize, thread);
	}

	DEBUG_LOG_MSG(m_log, QString("Archive is shutdowned."));

	return true;
}

bool Archive::flushImmediately(ArchFile* archFile)
{
	if (m_readOnlyArchive == true)
	{
		return true;
	}

	TEST_PTR_RETURN_FALSE(archFile);

	QMutexLocker locker(&m_immedaitelyFlushingMutex);

	if (m_alreadyInRequiredImmediatelyFlushing.contains(archFile))
	{
		return true;
	}

	archFile->setRequiredImmediatelyFlushing(true);

	m_requiredImmediatelyFlushing.append(archFile);
	m_alreadyInRequiredImmediatelyFlushing.insert(archFile, true);

	return true;
}

bool Archive::waitingForImmediatelyFlushing(Hash signalHash, int waitTimeoutSeconds)
{
	if (m_readOnlyArchive == true)
	{
		Q_ASSERT(false);
		return true;
	}

	ArchFile* archFile = m_archFiles.value(signalHash, nullptr);

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

ArchFile* Archive::getNextFileForFlushing(bool* flushAnyway)
{
	if (flushAnyway == nullptr)
	{
		assert(false);
		return nullptr;
	}

	ArchFile* archFile = nullptr;

	// highest priority flushing
	//
	archFile = getNextRequiredImediatelyFlushing();

	if (archFile != nullptr)
	{
		*flushAnyway = true;
		return archFile;
	}

	// high priority flushing
	//
	archFile = getNextEmergencyFile();

	if (archFile != nullptr)
	{
		*flushAnyway = true;
		return archFile;
	}

	// low priority flushing
	//
	*flushAnyway = false;		// ! it is OK

	archFile = getNextRegularFile();

	return archFile;
}

void Archive::maintenanceIsStarted()
{
	assert(isMaintenanceRequired() == true);

	m_isMaintenanceRequired.store(false);
}

qint64 Archive::getCurrentPartition()
{
	qint64 prevPartition = m_currentPartition.load();

	qint64 newPartition = (QDateTime::currentMSecsSinceEpoch() / PARTITION_PERIOD_MS) * PARTITION_PERIOD_MS;

	if (prevPartition != newPartition)
	{
		m_currentPartition.store(newPartition);

		m_isMaintenanceRequired.store(true);
	}

	return newPartition;
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

bool Archive::loadArchInfoFile()
{
	TEST_PTR_RETURN_FALSE(m_archInfoFileData);

	if (m_readOnlyArchive == false)
	{
		Q_ASSERT(false);
		return false;
	}

	m_archInfoFileData->clear();

	QString path = m_archFullPath + Separator::DIR + File::ARCH_INFO_PROTO;

	QFile archInfoFile(path);

	if (archInfoFile.open(QIODeviceBase::ReadOnly | QIODeviceBase::ExistingOnly) == true)
	{
		*m_archInfoFileData = archInfoFile.readAll();
		DEBUG_LOG_MSG(m_log, QString("File has read %1, size %2").arg(path).arg(m_archInfoFileData->size()));
		return true;
	}

	QString pathBak = m_archFullPath + Separator::DIR + File::ARCH_INFO_PROTO_BAK;

	QFile archInfoFileBak(pathBak);

	if (archInfoFileBak.open(QIODeviceBase::ReadOnly | QIODeviceBase::ExistingOnly) == true)
	{
		*m_archInfoFileData = archInfoFileBak.readAll();
		DEBUG_LOG_MSG(m_log, QString("File has read %1, size %2").arg(path).arg(m_archInfoFileData->size()));
		return true;
	}

	DEBUG_LOG_ERR(m_log, QString("Can't open file %1 or %2").arg(path).arg(pathBak));

	return false;
}

bool Archive::initArchFiles()
{
	if (m_archInfoFileData->isEmpty() == true)
	{
		Q_ASSERT(false);
		return false;
	}

	Proto::ArchInfo archInfo;

	bool res = archInfo.ParseFromArray(m_archInfoFileData->constData(), static_cast<int>(m_archInfoFileData->size()));

	if (res == false)
	{
		DEBUG_LOG_ERR(m_log, "File ArchInfo.proto parsing ERROR!")
		return false;
	}

	int signalsCount = archInfo.archsignal_size();

	m_archFiles.reserve(static_cast<int>(signalsCount * 1.2));
	m_archFilesArray.resize(signalsCount);
	m_regularFilesQueue.reserve(static_cast<int>(signalsCount * 1.2));

	std::vector<std::vector<ArchFile*>> archFilesGroups;

	archFilesGroups.resize(256);

	for(int i = 0; i < signalsCount; i++)
	{
		const Proto::ArchSignal& protoArchSignal = archInfo.archsignal(i);

		ArchFile* archFile = new ArchFile(protoArchSignal, m_archFullPath, m_log);

		m_archFiles.insert(archFile->hash(), archFile);

		m_archFilesArray[i] = archFile;

		m_regularFilesQueue.append(archFile);

		//

		archFilesGroups[archFile->group()].push_back(archFile);
	}

	writeArchFilesInfoFile(archFilesGroups);

	return true;
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

	if (m_archDir.trimmed().isEmpty() == true)
	{
		pass = 2;
	}

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

		// is "readonly" file exists checking

		if (QDir().exists(m_archFullPath + Separator::DIR + File::READONLY) == true)
		{
			m_readOnlyArchive = true;
			return true;
		}

		//

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
		QString x16 = (QString("%1").arg(i, 2, 16, Latin1Char::ZERO)).toUpper();

		QString dir = QString("%1/%2").arg(m_archFullPath).arg(x16);

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

bool Archive::saveArchInfoProtoFile() const
{
	QDir dir;

	QString protoFile = m_archFullPath + Separator::DIR + File::ARCH_INFO_PROTO;

	if (dir.exists(protoFile))
	{
		QString protoBakFile = m_archFullPath + Separator::DIR + File::ARCH_INFO_PROTO_BAK;

		if (dir.exists(protoBakFile) == true)
		{
			dir.remove(protoBakFile);
		}

		bool res = dir.rename(protoFile, protoBakFile);

		if (res == true)
		{
			DEBUG_LOG_MSG(m_log, QString("File %1 renamed to %2").arg(protoFile).arg(protoBakFile));
		}
		else
		{
			DEBUG_LOG_ERR(m_log, QString("Error renaming file %1 renamed to %2").arg(protoFile).arg(protoBakFile));
		}
	}

	QFile pf(protoFile);

	if (pf.open(QIODeviceBase::WriteOnly | QIODeviceBase::Truncate) == false)
	{
		DEBUG_LOG_ERR(m_log, QString("Can't open file %1 for writing").arg(protoFile));
		return false;
	}

	qint64 res = pf.write(*m_archInfoFileData);

	if (res == -1)
	{
		DEBUG_LOG_ERR(m_log, QString("Error writing file %1").arg(protoFile));
	}
	else
	{
		DEBUG_LOG_MSG(m_log, QString("File %1 written, size %2 bytes").arg(protoFile).arg(res));
	}

	pf.close();

	return (res != -1);
}

void Archive::writeArchFilesInfoFile(const std::vector<std::vector<ArchFile*>>& archFilesGroups)
{
	QFile infoFile(QString("%1/archive.info").arg(m_archFullPath));

	if (infoFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) == false)
	{
		return;
	}

	QTextStream info(&infoFile);

	for(int g = 0; g < 256; g++)
	{
		info << QString("Group %1\n\n").arg(QString("%1").arg(g, 2, 16, QChar('0')).toUpper());

		qsizetype filesCount = archFilesGroups[g].size();

		if (filesCount == 0)
		{
			info << "No files in group\n\n";
			continue;
		}

		for(qsizetype i = 0; i < filesCount; i++)
		{
			ArchFile* archFile = archFilesGroups[g][i];

			if (archFile == nullptr)
			{
				assert(false);
				continue;
			}

			info << QString("%1 %2 %3\n").
						arg(archFile->isAnalog() ? "A" : "D").
						arg(QString("%1").arg(archFile->hash(), 16, 16, QChar('0')).toUpper()).
						arg(archFile->appSignalID());
		}

		info << "\n";
	}

	infoFile.close();
}

quint32 Archive::getNewRequestID()
{
	return m_nextRequestID.fetch_add(1);
}

ArchFile* Archive::getNextRequiredImediatelyFlushing()
{
	m_immedaitelyFlushingMutex.lock();

	if (m_requiredImmediatelyFlushing.isEmpty() == true)
	{
		m_immedaitelyFlushingMutex.unlock();
		return nullptr;
	}

	ArchFile* archFile = m_requiredImmediatelyFlushing.first();

	m_requiredImmediatelyFlushing.removeAll(archFile);
	m_alreadyInRequiredImmediatelyFlushing.remove(archFile);

	m_immedaitelyFlushingMutex.unlock();

	removeFromEmergencyFiles(archFile);
	pushBackInRegularFilesQueue(archFile);

	return archFile;
}

void Archive::removeFromRequiredImmediatelyFlushing(ArchFile* file)
{
	QMutexLocker locker(&m_immedaitelyFlushingMutex);

	m_requiredImmediatelyFlushing.removeAll(file);
	m_alreadyInRequiredImmediatelyFlushing.remove(file);
}

void Archive::appendEmergencyFile(ArchFile* file)
{
	QMutexLocker locker(&m_emergencyFilesMutex);

	if (m_alreadyInEmergencyFiles.contains(file) == true)
	{
		return;
	}

	m_emergencyFiles.append(file);
	m_alreadyInEmergencyFiles.insert(file, true);
}

ArchFile* Archive::getNextEmergencyFile()
{
	m_emergencyFilesMutex.lock();

	if (m_emergencyFiles.isEmpty() == true)
	{
		m_emergencyFilesMutex.unlock();
		return nullptr;
	}

	ArchFile* archFile = m_emergencyFiles.first();

	m_emergencyFiles.removeAll(archFile);
	m_alreadyInEmergencyFiles.remove(archFile);

	m_emergencyFilesMutex.unlock();

	removeFromRequiredImmediatelyFlushing(archFile);
	pushBackInRegularFilesQueue(archFile);

	return archFile;
}

void Archive::removeFromEmergencyFiles(ArchFile* file)
{
	QMutexLocker locker(&m_emergencyFilesMutex);

	m_emergencyFiles.removeAll(file);
	m_alreadyInEmergencyFiles.remove(file);
}

ArchFile* Archive::getNextRegularFile()
{
	if (m_regularFilesQueue.isEmpty() == true)
	{
		return nullptr;
	}

	ArchFile* archFile = m_regularFilesQueue.first();

	m_regularFilesQueue.removeFirst();
	m_regularFilesQueue.append(archFile);

	return archFile;
}

void Archive::pushBackInRegularFilesQueue(ArchFile* file)
{
	qsizetype removed = m_regularFilesQueue.removeAll(file);

	if (removed != 1)
	{
		assert(false);
	}
	else
	{
		m_regularFilesQueue.append(file);
	}
}

void Archive::stopAllRequests()
{
	m_requestsMutex.lock();

	for(ArchRequestShared archRequest : m_requests)
	{
		if (archRequest != nullptr)
		{
			archRequest->quitAndWait(3 * 1000);
		}
	}

	m_requests.clear();

	DEBUG_LOG_MSG(m_log, QString("All requests is closed."));

	m_requestsMutex.unlock();
}

void Archive::stopMaintenanceThread()
{
	if (m_archMaintenanceThread != nullptr)
	{
		m_archMaintenanceThread->quitAndWait(5 * 1000);
		delete m_archMaintenanceThread;
		m_archMaintenanceThread = nullptr;

		DEBUG_LOG_MSG(m_log, QString("Maintenance thread is stoped."));
	}
}

void Archive::stopWriteThread()
{
	if (m_archWriterThread != nullptr)
	{
		m_archWriterThread->quitAndWait();			// undefined time to all files flushing
		delete m_archWriterThread;
		m_archWriterThread = nullptr;
	}
}

void Archive::clear()
{
	m_projectID.clear();

	for(ArchFile* archFile : m_archFiles)
	{
		delete archFile;
	}

	m_archFiles.clear();
	m_archFilesArray.clear();
}
