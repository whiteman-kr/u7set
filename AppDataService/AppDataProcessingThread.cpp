#include "AppDataProcessingThread.h"
#include "../UtilsLib/WUtils.h"
#include "AsyncAppDataReceiver.h"

// -------------------------------------------------------------------------------
//
// AppDataProcessingThread class implementation
//
// -------------------------------------------------------------------------------

AppDataProcessingThread::AppDataProcessingThread(AsyncAppDataReceiver& appDataReceiver,
												 int number) :
	m_appDataReceiver(appDataReceiver),
	m_appDataSources(appDataReceiver.appDataSources()),
	m_number(number)
{
	//setObjectName(QString("AppDataProcessingThread #%1").arg(number));
	DEBUG_STOP;
}

AppDataProcessingThread::AppDataProcessingThread(const AppDataProcessingThread& pt) :
	m_appDataReceiver(pt.m_appDataReceiver),
	m_number(pt.m_number),
	m_appDataSources(pt.m_appDataSources)
{
	DEBUG_STOP;
}

void AppDataProcessingThread::run()
{
	CircularLoggerShared log = m_appDataReceiver.log();

	DEBUG_LOG_MSG(log, QString("AppDataProcessingThread #%1 is started").arg(m_number));

	QThread* thisThread = QThread::currentThread();

	while(m_appDataReceiver.isQuitRequested() == false)
	{
		QThread::msleep(1);
/*		bool hasNoDataToProcessing = true;

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
		}*/
	}

	DEBUG_LOG_MSG(log, QString("AppDataProcessingThread #%1 is finished").arg(m_number));
}

// -------------------------------------------------------------------------------
//
// AppDataProcessingThreadsPool class implementation
//
// -------------------------------------------------------------------------------
/*
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
}*/

