#pragma once

#include "CircularLogger.h"
#include "OnlineDataSource.h"

//
// Class intended to get signal states from OnlineDataSource(s) queues and
// distribute this states to archive and clients
//
class SignalStatesDistributor
{
public:
	SignalStatesDistributor(CircularLoggerShared log);

/*	void registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
									   bool isArchivingQueue,
									   const QString& description);

	void unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue);

	void registerGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue,
									   const std::set<Hash>& hashes);

	void unregisterGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue);*/

	void processStates();
	void quit() { m_quitRequested = true; }

private:
	std::mutex m_distributionRequiredMutex;
	std::condition_variable m_distributionRequiredCondition;
	std::queue<OnlineDataSource*> m_distributionRequiredSources;	//	queue of sources requires states queue processing

	std::atomic_bool m_quitRequested = { false };

	//

	CircularLoggerShared m_log;

	SimpleMutex m_queuesMutex;

	// queuePtr => pair<isArchiveQueue, queueDescription>
	//
	std::map<SimpleAppSignalStatesQueueShared, std::pair<bool, QString>> m_queues;

	//

	const int GATEWAY_QUEUES_COUNT = sizeof(quint32);

	SimpleMutex m_gatewayQueuesMutex;

	struct GatewayQueueHashes
	{
		GatewayAppSignalStatesQueueShared queue;
		std::set<Hash> hashes;
	};

	std::vector<GatewayQueueHashes> m_gatewayQueues;
};

