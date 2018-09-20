#include "FileArchWriter.h"


ArchFile::ArchFile()
{
}

bool ArchFile::init(const FileArchWriter* writer, const QString& signalID, Hash hash, int initialQueueSize)
{
	TEST_PTR_RETURN_FALSE(writer);

	m_archWriter = writer;
	m_signalID = signalID;
	m_hash = hash;

	m_queue = new LockFreeQueue<SignalState>(initialQueueSize);

	return true;

}

bool ArchFile::pushState(qint64 archID, const SimpleAppSignalState& state)
{
	return true;
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

	bool result = initFiles();

	if (result == false)
	{
		DEBUG_LOG_ERR(m_log, "FileArchWriter terminated (files initialization error). ");
		return;
	}

	do
	{

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


		//writeArchInfoFile();
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

		int initialQueueSize = DISCRETES_INITIAL_QUEUE_SIZE;

		if (archSignal.isAnalog == true)
		{
			initialQueueSize = ANALOGS_INITIAL_QUEUE_SIZE;
		}

		ArchFile* archFile = m_archFiles + index;

		result &= archFile->init(this, signalID, hash, initialQueueSize);

		m_hashArchFiles.insert(hash, archFile);

		index++;
	}

	return result;
}


void FileArchWriter::shutdown()
{
	if (m_archFiles != nullptr)
	{
		delete [] m_archFiles;
		m_archFiles = nullptr;
	}
}


