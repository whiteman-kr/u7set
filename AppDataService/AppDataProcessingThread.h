#pragma once

#include "../UtilsLib/SimpleThread.h"
#include "../OnlineLib/CircularLogger.h"
#include "AppDataSource.h"

class AsyncAppDataReceiver;

class AppDataProcessingThread : public RunOverrideThread
{
public:
	AppDataProcessingThread(AsyncAppDataReceiver& appDataReceiver,
							int number,
							CircularLoggerShared log);

	void run() override;

private:
	AsyncAppDataReceiver& m_appDataReceiver;
	int m_number = 0;
	const AppDataSourcesIP& m_appDataSourcesIP;
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
								AsyncAppDataReceiver& appDataReciever,
								CircularLoggerShared log);

	void stopProcessingThreads();
};

