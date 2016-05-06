#include "AppDataProcessingThread.h"


// -------------------------------------------------------------------------------
//
// AppDataProcessingWorker class implementation
//
// -------------------------------------------------------------------------------

AppDataProcessingWorker::AppDataProcessingWorker(int number, RupDataQueue& rupDataQueue, const SourceParseInfoMap& sourceParseInfoMap, AppSignalStates& signalStates) :
	m_number(number),
	m_rupDataQueue(rupDataQueue),
	m_sourceParseInfoMap(sourceParseInfoMap),
	m_signalStates(signalStates)
{
}


void AppDataProcessingWorker::onThreadStarted()
{
	connect(&m_rupDataQueue, &RupDataQueue::queueNotEmpty, this, &AppDataProcessingWorker::slot_rupDataQueueIsNotEmpty);
	qDebug() << "Processing thread started" << m_number;
}


void AppDataProcessingWorker::onThreadFinished()
{
	qDebug() << "Processing thread finished" << m_number;
}


void AppDataProcessingWorker::slot_rupDataQueueIsNotEmpty()
{
	int count = 0;

	do
	{
		bool result = m_rupDataQueue.pop(&m_rupData);

		if (result == false)
		{
			break;
		}

		parseRupData();

		count++;
	}
	while(count < 50);		//
}


void AppDataProcessingWorker::parseRupData()
{
	m_parsedRupDataCount++;

	// parse data from m_rupData
	//
	quint32 sourceIP = m_rupData.sourceIP;

	SourceSignalsParseInfo* sourceParseInfo = m_sourceParseInfoMap.value(sourceIP, nullptr);

	if (sourceParseInfo == nullptr)
	{
		m_notFoundIPCount++;
		return;
	}

	for(const SignalParseInfo& parseInfo : *sourceParseInfo)
	{
		int valueOffset = parseInfo.valueAddr.offset();
		int valueBit = parseInfo.valueAddr.bit();

		int validityOffset = parseInfo.validityAddr.offset();
		int validityBit = parseInfo.validityAddr.bit();

		bool validity = true;
		double value = 0;

		switch (parseInfo.dataFormat)
		{
		case E::DataFormat::Float:
			break;

		case E::DataFormat::SignedInt:
			break;

		case E::DataFormat::UnsignedInt:
			break;
		}
	}
}


// -------------------------------------------------------------------------------
//
// AppDataProcessingThread class implementation
//
// -------------------------------------------------------------------------------

AppDataProcessingThread::AppDataProcessingThread(int number, RupDataQueue& rupDataQueue,
												const SourceParseInfoMap& sourceParseInfoMap,
												AppSignalStates& signalStates) :
	SimpleThread(new AppDataProcessingWorker(number, rupDataQueue, sourceParseInfoMap, signalStates))
{
}


// -------------------------------------------------------------------------------
//
// AppDataProcessingThreadsPool class implementation
//
// -------------------------------------------------------------------------------

void AppDataProcessingThreadsPool::createProcessingThreads(int poolSize, RupDataQueue& rupDataQueue,
											const SourceParseInfoMap& sourceParseInfoMap,
											AppSignalStates& signalStates)
{
	if (count() > 0)
	{
		stopAndClearProcessingThreads();
	}

	if (poolSize > 8)
	{
		poolSize = 8;
	}
	else
	{
		if (poolSize <= 0)
		{
			poolSize = 1;
		}
	}

	for(int i = 0; i < poolSize; i++)
	{
		AppDataProcessingThread* processingThread = new AppDataProcessingThread(i, rupDataQueue, sourceParseInfoMap, signalStates);

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
