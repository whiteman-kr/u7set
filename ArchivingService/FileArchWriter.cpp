#include "FileArchWriter.h"


ArchFile::SignalState ArchFile::m_buffer[ArchFile::QUEUE_MAX_SIZE];


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

	m_queue = new FastQueue<SignalState>(queueSize);

	m_path = QString("%1/%2/%3").
					arg(writer->archFullPath()).
					arg(QString().sprintf("%02X", static_cast<int>(m_hash & 0xFF))).
					arg(m_signalID.remove("#"));

	return true;
}

bool ArchFile::pushState(qint64 archID, const SimpleAppSignalState& state)
{
	if (state.hash != m_hash)
	{
		assert(false);
		return false;
	}

	TEST_PTR_RETURN_FALSE(m_queue);

	SignalState s;

	s.state.archID = archID;
	s.state.plant = state.time.plant.timeStamp;
	s.state.system = state.time.system.timeStamp;
	s.state.flags = state.flags;
	s.state.value = state.value;

	m_queue->push(s);

	return true;
}

void ArchFile::flush(qint64 curPartition)
{
	TEST_PTR_RETURN(m_queue);

	int copiedItemsCount = 0;

	bool result = m_queue->copyToBuffer(m_buffer, QUEUE_MAX_SIZE, &copiedItemsCount);

	if (result == false || copiedItemsCount == 0)
	{
		return;
	}

	writeFile(curPartition, m_buffer, copiedItemsCount);
}

bool ArchFile::isEmergency() const
{
	TEST_PTR_RETURN_FALSE(m_queue);

	return m_queue->size() >= static_cast<int>(m_queue->queueSize() * QUEUE_EMERGENCY_LIMIT);
}

bool ArchFile::writeFile(qint64 partition, SignalState* buffer, int statesCount)
{
	TEST_PTR_RETURN_FALSE(buffer);

	if (m_fileIsOpened == false)
	{
		if (m_pathIsExists == false)
		{
			QDir d;

			m_pathIsExists = d.mkpath(m_path);
		}

		QDateTime date = QDateTime::fromMSecsSinceEpoch(partition, Qt::UTC);

		QString fileName = QString("%1/%2_%3_%4_%5_%6.dat").
								arg(m_path).
								arg(date.date().year()).
								arg(QString().sprintf("%02d", date.date().month())).
								arg(QString().sprintf("%02d", date.date().day())).
								arg(QString().sprintf("%02d", date.time().hour())).
								arg(QString().sprintf("%02d", date.time().minute()));

		m_file.setFileName(fileName);

		if (m_file.open(QIODevice::Append) == false)
		{
			return false;
		}

		m_fileIsOpened = true;

		if (m_fileIsAligned == false)
		{
			QFileInfo fi(m_file);

			qint64 fileSize = fi.size();

			if ((fileSize % sizeof(SignalState)) != 0)
			{
				m_file.seek((fileSize / sizeof(SignalState)) * sizeof(SignalState));
			}

			m_fileIsAligned = true;
		}
	}

	qint64 sizeToWrite = statesCount * sizeof(SignalState);

	qint64 written = m_file.write(reinterpret_cast<const char*>(buffer), sizeToWrite);

	if (written == -1)
	{
		return false;
	}

	if (sizeToWrite != written)
	{
		m_fileIsAligned = false;
	}

	return true;
}

void ArchFile::close()
{
	assert(m_fileIsOpened == true);

	m_file.close();

	m_fileIsOpened = false;
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
			msleep(5);
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
			addEmergencyFile(archFile);
		}

		count++;
	}
	while(count < 100000);

	if (count > 0)
	{
		qDebug() << C_STR(QString("%1 states processed").arg(count));
	}

	return count > 0;
}

void FileArchWriter::updateCurrentPartition()
{
	const int PARTITTION_DIVIDER = 24 * 60 * 60 * 1000;			// each day

	qint64 curPartition = (QDateTime::currentMSecsSinceEpoch() / PARTITTION_DIVIDER) * PARTITTION_DIVIDER;

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

void FileArchWriter::runArchiveMaintenance()
{
	m_archMaintenanceIsRunning = true;
}

bool FileArchWriter::writeEmergencyFiles()
{
	int count = 0;

	do
	{
		ArchFile* emergencyFile = getNextEmergencyFile();

		if (emergencyFile == nullptr)
		{
			break;
		}

		emergencyFile->flush(m_curPartition);

		count++;
	}
	while(count < 200);

	return count > 0;
}

bool FileArchWriter::writeRegularFiles()
{
	if (m_regularArchFileIndex >= m_archFilesCount)
	{
		m_regularArchFileIndex = 0;
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
			archFile->flush(m_curPartition);
		}

		count++;
	}
	while(count < 100);

	return true;
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
	if (m_archFiles != nullptr)
	{
		delete [] m_archFiles;
		m_archFiles = nullptr;
	}
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

/*
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
*/

