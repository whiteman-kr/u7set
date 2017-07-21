#pragma once

#include "../lib/SimpleThread.h"
#include "../lib/CircularLogger.h"
#include "../lib/TimeStamp.h"
#include "../lib/Hash.h"
#include "../lib/Queue.h"
#include "../lib/SocketIO.h"

struct ArchRequest
{
	TimeType timeType = TimeType::System;

	qint64 startTime = 0;
	qint64 endTime = 0;

	int hashesCount = 0;

	Hash signalHashes[ARCH_REQUEST_MAX_SIGNALS];

	quint32 requestID = 0;
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

