#include "../lib/WUtils.h"

#include "SignalStatesProcessingThread.h"


SignalStatesProcessingThread::SignalStatesProcessingThread(const AppDataSources& appDataSources, CircularLoggerShared log) :
    m_appDataSources(appDataSources),
    m_log(log)
{

}

void SignalStatesProcessingThread::registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue, const QString& description)
{
    TEST_PTR_RETURN(destQueue);

    SimpleAppSignalStatesQueue* destQueuePtr = destQueue.get();

    m_queueMapMutex.lock();

    if (m_queueMap.contains(destQueuePtr) == true)
    {
	assert(false);
	return;
    }

    m_queueMap.insert(destQueuePtr, destQueue);

    m_queueMapMutex.unlock();

    DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread: register queue '%1'").arg(description));
}

void SignalStatesProcessingThread::unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue, const QString& description)
{
    TEST_PTR_RETURN(destQueue);

    SimpleAppSignalStatesQueue* destQueuePtr = destQueue.get();

    m_queueMapMutex.lock();

    if (m_queueMap.contains(destQueuePtr) == false)
    {
	assert(false);
	return;
    }

    m_queueMap.remove(destQueuePtr);

    m_queueMapMutex.unlock();

    DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread: unregister queue '%1'").arg(description));
}

void SignalStatesProcessingThread::run()
{
    DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread is started"));

    do
    {
		bool hasNoStatesToProcessing = true;

		for(AppDataSourceShared appDataSource : m_appDataSources)
		{
			TEST_PTR_CONTINUE(appDataSource);

			int processedStatesCount = 0;

			do
			{
				SimpleAppSignalState state;

				bool result = appDataSource->getSignalState(&state, this);

				if (result == false)
				{
					break;		    // appDataSource has no states to processing, go to next source
				}

				// state.print();

				hasNoStatesToProcessing = false;

				m_queueMapMutex.lock();

				for(SimpleAppSignalStatesQueueShared destQueue : m_queueMap)
				{
					destQueue->push(state, this);
				}

				m_queueMapMutex.unlock();

				processedStatesCount++;
			}
			while(processedStatesCount < 200);

			if (isQuitRequested() == true)
			{
				break;
			}
		}

		if (isQuitRequested() == true)
		{
			break;
		}

		if (hasNoStatesToProcessing == true)
		{
			usleep(500);
		}
    }
    while(isQuitRequested() == false);

    DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread is finished"));
}

