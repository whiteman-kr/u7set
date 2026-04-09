#pragma once

#include "../OnlineLib/CircularLogger.h"
#include "../UtilsLib/SpinLock.h"
#include "../AppSignalLib/SimpleAppSignalState.h"
#include "DynamicAppSignalState.h"

class AppDataReceiver;

class SignalStatesProcessingThread
{
public:
	SignalStatesProcessingThread(DynamicAppSignalStates& signalStates,
								 CircularLoggerShared log);

	void registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
									   bool isArchivingQueue,
									   const QString& description);

	void unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue);

	void registerGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue,
									   const std::set<Hash>& hashes);

	void unregisterGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue);

	void processStates(AppDataReceiver& receiver);

private:
	DynamicAppSignalStates& m_signalStates;
	CircularLoggerShared m_log;

	struct QueueInfo
	{
		SimpleAppSignalStatesQueueShared queue;
		bool isArchivingQueue = false;
		QString description;
	};

	std::mutex m_queuesMutex;
	std::vector<QueueInfo> m_queues;

	//

	static constexpr int GATEWAY_QUEUES_COUNT = sizeof(quint32) * 8;

	std::mutex m_gatewayQueuesMutex;

	struct GatewayQueueHashes
	{
		GatewayAppSignalStatesQueueShared queue;
		std::set<Hash> hashes;
	};

	std::vector<GatewayQueueHashes> m_gatewayQueues;

};
