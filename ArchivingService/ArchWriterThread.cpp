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

	int filesCount = m_archive->getFilesCount();

	int noFlushingExecuted = 0;

	m_statisticsTimer.start();

	while(isQuitRequested() == false)
	{
		if (filesCount == 0)
		{
			sleepMs(200);
			continue;
		}

		qint64 currentPartition = m_archive->getCurrentPartition();

		printStatistics(currentPartition);

		bool flushAnyway = false;

		ArchFile* fileToFlush = m_archive->getNextFileForFlushing(&flushAnyway);

		if (fileToFlush == nullptr)
		{
			sleepMs(1);
		}
		else
		{
			bool flushingExecuted = fileToFlush->flush(currentPartition, &m_totalFlushedStatesCount, flushAnyway);

			if (flushingExecuted == false)
			{
				noFlushingExecuted++;

				if (noFlushingExecuted >= filesCount)
				{
					sleepMs(5);
					noFlushingExecuted = 0;
				}
			}
			else
			{
				noFlushingExecuted = 0;
			}
		}
	}

	m_archive->shutdown();
}

void ArchWriterThread::printStatistics(qint64 currentPartition)
{
	if (m_prevPartition == -1)
	{
		m_prevPartition = currentPartition;
		m_prevFlushedStatesCount = m_totalFlushedStatesCount;
		return;
	}

	const qint64 TEN_SECONDS = 10 * 1000;

	if (currentPartition - m_prevPartition > TEN_SECONDS)
	{
		qint64 dn = m_totalFlushedStatesCount - m_prevFlushedStatesCount;

		m_prevFlushedStatesCount = m_totalFlushedStatesCount;

		qint64 elapsedTime = m_statisticsTimer.elapsed();

		qint64 dt_clear = elapsedTime - m_sleepTime;

		qint64 sleepPercent = m_sleepTime * 100 / elapsedTime;

		qDebug() << C_STR(QString("Flush states %1 time %2 (sleep %3%) (per state %4 mcs)").
						  arg(dn).arg(elapsedTime).arg(sleepPercent).arg((dt_clear * 1000) / dn));

		m_statisticsTimer.start();
		m_sleepTime = 0;
	}
}

void ArchWriterThread::sleepMs(int ms)
{
	msleep(ms);
	m_sleepTime += ms;
}


