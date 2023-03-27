#pragma once

#include "../UtilsLib/SimpleThread.h"
#include "../OnlineLib/CircularLogger.h"
#include "../UtilsLib/SimpleMutex.h"
#include "../AppSignalLib/SimpleAppSignalState.h"

class AppDataReceiver;

class SignalStatesProcessingThread
{
public:
	SignalStatesProcessingThread(CircularLoggerShared log);

	void registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue,
									   bool isArchivingQueue,
									   const QString& description);

	void unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue);

	void processStates(AppDataReceiver& receiver);

private:
	CircularLoggerShared m_log;

	SimpleMutex m_queuesMutex;

	// queuePtr => pair<isArchiveQueue, queueDescription>
	//
	std::map<SimpleAppSignalStatesQueueShared, std::pair<bool, QString>> m_queues;
};
