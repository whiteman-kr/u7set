#include "../UtilsLib/WUtils.h"

#include "SignalStatesProcessingThread.h"
#include "AppDataReceiver.h"

SignalStatesProcessingThread::SignalStatesProcessingThread(DynamicAppSignalStates& signalStates,
														   CircularLoggerShared log) :
	m_signalStates(signalStates),
	m_log(log)
{
	m_gatewayQueues.resize(GATEWAY_QUEUES_COUNT);
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

void SignalStatesProcessingThread::registerGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue,
								   const std::set<Hash>& hashes)
{
	quint32 queueMask = 0;

	m_gatewayQueuesMutex.lock();

	for(int i = 0; i < GATEWAY_QUEUES_COUNT; i++)
	{
		GatewayQueueHashes& gqh = m_gatewayQueues[i];

		if (gqh.queue == nullptr)
		{
			gqh.queue = destQueue;
			gqh.hashes = hashes;

			queueMask = 1 << i;

			break;
		}
	}

	m_gatewayQueuesMutex.unlock();

	if (queueMask != 0)
	{
		m_signalStates.setGatewayQueueMask(hashes, queueMask);
	}
}

void SignalStatesProcessingThread::unregisterGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue)
{
	quint32 queueMask = 0;
	std::set<Hash> hashes;

	m_gatewayQueuesMutex.lock();

	for(int i = 0; i < GATEWAY_QUEUES_COUNT; i++)
	{
		GatewayQueueHashes& gqh = m_gatewayQueues[i];

		if (gqh.queue == destQueue)
		{
			gqh.queue = nullptr;
			hashes.swap(gqh.hashes);

			queueMask = 1 << i;

			break;
		}
	}

	m_gatewayQueuesMutex.unlock();

	if (queueMask != 0)
	{
		m_signalStates.resetGatewayQueueMask(hashes, queueMask);
	}
}

void SignalStatesProcessingThread::processStates(AppDataReceiver& receiver)
{
	DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread is started"));

	auto& waitConditionMutex = receiver.m_statesProcessingRequiredMutex;
	auto& waitCondition = receiver.m_statesProcessingRequiredCondition;
	auto& requireProcessing = receiver.m_statesProcessingRequired;

	SimpleAppSignalStateArchiveFlag state;
	GatewayAppSignalStateQueueMask gwState;
	bool haveStateToProcessing = false;

	std::unique_lock ul(waitConditionMutex, std::defer_lock);

	AppDataSource* sourceToStatesProcessing = nullptr;

	while(true)
	{
		ul.lock();

		if (sourceToStatesProcessing != nullptr)
		{
			requireProcessing.push(sourceToStatesProcessing);
		}

		waitCondition.wait(ul, [&requireProcessing, &receiver]() -> bool
								{
									return	!requireProcessing.empty() ||
											receiver.isQuitRequested();
								});

		if (receiver.isQuitRequested() == true)
		{
			ul.unlock();
			break;
		}

		sourceToStatesProcessing = nullptr;

		if (requireProcessing.empty() == false)
		{
			sourceToStatesProcessing = requireProcessing.front();
			requireProcessing.pop();
		}

		ul.unlock();

		if (sourceToStatesProcessing != nullptr)
		{
			int ctr = 500;

			bool res = true;

			m_queuesMutex.lock();

			while(ctr > 0)
			{
				haveStateToProcessing = sourceToStatesProcessing->getSignalState(&state);

				if (haveStateToProcessing == false)
				{
					break;
				}

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
							res = queue->push(state.state);
							Q_ASSERT(res == true);				// queue overflow
							ctr--;
						}
					}
					else
					{
						res = queue->push(state.state);
						Q_ASSERT(res == true);					// queue overflow
						ctr--;
					}
				}
			}

			m_queuesMutex.unlock();

			//

			ctr = 500;

			m_gatewayQueuesMutex.lock();

			while(ctr > 0)
			{
				haveStateToProcessing = sourceToStatesProcessing->getGatewaySignalState(&gwState);

				if (haveStateToProcessing == false)
				{
					sourceToStatesProcessing = nullptr;
					break;
				}

				quint32 queueMask = gwState.gatewayQueueMask;

				for(int bit = 0; queueMask != 0 && bit < sizeof(quint32) * 8;  queueMask >>= 1, bit++)
				{
					if ((queueMask & 1) != 0)
					{
						GatewayAppSignalStatesQueueShared queue = m_gatewayQueues[bit].queue;

						if (queue != nullptr)
						{
							queue->push(gwState);
							ctr--;
						}
					}
				}
			}

			m_gatewayQueuesMutex.unlock();
		}
	}

	DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread finished"));
}
