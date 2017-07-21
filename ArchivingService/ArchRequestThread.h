#pragma once

#include "../lib/SimpleThread.h"
#include "../lib/CircularLogger.h"
#include "../lib/TimeStamp.h"
#include "../lib/Hash.h"
#include "../lib/Queue.h"


struct ArchRequest
{
	static const int MAX_SIGNALS = 32;

	TimeType timeType = TimeType::System;

	qint64 startTime = 0;
	qint64 endTime = 0;

	int hashesCount = 0;

	Hash signalHashes[MAX_SIGNALS];
};

typedef Queue<ArchRequest> ArchRequestQueue;


class ArchRequestThreadWorker : public SimpleThreadWorker
{
public:
	ArchRequestThreadWorker(const QString& projectID, CircularLoggerShared& logger);

private:
	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

private:
	QString m_projectID;
	CircularLoggerShared m_logger;
};


class ArchRequestThread : public SimpleThread
{
public:
	ArchRequestThread(const QString &projectID, CircularLoggerShared& logger);
};

