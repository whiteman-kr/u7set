#include "FileArchWriter.h"
#include "../lib/Crc16.h"

// -----------------------------------------------------------------------------------------------------------------------
//
// FileArchWriter class implementation
//
// -----------------------------------------------------------------------------------------------------------------------

FileArchWriter::FileArchWriter(ArchiveShared archive,
								CircularLoggerShared logger) :
	m_archive(archive),
	m_log(logger)
{
	TEST_PTR_RETURN(archive);

	m_saveStatesQueue = &archive->saveStatesQueue();
}

bool FileArchWriter::flushFileBeforeReading(Hash signalHash, QString* filePath)
{
	if (filePath == nullptr)
	{
		assert(false);
		return false;
	}

	ArchFile* archFile = m_hashArchFiles.value(signalHash, nullptr);

	if (archFile == nullptr)
	{
		return false;
	}

	qint64 flashedCount = 0;

	archFile->flush(m_curPartition, &flashedCount);

	*filePath = archFile->path();

	return true;
}

void FileArchWriter::run()
{
	if (m_log == nullptr)
	{
		assert(false);
		return;
	}

	if (m_archive == nullptr)
	{
		DEBUG_LOG_ERR(m_log, "FileArchWriter terminated (parameters error). ");
		return;
	}

	m_thisThread = QThread::currentThread();

	bool result = initFiles();

	if (result == false)
	{
		DEBUG_LOG_ERR(m_log, "FileArchWriter terminated (files initialization error). ");
		return;
	}

	qint64 prevFlushedStatesCount = 0;

	do
	{
		bool doWork = false;

		doWork |= processSaveStatesQueue();

		if (isQuitRequested() == true)
		{
			break;
		}

		updateCurrentPartition();

		doWork |= writeEmergencyFiles();

		if (isQuitRequested() == true)
		{
			break;
		}

		doWork |= writeRegularFiles();

		if (isQuitRequested() == true)
		{
			break;
		}

		doWork |= archiveMaintenance();

		if (isQuitRequested() == true)
		{
			break;
		}

		if (doWork == false)
		{
			msleep(2);
		}

		if (m_totalFlushedStatesCount - prevFlushedStatesCount > 1000)
		{
			prevFlushedStatesCount = m_totalFlushedStatesCount;
			qDebug() << "Flush states" << m_totalFlushedStatesCount;
		}
	}
	while(isQuitRequested() == false);

	shutdown();
}


bool FileArchWriter::initFiles()
{
	bool result = archDirIsWritableChecking();

	if (result == false)
	{
		return false;
	}

	result = createGroupDirs();

	if (result == false)
	{
		return false;
	}

	result = createArchFiles();

	if (result == false)
	{
		return false;
	}

	return true;
}

bool FileArchWriter::archDirIsWritableChecking()
{
	int pass = 1;

	bool result = false;

	do
	{
		if (pass == 1)
		{
			m_archFullPath = m_archive->archFullPath();
		}
		else
		{
			m_archFullPath = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::AppDataLocation);
		}

		m_archFullPath = QDir(m_archFullPath).absolutePath();

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

bool FileArchWriter::createGroupDirs()
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

bool FileArchWriter::createArchFiles()
{
	bool result = true;

	const QHash<Hash, ArchSignal>& archSignals = m_archive->archSignals();

	m_archFiles = new ArchFile[archSignals.size()];

	TEST_PTR_RETURN_FALSE(m_archFiles);

	int index = 0;

	for(const ArchSignal& archSignal : archSignals)
	{
		Hash hash = archSignal.hash;

		if (m_hashArchFiles.contains(hash) == true)
		{
			assert(false);
			continue;
		}

		QString signalID = m_archive->getSignalID(hash);

		if (signalID.isEmpty() == true)
		{
			assert(false);
			continue;
		}

		ArchFile* archFile = m_archFiles + index;

		result &= archFile->init(this, signalID, hash, archSignal.isAnalog);

		m_hashArchFiles.insert(hash, archFile);

		index++;
	}

	m_archFilesCount = index;

	return result;
}

bool FileArchWriter::processSaveStatesQueue()
{
	TEST_PTR_RETURN_FALSE(m_saveStatesQueue);

	SimpleAppSignalState state;

	int count = 0;

	do
	{
		bool result = m_saveStatesQueue->pop(&state);

		if (result == false)
		{
			break;
		}

		ArchFile* archFile = m_hashArchFiles.value(state.hash, nullptr);

		if (archFile == nullptr)
		{
			assert(false);
			continue;
		}

		archFile->pushState(m_archID, state);

		m_archID++;

		if (archFile->isEmergency() == true)
		{
			addEmergencyFile(archFile);
		}

		count++;
	}
	while(count < 10000);

	return count > 0;
}

void FileArchWriter::updateCurrentPartition()
{
	// const int PARTITTION_DIVIDER = 24 * 60 * 60 * 1000;			// each day
	const int PARTITTION_DIVIDER = 60 * 1000;						// each minute

	qint64 curPartition = (QDateTime::currentMSecsSinceEpoch() / PARTITTION_DIVIDER) * PARTITTION_DIVIDER;

/*	const int MINUTE_TIME = 60 * 1000;

	if (curPartition / MINUTE_TIME != m_curPartition / MINUTE_TIME)
	{
		writeMinuteCheckpoint((curPartition / MINUTE_TIME) * MINUTE_TIME);		// trunc system time to minutes
	}*/

	if (m_curPartition == curPartition)
	{
		return;
	}

	if (m_curPartition == -1)
	{
		m_curPartition = curPartition;
	}
	else
	{
		m_curPartition = curPartition;

		runArchiveMaintenance();
	}
}

bool FileArchWriter::writeMinuteCheckpoint(qint64 minuteSystemTime)
{
	QString fileName = m_archFullPath + "/mincheckpoints.dat";

	QFile minuteCheckpointsFile(fileName);

	if (minuteCheckpointsFile.open(QIODevice::ReadWrite) == false)
	{
		DEBUG_LOG_ERR(m_log, "Can't open minutes checkpoint file");
		return false;
	}

	QFileInfo fi(minuteCheckpointsFile);

	qint64 writePos = (fi.size() / sizeof(MinuteCheckpoint)) * sizeof(MinuteCheckpoint);

	minuteCheckpointsFile.seek(writePos);

	MinuteCheckpoint mc;

	mc.checkpoint.minuteSystemTime = minuteSystemTime;
	mc.checkpoint.archiveID = m_archID;

	m_archID++;

	mc.crc16 = calcCrc16(&mc.checkpoint, sizeof(mc.checkpoint));

	minuteCheckpointsFile.write(reinterpret_cast<const char*>(&mc), sizeof(mc));

	minuteCheckpointsFile.close();

	return true;
}


void FileArchWriter::runArchiveMaintenance()
{
	m_archMaintenanceIsRunning = true;
}

bool FileArchWriter::writeEmergencyFiles()
{
	int count = 0;
	int flushedCount = 0;

	do
	{
		ArchFile* emergencyFile = getNextEmergencyFile();

		if (emergencyFile == nullptr)
		{
			break;
		}

		bool res = emergencyFile->flush(m_curPartition, &m_totalFlushedStatesCount);

		if (res == true)
		{
			flushedCount++;
		}

		count++;
	}
	while(count < 200);

	return flushedCount > 0;
}

bool FileArchWriter::writeRegularFiles()
{
	if (m_regularArchFileIndex >= m_archFilesCount)
	{
		m_regularArchFileIndex = 0;
	}

	int count = 0;
	int flashedCount = 0;

	do
	{
		ArchFile* archFile = m_archFiles + m_regularArchFileIndex;

		m_regularArchFileIndex++;

		if (m_regularArchFileIndex >= m_archFilesCount)
		{
			m_regularArchFileIndex = 0;
		}

		if (archFile != nullptr)
		{
			bool res = archFile->flush(m_curPartition, &m_totalFlushedStatesCount);

			if (res == true)
			{
				flashedCount++;
			}
		}

		count++;
	}
	while(count < 100);

	return flashedCount > 0;
}

bool FileArchWriter::archiveMaintenance()
{
	if (m_archMaintenanceIsRunning == false)
	{
		return false;
	}

	// do real work here !

	return true;
}

void FileArchWriter::shutdown()
{
	TEST_PTR_RETURN(m_archFiles);

	for(int i = 0; i < m_archFilesCount; i++)
	{
		ArchFile* archFile = m_archFiles + i;

		archFile->shutdown(m_curPartition, &m_totalFlushedStatesCount);
	}

	delete [] m_archFiles;
	m_archFiles = nullptr;
}

void FileArchWriter::addEmergencyFile(ArchFile* file)
{
//	takeEmergencyFilesOwnership(thread);

	if (m_emergencyFilesInQueue.contains(file) == false)
	{
		m_emergencyFilesQueue.append(file);
		m_emergencyFilesInQueue.insert(file, true);
	}

//	releaseEmergencyFilesOwnership(thread);
}

ArchFile* FileArchWriter::getNextEmergencyFile()
{
	ArchFile* emergencyFile = nullptr;

//	takeEmergencyFilesOwnership(thread);

	if (m_emergencyFilesQueue.isEmpty() == false)
	{
		emergencyFile = m_emergencyFilesQueue.first();
		m_emergencyFilesQueue.removeFirst();
		m_emergencyFilesInQueue.remove(emergencyFile);
	}

//	releaseEmergencyFilesOwnership(thread);

	return emergencyFile;
}

