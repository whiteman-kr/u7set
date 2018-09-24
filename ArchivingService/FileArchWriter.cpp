#include "FileArchWriter.h"


ArchFile::ArchFile()
{
}

bool ArchFile::init(const FileArchWriter* writer, const QString& signalID, Hash hash, bool isAnalogSignal)
{
	TEST_PTR_RETURN_FALSE(writer);

	m_archWriter = writer;
	m_signalID = signalID;
	m_hash = hash;

	int queueSize = QUEUE_MIN_SIZE;

	if (isAnalogSignal == true)
	{
		queueSize = QUEUE_MIN_SIZE * 16;
	}

	m_queue = new LockFreeQueue<SignalState>(queueSize);

	return true;

}

bool ArchFile::pushState(qint64 archID, const SimpleAppSignalState& state)
{
	if (state.hash != m_hash)
	{
		assert(false);
		return false;
	}

	if (m_queue == nullptr)
	{
		assert(false);
		return false;
	}

	SignalState s;

	s.state.archID = archID;
	s.state.plant = state.time.plant.timeStamp;
	s.state.system = state.time.system.timeStamp;
	s.state.flags = state.flags;
	s.state.value = state.value;

	m_queue->push(&s);

	return true;
}

void ArchFile::flush()
{

}

bool ArchFile::isEmergency() const
{
	TEST_PTR_RETURN_FALSE(m_queue);

	return m_queue->size() >= static_cast<int>(m_queue->queueSize() * QUEUE_EMERGENCY_LIMIT);
}


FileArchWriter::FileArchWriter(ArchiveShared archive,
								Queue<SimpleAppSignalState>& saveStatesQueue,
								CircularLoggerShared logger) :
	m_archive(archive),
	m_saveStatesQueue(saveStatesQueue),
	m_log(logger)
{
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

	do
	{
		processSaveStatesQueue();

		if (isQuitRequested() == true)
		{
			break;
		}

		writeEmergencyFiles();

		if (isQuitRequested() == true)
		{
			break;
		}

		writeRegularFiles();
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
	SimpleAppSignalState state;

	int count = 0;

	do
	{
		bool result = m_saveStatesQueue.pop(&state);

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
			addEmergencyFile(archFile, thread);
		}

		count++;
	}
	while(count < 10000);

	return true;
}

bool FileArchWriter::writeEmergencyFiles()
{
	int count = 0;

	do
	{
		ArchFile* emergencyFile = getNextEmergencyFile(m_thisThread);

		if (emergencyFile != nullptr)
		{
			emergencyFile->flush();
		}

		count++;
	}
	while(count < 200);

	return true;
}

bool FileArchWriter::writeRegularFiles()
{
	if (m_regularArchFileIndex >= m_archFilesCount)
	{
		return false;
	}

	int count = 0;

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
			archFile->flush();
		}

		count++;
	}
	while(count < 100);

	return true;
}

void FileArchWriter::shutdown()
{
	if (m_archFiles != nullptr)
	{
		delete [] m_archFiles;
		m_archFiles = nullptr;
	}
}

void FileArchWriter::addEmergencyFile(ArchFile* file, const QThread* thread)
{
	takeEmergencyFilesOwnership(thread);

	if (m_emergencyFilesInQueue.contains(file) == false)
	{
		m_emergencyFilesQueue.append(file);
		m_emergencyFilesInQueue.insert(file, true);
	}

	releaseEmergencyFilesOwnership(thread);
}

ArchFile* FileArchWriter::getNextEmergencyFile(const QThread* thread)
{
	ArchFile* emergencyFile = nullptr;

	takeEmergencyFilesOwnership(thread);

	if (m_emergencyFilesQueue.isEmpty() == false)
	{
		emergencyFile = m_emergencyFilesQueue.first();
		m_emergencyFilesQueue.removeFirst();
		m_emergencyFilesInQueue.remove(emergencyFile);
	}

	releaseEmergencyFilesOwnership(thread);

	return emergencyFile;
}


void FileArchWriter::takeEmergencyFilesOwnership(const QThread* newOwner)
{
	bool result = false;

	do
	{
		const QThread* expectedOwner = nullptr;
		result = m_emergencyFilesOwner.compare_exchange_strong(expectedOwner, newOwner);
	}
	while(result == false);
}

void FileArchWriter::releaseEmergencyFilesOwnership(const QThread* currentOwner)
{
	bool result = m_emergencyFilesOwner.compare_exchange_strong(currentOwner, nullptr);

	assert(result == true);

	Q_UNUSED(result);
}


