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

		if ((m_parsedRupPacketCount % 100) == 0)
		{
			qDebug() << " tread " << m_number << "parsed " << m_parsedRupPacketCount << " ----- success" << m_successOwnership << "/" << m_failOwnership;
		}
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
												 const AppDataReceiver* appDataReceiver,
												 CircularLoggerShared log) :
	SimpleThread(new AppDataProcessingWorker(number, appDataSourcesIP, appDataReceiver, log))
{
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

