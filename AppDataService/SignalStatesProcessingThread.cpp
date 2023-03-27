#include "../UtilsLib/WUtils.h"

#include "SignalStatesProcessingThread.h"
#include "AppDataReceiver.h"

SignalStatesProcessingThread::SignalStatesProcessingThread(CircularLoggerShared log) :
    m_log(log)
{
}

void SignalStatesProcessingThread::registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
																 bool isArchivingQueue,
																 const QString& description)
{
    TEST_PTR_RETURN(destQueue);

	m_queuesMutex.lock();

	auto it = m_queues.find(destQueue);

	if (it == m_queues.end())
	{
		m_queues.insert({ destQueue, { isArchivingQueue, description }});
	}
	else
	{
		Q_ASSERT(false);
	}
	m_queuesMutex.unlock();

    DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread: register queue '%1'").arg(description));
}

void SignalStatesProcessingThread::unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue)
{
    TEST_PTR_RETURN(destQueue);

	bool removeOk = false;
	QString description;

	m_queuesMutex.lock();

	auto it = m_queues.find(destQueue);

	if (it != m_queues.end())
	{
		description = it->second.second;

		m_queues.erase(it);

		removeOk = true;
	}
	else
	{
		Q_ASSERT(false);
	}

	m_queuesMutex.unlock();

	if (removeOk == true)
	{
		DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread: unregister queue '%1'").arg(description));
	}
}

void SignalStatesProcessingThread::processStates(AppDataReceiver& receiver)
{
	DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread is started"));

	auto& waitConditionMutex = receiver.m_statesProcessigRequiredMutex;
	auto& waitCondition = receiver.m_statesProcessingRequiredCondition;
	auto& requireProcessing = receiver.m_statesProcessingRequired;

	QThread* thisThread = QThread::currentThread();

	SimpleAppSignalStateArchiveFlag state;

	std::unique_lock ul(waitConditionMutex, std::defer_lock);

	while(true)
	{
		ul.lock();

		waitCondition.wait(ul);

		if (receiver.isQuitRequested() == true)
		{
			ul.unlock();
			break;
		}

		while(true)
		{
			if (requireProcessing.empty() == true)
			{
				ul.unlock();
				break;
			}

			AppDataSource* source = requireProcessing.front();

			requireProcessing.pop();

			ul.unlock();

			bool stateExists = source->getSignalState(&state, thisThread);

			while(stateExists == true)
			{
				m_queuesMutex.lock(thisThread);

				for(const auto& p : m_queues)
				{
					SimpleAppSignalStatesQueueShared queue = p.first;
					bool isAchiveQueue = p.second.first;

					if (isAchiveQueue == true)
					{
						// is archiving queue
						//
						if (state.sendStateToArchive == true)
						{
							queue->push(state.state, thisThread);
						}
					}
					else
					{
						queue->push(state.state, thisThread);
					}
				}

				m_queuesMutex.unlock(thisThread);

				stateExists = source->getSignalState(&state, thisThread);
			}

			ul.lock();
		}
	}

	DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread finished"));
}
