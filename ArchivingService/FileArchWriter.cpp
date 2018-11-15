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

	bool result = m_archive->checkAndCreateArchiveDirs();

	if (result == false)
	{
		DEBUG_LOG_ERR(m_log, "FileArchWriter terminated (files initialization error). ");
		return;
	}

	qint64 prevFlushedStatesCount = 0;

	do
	{
		bool doWork = false;

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

	m_archive->shutdown();
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

/*
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
*/

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
		ArchFile* emergencyFile = m_archive->getNextEmergencyFile();

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
	int count = 0;
	int flashedCount = 0;

	do
	{
		ArchFile* archFile = m_archive->getNextRegularFile();

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



