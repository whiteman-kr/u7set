#include "AppDataProcessingThread.h"
#include "../lib/WUtils.h"


// -------------------------------------------------------------------------------
//
// AppDataProcessingWorker class implementation
//
// -------------------------------------------------------------------------------

AppDataProcessingWorker::AppDataProcessingWorker(int number,
												 const AppDataSourcesIP& appDataSourcesIP,
												 const AppDataReceiver* appDataReceiver,
												 CircularLoggerShared log) :
	m_number(number),
	m_appDataSourcesIP(appDataSourcesIP),
	m_appDataReceiver(appDataReceiver),
	m_log(log)
{
	assert(appDataReceiver != nullptr);
}

void AppDataProcessingWorker::onThreadStarted()
{
	m_thisThread = QThread::currentThread();

	connect(m_appDataReceiver, &AppDataReceiver::rupFrameIsReceived, this, &AppDataProcessingWorker::onAppDataSourceReceiveRupFrame);

	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThread #%1 is started").arg(m_number));
}

void AppDataProcessingWorker::onThreadFinished()
{
	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThread #%1 is finished").arg(m_number));
}

void AppDataProcessingWorker::onAppDataSourceReceiveRupFrame(quint32 appDataSourceIP)
{
	AppDataSourceShared appDataSource = m_appDataSourcesIP.value(appDataSourceIP, nullptr);

	bool result = appDataSource->seizeProcessingOwnership(m_thisThread);

	if (result == false)
	{
		m_failOwnership++;
		return;
	}

	m_successOwnership++;

	while(quitRequested() == false)
	{
		result = appDataSource->processRupFrameTimeQueue();

		if (result == false)
		{
			break;
		}

		appDataSource->parsePacket();

		m_parsedRupPacketCount++;

		if ((m_parsedRupPacketCount % 100) == 0)
		{
			qDebug() << " tread " << m_number << "parsed " << m_parsedRupPacketCount << " ----- success" << m_successOwnership << "/" << m_failOwnership;
		}
	}

	appDataSource->releaseProcessingOwnership(m_thisThread);
}

// -------------------------------------------------------------------------------
//
// AppDataProcessingThread class implementation
//
// -------------------------------------------------------------------------------

AppDataProcessingThread::AppDataProcessingThread(int number,
												 const AppDataSourcesIP& appDataSourcesIP,
												 const AppDataReceiver* appDataReceiver,
												 CircularLoggerShared log) :
	SimpleThread(new AppDataProcessingWorker(number, appDataSourcesIP, appDataReceiver, log))
{
}


// -------------------------------------------------------------------------------
//
// AppDataProcessingThread2 class implementation
//
// -------------------------------------------------------------------------------


AppDataProcessingThread2::AppDataProcessingThread2(int number,
												 const AppDataSourcesIP& appDataSourcesIP,
												 const AppDataReceiver* appDataReceiver,
												 CircularLoggerShared log) :
	m_number(number),
	m_appDataSourcesIP(appDataSourcesIP),
	m_appDataReceiver(appDataReceiver),
	m_log(log)
{
	assert(appDataReceiver != nullptr);
}

void AppDataProcessingThread2::run()
{
	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThread #%1 is started").arg(m_number));

	QThread* thisThread = currentThread();

	while(m_quitRequested == false)
	{
		bool hasNoDataToProcessing = true;

		for(AppDataSourceShared appDataSource : m_appDataSourcesIP)
		{
			if (appDataSource == nullptr)
			{
				assert(false);
				continue;
			}

			bool result = appDataSource->seizeProcessingOwnership(thisThread);

			if (result == false)
			{
				m_failOwnership++;
				continue;
			}

			m_successOwnership++;

			if (appDataSource->rupFramesQueueIsEmpty() == false)
			{
				do
				{
					result = appDataSource->processRupFrameTimeQueue();

					if (result == false)
					{
						break;
					}

					hasNoDataToProcessing = false;

					appDataSource->parsePacket();

					m_parsedRupPacketCount++;

					if ((m_parsedRupPacketCount % 100) == 0)
					{
						qDebug() << " tread " << m_number << "parsed " << m_parsedRupPacketCount <<
									" ----- success" << m_successOwnership << "/" << m_failOwnership <<
									"queue max size" << appDataSource->rupFramesQueueMaxSize() <<
									"losted" << appDataSource->lostedFramesCount();
					}
				}
				while(m_quitRequested == false);
			}

			appDataSource->releaseProcessingOwnership(thisThread);
		}

		if (hasNoDataToProcessing == true)
		{
			usleep(500);
		}
	}

	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThread #%1 is finished").arg(m_number));
}

void AppDataProcessingThread2::quitAndWait()
{
	quit();
	wait();
}



// -------------------------------------------------------------------------------
//
// AppDataProcessingThreadsPool class implementation
//
// -------------------------------------------------------------------------------

void AppDataProcessingThreadsPool::startProcessingThreads(int poolSizeFromSettings,
														  const AppDataSourcesIP& appDataSourcesIP,
														  const AppDataReceiver* appDataReceiver,
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
		AppDataProcessingThread2* processingThread = new AppDataProcessingThread2(i + 1, appDataSourcesIP, appDataReceiver, log);

		append(processingThread);

		processingThread->start();
	}

	DEBUG_LOG_MSG(log, QString("AppDataProcessingThreadsPool started. Running threads count %1%2 (count from settings %3)").
							arg(poolSize).arg(poolSize == idealThreadCount ? " (ideal)" : "").arg(poolSizeFromSettings));
}

void AppDataProcessingThreadsPool::stopProcessingThreads()
{
	for(AppDataProcessingThread2* processingThread : *this)
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

