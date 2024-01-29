#pragma once

#include "../UtilsLib/SimpleThread.h"
#include "../OnlineLib/CircularLogger.h"
#include "../UtilsLib/SimpleMutex.h"
//#include "../AppSignalLib/SimpleAppSignalState.h"
#include "DynamicDiagSignalState.h"

class DiagDataReceiver;

class SignalStatesProcessingThread
{
public:
	SignalStatesProcessingThread(DynamicDiagSignalStates& signalStates,
								 CircularLoggerShared log);

/*	void registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
									   bool isArchivingQueue,
									   const QString& description);

	void unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue);

	void registerGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue,
									   const std::set<Hash>& hashes);

	void unregisterGatewaySignalStatesQueue(GatewayAppSignalStatesQueueShared destQueue);*/

	void processStates(DiagDataReceiver& receiver);

private:
	DynamicDiagSignalStates& m_signalStates;
	CircularLoggerShared m_log;

	SimpleMutex m_queuesMutex;

	// queuePtr => pair<isArchiveQueue, queueDescription>
	//
//	std::map<SimpleAppSignalStatesQueueShared, std::pair<bool, QString>> m_queues;

	//

/*	const int GATEWAY_QUEUES_COUNT = sizeof(quint32);

	SimpleMutex m_gatewayQueuesMutex;

	struct GatewayQueueHashes
	{
		GatewayAppSignalStatesQueueShared queue;
		std::set<Hash> hashes;
	};

	std::vector<GatewayQueueHashes> m_gatewayQueues; */

};
