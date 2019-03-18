#include "AppDataProcessingThread.h"
#include "../lib/WUtils.h"

// -------------------------------------------------------------------------------
//
// AppDataProcessingThread2 class implementation
//
// -------------------------------------------------------------------------------


AppDataProcessingThread::AppDataProcessingThread(int number,
												 const AppDataSourcesIP& appDataSourcesIP,
												 const AppDataReceiverThread* appDataReceiver,
												 CircularLoggerShared log) :
	m_number(number),
	m_appDataSourcesIP(appDataSourcesIP),
	m_appDataReceiver(appDataReceiver),
	m_log(log)
{
	assert(appDataReceiver != nullptr);
}

void AppDataProcessingThread::run()
{
	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThread #%1 is started").arg(m_number));

	QThread* thisThread = currentThread();

	while(isQuitRequested() == false)
	{
		bool hasNoDataToProcessing = true;

		for(AppDataSourceShared appDataSource : m_appDataSourcesIP)
		{
			if (appDataSource == nullptr)
			{
				assert(false);
				continue;
			}

			bool result = appDataSource->takeProcessingOwnership(thisThread);

			if (result == false)
			{
				m_failOwnership++;
				continue;
			}

			m_successOwnership++;

			do
			{
				result = appDataSource->processRupFrameTimeQueue(thisThread);

				if (result == false)
				{
					break;
				}

				hasNoDataToProcessing = false;

				appDataSource->parsePacket();

				m_parsedRupPacketCount++;
			}
			while(isQuitRequested() == false);

			appDataSource->releaseProcessingOwnership(thisThread);
		}

		if (hasNoDataToProcessing == true)
		{
			usleep(500);
		}
	}

	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThread #%1 is finished").arg(m_number));
}

// -------------------------------------------------------------------------------
//
// AppDataProcessingThreadsPool class implementation
//
// -------------------------------------------------------------------------------

void AppDataProcessingThreadsPool::startProcessingThreads(int poolSizeFromSettings,
														  const AppDataSourcesIP& appDataSourcesIP,
														  const AppDataReceiverThread* appDataReceiver,
														  CircularLoggerShared log)
{
	assert(count() == 0);

	int poolSize = poolSizeFromSettings;

	int idealThreadCount = QThread::idealThreadCount();

	if (poolSize <= 0 || poolSize > idealThreadCount)
	{
		poolSize = idealThreadCount;
	}

	for(int i = 0; i < poolSize; i++)
	{
		AppDataProcessingThread* processingThread = new AppDataProcessingThread(i + 1, appDataSourcesIP, appDataReceiver, log);

		append(processingThread);

		processingThread->start();
	}

	DEBUG_LOG_MSG(log, QString("AppDataProcessingThreadsPool started. Running threads count %1%2 (count from settings %3)").
							arg(poolSize).arg(poolSize == idealThreadCount ? " (ideal)" : "").arg(poolSizeFromSettings));
}

void AppDataProcessingThreadsPool::stopProcessingThreads()
{
	for(AppDataProcessingThread* processingThread : *this)
	{
		if (processingThread == nullptr)
		{
			assert(false);
			continue;
		}

		processingThread->quitAndWait();
		delete processingThread;
	}

	clear();
}

