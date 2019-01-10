#pragma once

#include "../lib/SimpleThread.h"
#include "AppDataReceiver.h"

class AppDataProcessingThread : public RunOverrideThread
{
public:
	AppDataProcessingThread(int number,
							const AppDataSourcesIP& appDataSourcesIP,
							const AppDataReceiverThread* appDataReceiver,
							CircularLoggerShared log);

	void run() override;

private:
	int m_number = 0;
	const AppDataSourcesIP& m_appDataSourcesIP;
	const AppDataReceiverThread* m_appDataReceiver;
	CircularLoggerShared m_log;

	// parsing statistics
	//
	quint64 m_parsedRupPacketCount = 0;
	quint64 m_successOwnership = 0;
	quint64 m_failOwnership = 0;
};

class AppDataProcessingThreadsPool : public QList<AppDataProcessingThread*>
{
public:
	static const int IDEAL_THREADS_COUNT = -1;

public:
	void startProcessingThreads(int poolSize,
								const AppDataSourcesIP& appDataSourcesIP,
								const AppDataReceiverThread* appDataReceiver,
								CircularLoggerShared log);

	void stopProcessingThreads();
};

