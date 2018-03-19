#pragma once

#include "AppDataServiceTypes.h"
#include "../lib/SimpleThread.h"
#include "../lib/DataChannel.h"
#include "AppSignalStateEx.h"
#include "AppDataReceiver.h"


class AppDataProcessingWorker : public SimpleThreadWorker
{
public:
	AppDataProcessingWorker(int number,
							const AppDataSourcesIP& appDataSources,
							AppDataReceiver& appDataReceiver,
							CircularLoggerShared log);

	void connectToReceiver(AppDataReceiver& appDataReceiver);

private:
	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

	void parseRupData();
	bool getDoubleValue(const SignalParseInfo& parseInfo, double& value);
	bool getValidity(const SignalParseInfo& parseInfo, quint32& validity);

public slots:
	void onAppDataSourceReceiveRupFrame(quint32 appDataSourceIP);

private:
	int m_number = 0;
	const AppDataSourcesIP& m_appDataSourcesIP;
	const AppDataReceiver& m_appDataReceiver;
	CircularLoggerShared m_log;

	// parsing statistics
	//
	quint64 m_parsedRupDataCount = 0;
	quint64 m_notFoundIPCount = 0;
	quint64 m_valueParsingErrorCount = 0;
	quint64 m_validityParsingErrorCount = 0;
	quint64 m_badSignalStateIndexCount = 0;
};


class AppDataProcessingThread : public SimpleThread
{
public:
	AppDataProcessingThread(int number, const AppDataSources& appDataSources, AppDataReceiver& appDataReceiver);
};


class AppDataProcessingThreadsPool : public QList<AppDataProcessingThread*>
{
public:
	void createProcessingThreads(int poolSize,
								 const AppDataSourcesIP& appDataSourcesIP,
								 const AppDataReceiver& appDataReceiver);

	void startProcessingThreads();

	void stopAndClearProcessingThreads();
};

