#include "AppDataProcessingThread.h"
#include "../UtilsLib/WUtils.h"
#include "AsyncAppDataReceiver.h"

// -------------------------------------------------------------------------------
//
// AppDataProcessingThread class implementation
//
// -------------------------------------------------------------------------------

AppDataProcessingThread::AppDataProcessingThread(AsyncAppDataReceiver& appDataReceiver,
												 int number,
												 CircularLoggerShared log) :
	m_appDataReceiver(appDataReceiver),
	m_appDataSourcesIP(appDataReceiver.appDataSourcesIP()),
	m_number(number),
	m_log(log)
{
	setObjectName(QString("AppDataProcessingThread #%1").arg(number));
}

void AppDataProcessingThread::run()
{
	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThread #%1 is started").arg(m_number));

	QThread* thisThread = currentThread();

	while(isQuitRequested() == false)
	{
		bool hasNoDataToProcessing = true;

		for(auto& p : m_appDataSourcesIP)
		{
			AppDataSource* appDataSource = p.second;

			if (appDataSource == nullptr)
			{
				Q_ASSERT(false);
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
														  AsyncAppDataReceiver& appDataReciever,
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
		AppDataProcessingThread* processingThread = new AppDataProcessingThread(appDataReciever, i + 1, log);

		append(processingThread);

		processingThread->start();
	}

	DEBUG_LOG_MSG(log, QString("AppDataProcessingThreadsPool started. Running threads count %1%2").
							arg(poolSize).arg(poolSize == idealThreadCount ? " (ideal)" : ""));
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

