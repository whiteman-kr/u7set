#include "AppDataProcessingThread.h"
#include "../lib/WUtils.h"


// -------------------------------------------------------------------------------
//
// AppDataProcessingWorker class implementation
//
// -------------------------------------------------------------------------------

AppDataProcessingWorker::AppDataProcessingWorker(int number,
												 const AppDataSourcesIP& appDataSourcesIP,
												 const AppDataReceiver& appDataReceiver,
												 CircularLoggerShared log) :
	m_number(number),
	m_appDataSourcesIP(appDataSourcesIP),
	m_appDataReceiver(appDataReceiver),
	m_log(log)
{
}

void AppDataProcessingWorker::onThreadStarted()
{
	connect(&m_appDataReceiver, &AppDataReceiver::rupFrameIsReceived, this, &AppDataProcessingWorker::onAppDataSourceReceiveRupFrame);

	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThread #%1 is started").arg(m_number));
}

void AppDataProcessingWorker::onThreadFinished()
{
	DEBUG_LOG_MSG(m_log, QString("AppDataProcessingThread #%1 is finished").arg(m_number));
}

void AppDataProcessingWorker::onAppDataSourceReceiveRupFrame(quint32 appDataSourceIP)
{
	AppDataSourceShared appDataSource = m_appDataSourcesIP.value(appDataSourceIP, nullptr);

	bool result = appDataSource->seizeProcessingOwnership(this);

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
	}

	appDataSource->releaseProcessingOwnership(this);
}

// -------------------------------------------------------------------------------
//
// AppDataProcessingThread class implementation
//
// -------------------------------------------------------------------------------

AppDataProcessingThread::AppDataProcessingThread(int number,
												 const AppDataSourcesIP& appDataSourcesIP,
												 const AppDataReceiver& appDataReceiver,
												 CircularLoggerShared log) :
	SimpleThread(new AppDataProcessingWorker(number, appDataSourcesIP, appDataReceiver, log))
{
}


// -------------------------------------------------------------------------------
//
// AppDataProcessingThreadsPool class implementation
//
// -------------------------------------------------------------------------------

void AppDataProcessingThreadsPool::createProcessingThreads(int poolSize,
														   const AppDataSourcesIP& appDataSourcesIP,
														   const AppDataReceiver& appDataReceiver,
														   CircularLoggerShared log)
{
	if (count() > 0)
	{
		stopAndClearProcessingThreads();
	}

	if (poolSize > 32)
	{
		poolSize = 32;
	}
	else
	{
		if (poolSize <= 0)
		{
			poolSize = QThread::idealThreadCount();
		}
	}

	for(int i = 0; i < poolSize; i++)
	{
		AppDataProcessingThread* processingThread = new AppDataProcessingThread(i + 1, appDataSourcesIP, appDataReceiver, log);

		append(processingThread);
	}
}


void AppDataProcessingThreadsPool::startProcessingThreads()
{
	for(AppDataProcessingThread* processingThread : *this)
	{
		if (processingThread == nullptr)
		{
			assert(false);
			continue;
		}

		processingThread->start();
	}
}


void AppDataProcessingThreadsPool::stopAndClearProcessingThreads()
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

