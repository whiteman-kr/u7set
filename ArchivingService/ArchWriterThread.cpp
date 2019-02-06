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
			msleep(100);
			continue;
		}

		qint64 currentPartition = m_archive->getCurrentPartition();

		bool flushAnyway = false;

		ArchFile* fileToFlush = m_archive->getNextFileForFlushing(&flushAnyway);

		if (fileToFlush == nullptr)
		{
			msleep(1);
		}
		else
		{
			bool flushingExecuted = fileToFlush->flush(currentPartition, &m_totalFlushedStatesCount, flushAnyway);

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

		if (m_totalFlushedStatesCount - prevFlushedStatesCount > 10000)
		{
			qint64 dn = m_totalFlushedStatesCount - prevFlushedStatesCount;

			prevFlushedStatesCount = m_totalFlushedStatesCount;

			qint64 elapsedTime = t.elapsed();

			qint64 dt_clear = elapsedTime - sleepTime;

			qint64 sleepPercent = sleepTime * 100 / elapsedTime;

			qDebug() << C_STR(QString("Flush states %1 time %2 (sleep %3%) (per state %4 mcs)").
							  arg(dn).arg(elapsedTime).arg(sleepPercent).arg((dt_clear * 1000) / dn));

			t.start();
			sleepTime = 0;
		}
	}
	while(isQuitRequested() == false);

	m_archive->shutdown();
}

