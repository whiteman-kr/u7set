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

	m_timer.start();

	while(isQuitRequested() == false)
	{
		if (filesCount == 0)
		{
			msleep(200);
			continue;
		}

		printStatistics();

		bool flushAnyway = false;

		ArchFile* fileToFlush = m_archive->getNextFileForFlushing(&flushAnyway);

		if (fileToFlush == nullptr)
		{
			msleep(1);
		}
		else
		{
			qint64 currentPartition = m_archive->getCurrentPartition();

			qint64 startTime = m_timer.elapsed();

			bool flushingExecuted = fileToFlush->flush(currentPartition, &m_totalFlushedStatesCount, flushAnyway);

			m_flushTime += m_timer.elapsed() - startTime;

			if (flushingExecuted == false)
			{
				noFlushingExecuted++;

				if (noFlushingExecuted >= filesCount)
				{
					msleep(5);
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

void ArchWriterThread::printStatistics()
{
	const qint64 TEN_SECONDS = 10 * 1000;

	qint64 elapsedTime = m_timer.elapsed();

	if (elapsedTime < TEN_SECONDS)
	{
		return;
	}

	if (m_totalFlushedStatesCount == m_prevFlushedStatesCount)
	{
		qDebug() << "No flushing";
		return;
	}

	qint64 flushedCount = m_totalFlushedStatesCount - m_prevFlushedStatesCount;

	m_prevFlushedStatesCount = m_totalFlushedStatesCount;

	qint64 sleepPercent = 100 - (m_flushTime * 100) / elapsedTime;

	qDebug() << C_STR(QString("Flush states %1 time %2 (sleep %3%) (per state %4 mcs)").
						  arg(flushedCount).arg(elapsedTime).arg(sleepPercent).arg((m_flushTime * 1000) / flushedCount));

	m_timer.start();
	m_flushTime = 0;
}

