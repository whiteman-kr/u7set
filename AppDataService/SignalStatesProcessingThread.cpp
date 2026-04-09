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

	{
		std::lock_guard lg(m_queuesMutex);

		auto it = std::find_if(m_queues.begin(), m_queues.end(),
							   [&destQueue](const QueueInfo& qi)
							   {
								   return qi.queue == destQueue;
							   });

		if (it != m_queues.end())
		{
			Q_ASSERT(false);				// queue already registered
			return;
		}

		m_queues.emplace_back(destQueue, isArchivingQueue, description);
	}

    DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread: register queue '%1'").arg(description));
}

void SignalStatesProcessingThread::unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue)
{
    TEST_PTR_RETURN(destQueue);

	QString description;

	{
		std::lock_guard lg(m_queuesMutex);

		auto it = std::find_if(m_queues.begin(), m_queues.end(),
							   [&destQueue](const QueueInfo& qi)
							   {
								   return qi.queue == destQueue;
							   });

		if (it == m_queues.end())
		{
			Q_ASSERT(false);				// queue not found
			return;
		}

		description = it->description;
		m_queues.erase(it);
	}

	DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread: unregister queue '%1'").arg(description));
}

void SignalStatesProcessingThread::registerGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue,
								   const std::set<Hash>& hashes)
{
	quint32 queueMask = 0;

	{
		std::lock_guard lg(m_gatewayQueuesMutex);

		for(int i = 0; i < GATEWAY_QUEUES_COUNT; i++)
		{
			GatewayQueueHashes& gqh = m_gatewayQueues[i];

			if (gqh.queue == nullptr)
			{
				gqh.queue = destQueue;
				gqh.hashes = hashes;

				queueMask = (quint32{1} << i);

				break;
			}
		}
	}

	if (queueMask == 0)
	{
		return;
	}

	m_signalStates.setGatewayQueueMask(hashes, queueMask);
}

void SignalStatesProcessingThread::unregisterGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue)
{
	quint32 queueMask = 0;
	std::set<Hash> hashes;

	{
		std::lock_guard lg(m_gatewayQueuesMutex);

		for(int i = 0; i < GATEWAY_QUEUES_COUNT; i++)
		{
			GatewayQueueHashes& gqh = m_gatewayQueues[i];

			if (gqh.queue == destQueue)
			{
				gqh.queue = nullptr;
				hashes.swap(gqh.hashes);

				queueMask = (quint32{1} << i);

				break;
			}
		}
	}

	if (queueMask == 0)
	{
		return;
	}

	m_signalStates.resetGatewayQueueMask(hashes, queueMask);
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

		if (sourceToStatesProcessing == nullptr)
		{
			continue;
		}

		{
			std::vector<QueueInfo> queues;

			{
				std::lock_guard lg(m_queuesMutex);
				queues = m_queues;
			}

			int ctr = 1000;
			bool res = true;

			while(ctr > 0)
			{
				haveStateToProcessing = sourceToStatesProcessing->getSignalState(&state);

				if (haveStateToProcessing == false)
				{
					break;
				}

				for(const QueueInfo& qi : queues)
				{
					if (qi.isArchivingQueue == true)
					{
						// is archiving queue
						//
						if (state.sendStateToArchive == true)
						{
							res = qi.queue->push(state.state);
							ctr--;
						}
					}
					else
					{
						res = qi.queue->push(state.state);
						ctr--;
					}
				}
			}
		}

		//

		{
			std::array<GatewayAppSignalStatesQueueShared, GATEWAY_QUEUES_COUNT> gatewayQueues;

			{
				std::lock_guard lg(m_gatewayQueuesMutex);

				for (int i = 0; i < GATEWAY_QUEUES_COUNT; ++i)
				{
					gatewayQueues[i] = m_gatewayQueues[i].queue;
				}
			}

			int ctr = 1000;

			while(ctr > 0)
			{
				haveStateToProcessing = sourceToStatesProcessing->getGatewaySignalState(&gwState);

				if (haveStateToProcessing == false)
				{
					sourceToStatesProcessing = nullptr;
					break;
				}

				quint32 queueMask = gwState.gatewayQueueMask;

				for(int bit = 0; queueMask != 0 && bit < GATEWAY_QUEUES_COUNT;  queueMask >>= 1, bit++)
				{
					if ((queueMask & 1) != 0)
					{
						GatewayAppSignalStatesQueueShared queue = gatewayQueues[bit];

						if (queue != nullptr)
						{
							queue->push(gwState);
							ctr--;
						}
					}
				}
			}
		}
	}

	DEBUG_LOG_MSG(m_log, QString("SignalStatesProcessingThread finished"));
}
