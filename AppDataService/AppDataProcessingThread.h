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
	const QThread* m_thisThread = nullptr;
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

class AppDataProcessingThread2 : public QThread
{
public:
	AppDataProcessingThread2(int number,
							const AppDataSourcesIP& appDataSourcesIP,
							const AppDataReceiver* appDataReceiver,
							CircularLoggerShared log);

	void run() override;

	void quit() { m_quitRequested = true; }

	void quitAndWait();

private:
	bool m_quitRequested = false;
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



class AppDataProcessingThreadsPool : public QList<AppDataProcessingThread2*>
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

