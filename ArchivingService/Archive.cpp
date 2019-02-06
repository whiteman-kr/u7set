#include "../lib/WUtils.h"

#include "Archive.h"
#include "ArchFile.h"
#include "ArchRequest.h"
#include "ArchWriterThread.h"

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

QString Archive::formatTime(qint64 time)
{
	return QDateTime::fromMSecsSinceEpoch(time, Qt::TimeSpec::UTC).toString("yyyy-MM-dd HH:mm:ss");
}

Archive::Archive(const QString& projectID,
				 const QString& equipmentID,
				 const QString& archDir,
				 const Proto::ArchSignals& protoArchSignals,
				 int shortTermPeriod,
				 int longTermPeriod,
				 CircularLoggerShared logger) :
	m_projectID(projectID),
	m_equipmentID(equipmentID),
	m_archDir(archDir),
	m_shortTermPeriod(shortTermPeriod),
	m_longTermPeriod(longTermPeriod),
	m_log(logger)
{
	if (m_shortTermPeriod < 2)
	{
		m_shortTermPeriod = 2;
	}

	if (m_shortTermPeriod >= m_longTermPeriod)
	{
		m_longTermPeriod = m_shortTermPeriod + 1;
	}

	// period to milliseconds conversation
	//
	m_shortTermPeriod *= PARTITION_PERIOD_MS;
	m_longTermPeriod *= PARTITION_PERIOD_MS;

	int signalsCount = protoArchSignals.archsignals_size();

	m_archFiles.reserve(static_cast<int>(signalsCount * 1.2));
	m_archFilesArray.resize(signalsCount);
	m_regularFilesQueue.reserve(static_cast<int>(signalsCount * 1.2));

	for(int i = 0; i < signalsCount; i++)
	{
		const Proto::ArchSignal& protoArchSignal = protoArchSignals.archsignals(i);

		ArchFile* archFile = new ArchFile(protoArchSignal);

		m_archFiles.insert(archFile->hash(), archFile);

		m_archFilesArray[i] = archFile;

		m_regularFilesQueue.append(archFile);
	}
}

Archive::~Archive()
{
	clear();
}

void Archive::start()
{
	m_isWorkable = false;

	bool result = checkAndCreateArchiveDirs();

	if (result == false)
	{
		DEBUG_LOG_ERR(m_log, "Archive directories creation error");
		return;
	}

	for(ArchFile* archFile : m_archFilesArray)
	{
		if (archFile == nullptr)
		{
			assert(false);
			continue;
		}

		archFile->setArchFullPath(m_archFullPath);
	}

	assert(m_archWriterThread == nullptr);

	m_archWriterThread = new ArchWriterThread(this, m_log);
	m_archWriterThread->start();

	assert(m_archMaintenanceThread == nullptr);

	m_archMaintenanceThread = new ArchMaintenanceThread(*this, m_log);
	m_archMaintenanceThread->start();

	m_isWorkable = true;
}

void Archive::stop()
{
	m_requestsMutex.lock();

	assert(m_requests.count() == 0);

	for(ArchRequestShared archRequest : m_requests)
	{
		if (archRequest != nullptr)
		{
			archRequest->quitAndWait();
		}
	}

	m_requests.clear();

	m_requestsMutex.unlock();

	//

	if (m_archWriterThread != nullptr)
	{
		m_archWriterThread->quitAndWait();
		delete m_archWriterThread;
		m_archWriterThread = nullptr;
	}

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

bool Archive::canReadWriteSignal(Hash signalHash)
{
	ArchFile* archFile = m_archFiles.value(signalHash, nullptr);

	if (archFile == nullptr)
	{
		assert(false);
		return false;
	}

	return archFile->canReadWrite();
}

void Archive::setCanReadWriteSignal(Hash signalHash, bool canReadWrite)
{
	ArchFile* archFile = m_archFiles.value(signalHash, nullptr);

	if (archFile == nullptr)
	{
		assert(false);
		return;
	}

	archFile->setCanReadWrite(canReadWrite);
}

void Archive::setSignalInitialized(Hash signalHash, bool initilaized)
{
	ArchFile* archFile = m_archFiles.value(signalHash, nullptr);

	if (archFile == nullptr)
	{
		assert(false);
		return;
	}

	archFile->setInitialized(initilaized);
}

void Archive::getArchSignalStatus(Hash signalHash, bool* canReadWrite, bool* isInitialized, bool* isAnalog)
{
	TEST_PTR_RETURN(canReadWrite);
	TEST_PTR_RETURN(isInitialized);
	TEST_PTR_RETURN(isAnalog);

	ArchFile* archFile = m_archFiles.value(signalHash, nullptr);

	if (archFile == nullptr)
	{
		*canReadWrite = false;
		*isInitialized = false;
		*isAnalog = false;
		return;
	}

	*canReadWrite = archFile->canReadWrite();
	*isInitialized = archFile->isInitialized();
	*isAnalog = archFile->isAnalog();
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

void Archive::saveState(const SimpleAppSignalState& state)
{
	ArchFile* archFile = m_archFiles.value(state.hash, nullptr);

	if (archFile == nullptr)
	{
		assert(false);
		return;
	}

	m_archID++;

	archFile->pushState(m_archID, state);

	if (archFile->isEmergency() == true)
	{
		appendEmergencyFile(archFile);
	}
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
	while(1);*/

	return true;
}

bool Archive::flushImmediately(ArchFile* archFile)
{
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

bool Archive::startMaintenance()
{
	assert(isMaintenanceRequired() == true);

	m_isMaintenanceRequired.store(false);

	if (m_archFilesArray.count() > 0)
	{
		m_maintenanceFileIndex = 0;
		return true;
	}

	return false;
}

bool Archive::continueMaintenance(int* deletedCount, int* packedCount)
{
	if (m_maintenanceFileIndex < 0 || m_maintenanceFileIndex >= m_archFilesArray.count())
	{
		m_maintenanceFileIndex = -1;
		return false;
	}

	ArchFile* archFile = m_archFilesArray[m_maintenanceFileIndex];

	archFile->maintenance(getCurrentPartition(), m_shortTermPeriod, m_longTermPeriod, deletedCount, packedCount);

	m_maintenanceFileIndex++;

	return true;
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
	int removed = m_regularFilesQueue.removeAll(file);

	if (removed != 1)
	{
		assert(false);
	}
	else
	{
		m_regularFilesQueue.append(file);
	}
}

void Archive::clear()
{
	m_projectID.clear();
	m_archFiles.clear();

	for(ArchFile* archFile : m_archFiles)
	{
		delete archFile;
	}

	m_archFiles.clear();
	m_archFilesArray.clear();
/*
	for(RequestContext* reqContext : m_requestContexts)
	{
		delete reqContext;
	}

	m_requestContexts.clear();*/
}
