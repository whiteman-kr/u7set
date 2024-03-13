#include "SignalStatesDistributor.h"

/*
AppSignalStatesDistributor::AppSignalStatesDistributor(DynamicAppSignalStates& signalStates,
														   CircularLoggerShared log) :
	m_signalStates(signalStates),
	m_log(log)
{
	m_gatewayQueues.resize(GATEWAY_QUEUES_COUNT);
}

void AppSignalStatesDistributor::registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
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

void AppSignalStatesDistributor::unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue)
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

void AppSignalStatesDistributor::registerGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue,
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

void AppSignalStatesDistributor::unregisterGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue)
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

void SignalStatesDistributor::processStates()
{
	DEBUG_LOG_MSG(m_log, QString("SignalStatesDistributor is started"));

	QThread* thisThread = QThread::currentThread();

	SimpleAppSignalStateArchiveFlag state;
	GatewayAppSignalStateQueueMask gwState;
	bool haveStateToProcessing = false;

	std::unique_lock ul(m_distributionRequiredMutex, std::defer_lock);

	OnlineDataSource* sourceToStatesProcessing = nullptr;

	while(true)
	{
		ul.lock();

		if (sourceToStatesProcessing != nullptr)
		{
			m_distributionRequiredSources.push(sourceToStatesProcessing);
		}

		m_distributionRequiredCondition.wait(ul, [&m_distributionRequiredSources, &m_quitRequested]() -> bool
								{
									return	!m_distributionRequiredSources.empty() ||
											m_quitRequested;
								});

		if (m_quitRequested == true)
		{
			ul.unlock();
			break;
		}

		sourceToStatesProcessing = nullptr;

		if (m_distributionRequiredSources.empty() == false)
		{
			sourceToStatesProcessing = m_distributionRequiredSources.front();
			m_distributionRequiredSources.pop();
		}

		ul.unlock();

		int ctr = 500;

		if (sourceToStatesProcessing != nullptr)
		{
			m_queuesMutex.lock(thisThread);

			while(ctr > 0)
			{
				haveStateToProcessing = sourceToStatesProcessing->getSignalState(&state, thisThread);

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
							queue->push(state.state, thisThread);
							ctr--;
						}
					}
					else
					{
						queue->push(state.state, thisThread);
						ctr--;
					}
				}
			}

			m_queuesMutex.unlock(thisThread);

			//

			m_gatewayQueuesMutex.lock(thisThread);

			while(ctr > 0)
			{
				haveStateToProcessing = sourceToStatesProcessing->getGatewaySignalState(&gwState, thisThread);

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
							queue->push(gwState, thisThread);
							ctr--;
						}
					}
				}
			}

			m_gatewayQueuesMutex.unlock(thisThread);
		}
	}

	DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread finished"));
}
*/
