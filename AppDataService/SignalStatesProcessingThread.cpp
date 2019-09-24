#include "../lib/WUtils.h"

#include "SignalStatesProcessingThread.h"


SignalStatesProcessingThread::SignalStatesProcessingThread(const AppDataSources& appDataSources, CircularLoggerShared log) :
    m_appDataSources(appDataSources),
    m_log(log)
{

}

void SignalStatesProcessingThread::registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
																 bool isArchivingQueue,
																 const QString& description)
{
    TEST_PTR_RETURN(destQueue);

	bool found = false;

	m_queuesMutex.lock();

	for(const QPair<SimpleAppSignalStatesQueueShared, bool>& queue : m_queues)
	{
		if (queue.first == destQueue)
		{
			found = true;
			assert(false);
			break;
		}
	}

	if (found == false)
	{
		m_queues.append(QPair<SimpleAppSignalStatesQueueShared, bool>(destQueue, isArchivingQueue));
	}

	m_queuesMutex.unlock();

    DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread: register queue '%1'").arg(description));
}

void SignalStatesProcessingThread::unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue, const QString& description)
{
    TEST_PTR_RETURN(destQueue);

	int curIndex = 0;
	bool removeOk = false;

	m_queuesMutex.lock();

	for(const QPair<SimpleAppSignalStatesQueueShared, bool>& queue : m_queues)
	{
		if (queue.first == destQueue)
		{
			m_queues.removeAt(curIndex);
			removeOk = true;
			break;
		}

		curIndex++;
	}

	m_queuesMutex.unlock();

	if (removeOk == false)
	{
		assert(false);			// destQueue is not found in m_queues
	}

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

			SimpleAppSignalStateArchiveFlag state;

			int processedStatesCount = 0;

			m_queuesMutex.lock();

			do
			{
				bool result = appDataSource->getSignalState(&state, this);

				if (result == false)
				{
					break;		    // appDataSource has no states to processing, go to next source
				}

				hasNoStatesToProcessing = false;

				for(const QPair<SimpleAppSignalStatesQueueShared, bool>& queue : m_queues)
				{
					if (queue.second == true)
					{
						// is archiving queue
						//
						if (state.sendStateToArchive == true)
						{
							queue.first->push(state.state, this);
						}
					}
					else
					{
						queue.first->push(state.state, this);
					}
				}

				processedStatesCount++;
			}
			while(processedStatesCount < 100);

			m_queuesMutex.unlock();

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

