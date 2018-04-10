#pragma once

#include "../lib/SimpleThread.h"
#include "../lib/CircularLogger.h"

#include "AppDataSource.h"

class SignalStatesProcessingThread : public RunOverrideThread
{
public:
	SignalStatesProcessingThread(const AppDataSources& appDataSources, CircularLoggerShared log);

	void registerDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue, const QString& description);
	void unregisterDestSignalStatesQueue(SimpleAppSignalStatesQueueShared destQueue, const QString& description);

	void run() override;

private:
	const AppDataSources& m_appDataSources;
	CircularLoggerShared m_log;

	//

	QMutex m_queueMapMutex;

	QHash<SimpleAppSignalStatesQueue*, SimpleAppSignalStatesQueueShared> m_queueMap;
};
