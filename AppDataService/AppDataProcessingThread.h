#pragma once

#include "../lib/SimpleThread.h"
#include "AppDataReceiver.h"

class AppDataProcessingWorker : public SimpleThreadWorker
{
public:
	AppDataProcessingWorker(int number,
							const AppDataSourcesIP& appDataSourcesIP,
							const AppDataReceiver* appDataReceiver,
							CircularLoggerShared log);

	void connectToReceiver(AppDataReceiver& appDataReceiver);

private:
	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

public slots:
	void onAppDataSourceReceiveRupFrame(quint32 appDataSourceIP);

private:
	int m_number = 0;
	const AppDataSourcesIP& m_appDataSourcesIP;
	const AppDataReceiver* m_appDataReceiver;
	CircularLoggerShared m_log;

	// parsing statistics
	//
	quint64 m_parsedRupPacketCount = 0;
	quint64 m_successOwnership = 0;
	quint64 m_failOwnership = 0;
};


class AppDataProcessingThread : public SimpleThread
{
public:
	AppDataProcessingThread(int number,
							const AppDataSourcesIP& appDataSourcesIP,
							const AppDataReceiver* appDataReceiver,
							CircularLoggerShared log);
};


class AppDataProcessingThreadsPool : public QList<AppDataProcessingThread*>
{
public:
	static const int IDEAL_THREADS_COUNT = -1;

public:
	void startProcessingThreads(int poolSize,
								const AppDataSourcesIP& appDataSourcesIP,
								const AppDataReceiver* appDataReceiver,
								CircularLoggerShared log);

	void stopProcessingThreads();
};

