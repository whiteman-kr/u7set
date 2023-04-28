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

void SignalStatesProcessingThread::registerGatewaySignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
								   const Network::GetAppSignalStateChangesForGatewayRequest& request,
								   const QString& description)
{
	Q_ASSERT(false);
}

void SignalStatesProcessingThread::unregisterGatewaySignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue)
{
	Q_ASSERT(false);
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

	while(receiver.isQuitRequested() == false)
	{
		ul.lock();

		waitCondition.wait_for(ul, std::chrono::milliseconds(10));

		while(true)
		{
			if (requireProcessing.empty() == true ||
				receiver.isQuitRequested() == true)
			{
				ul.unlock();
				break;
			}

			AppDataSource* source = requireProcessing.front();

			requireProcessing.pop();

			ul.unlock();

			while(source->getSignalState(&state, thisThread) == true)
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
			}

			ul.lock();
		}
	}

	DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread finished"));
}
