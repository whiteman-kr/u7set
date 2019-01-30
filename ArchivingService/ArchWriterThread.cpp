#include "ArchWriterThread.h"
#include "../lib/Crc16.h"

// -----------------------------------------------------------------------------------------------------------------------
//
// FileArchWriter class implementation
//
// -----------------------------------------------------------------------------------------------------------------------

ArchWriterThread::ArchWriterThread(Archive* archive,
									CircularLoggerShared logger) :
	m_archive(archive),
	m_log(logger)
{
	TEST_PTR_RETURN(archive);
}

void ArchWriterThread::run()
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

	qint64 prevFlushedStatesCount = 0;

	int filesCount = m_archive->getFilesCount();

	int noFlushingExecuted = 0;

	QElapsedTimer t;

	t.start();

	int sleepTime = 0;

	do
	{
		if (filesCount == 0)
		{
			msleep(5);
			continue;
		}

		updateCurrentPartition();

		bool flushAnyway = false;

		ArchFile* fileToFlush = m_archive->getNextFileForFlushing(&flushAnyway);

		if (fileToFlush == nullptr)
		{
			msleep(1);
		}
		else
		{
			bool flushingExecuted = fileToFlush->flush(m_curPartition, &m_totalFlushedStatesCount, flushAnyway);

			if (flushingExecuted == false)
			{
				noFlushingExecuted++;

				if (noFlushingExecuted >= filesCount)
				{
					msleep(5);
					sleepTime += 5;
					noFlushingExecuted = 0;
				}
			}
			else
			{
				noFlushingExecuted = 0;
			}
		}

		if (m_totalFlushedStatesCount - prevFlushedStatesCount > 1000)
		{
			qint64 dn = m_totalFlushedStatesCount - prevFlushedStatesCount;

			prevFlushedStatesCount = m_totalFlushedStatesCount;

			int tel = t.elapsed() - sleepTime;

//			qDebug() << "Flush states" << dn << ", time" << tel << ", per state" << (tel * 1000) / dn << "mcs";

			t.start();
			sleepTime = 0;
		}
	}
	while(isQuitRequested() == false);

	m_archive->shutdown();
}

void ArchWriterThread::updateCurrentPartition()
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

		// needs runArchiveMaintenance(); here !!!
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
