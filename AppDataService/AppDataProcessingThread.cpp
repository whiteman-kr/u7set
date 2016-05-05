#include "AppDataProcessingThread.h"


AppDataProcessingWorker::AppDataProcessingWorker(RupDataQueue& rupDataQueue, const SourceParseInfoMap& sourceParseInfoMap, AppSignalStates& signalStates) :
	m_rupDataQueue(rupDataQueue),
	m_sourceParseInfoMap(sourceParseInfoMap),
	m_signalStates(signalStates)
{
}

AppDataProcessingThread::AppDataProcessingThread(RupDataQueue& rupDataQueue,
												const SourceParseInfoMap& sourceParseInfoMap,
												AppSignalStates& signalStates) :
	SimpleThread(new AppDataProcessingWorker(rupDataQueue, sourceParseInfoMap, signalStates))
{
}


void AppDataProcessingThreadsPool::start(int poolSize, RupDataQueue& rupDataQueue,
											const SourceParseInfoMap& sourceParseInfoMap,
											AppSignalStates& signalStates)
{
	if (count() > 0)
	{
		stopAndClear();
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
		AppDataProcessingThread* processingThread = new AppDataProcessingThread(rupDataQueue, sourceParseInfoMap, signalStates);

		append(processingThread);

		processingThread->start();
	}

	qDebug() << "AppDataProcessingThreads starts: " << poolSize;
}


void AppDataProcessingThreadsPool::stopAndClear()
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

	qDebug() << "AppDataProcessingThreads finished";
}
