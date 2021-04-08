#pragma once

#include "../lib/SimpleThread.h"
#include "../OnlineLib/CircularLogger.h"
#include "../lib/SimpleMutex.h"

#include "AppDataSource.h"

class SignalStatesProcessingThread : public RunOverrideThread
{
public:
	SignalStatesProcessingThread(const AppDataSources& appDataSources, CircularLoggerShared log);

	void registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue, bool isArchivingQueue, const QString& description);
	void unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue, const QString& description);

	void run() override;

private:
	const AppDataSources& m_appDataSources;
	CircularLoggerShared m_log;

	//

	SimpleMutex m_queuesMutex;

	QVector<QPair<SimpleAppSignalStatesQueueShared, bool>> m_queues;
};
