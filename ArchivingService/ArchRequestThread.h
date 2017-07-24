#pragma once

#include <QSqlQuery>

#include "../lib/SimpleThread.h"
#include "../lib/CircularLogger.h"
#include "../lib/TimeStamp.h"
#include "../lib/Hash.h"
#include "../lib/Queue.h"
#include "../lib/SocketIO.h"

class TcpArchRequestsServer;

struct ArchRequestParam
{
	// params from ARCHS_GET_APP_SIGNALS_STATES_START request
	//
	TimeType timeType = TimeType::System;

	qint64 startTime = 0;
	qint64 endTime = 0;

	int hashesCount = 0;

	Hash signalHashes[ARCH_REQUEST_MAX_SIGNALS];

	// requestID generated by TcpArchiveRequestServer
	//
	quint32 requestID = 0;

	TcpArchRequestsServer* archRequestServer = nullptr;
};


class ArchRequestContext
{
public:
	ArchRequestContext(const ArchRequestParam& param);

	quint32 requestID() const { return m_param.requestID; }
	bool isDataReady() const { return m_dataReady; }

	int totalStates() const { return m_totalStates; }
	int sentStates() const { return m_sentStates; }

private:
	ArchRequestParam m_param;

	QSqlQuery m_query;

	bool m_dataReady = false;

	int m_totalStates = 0;
	int m_sentStates = 0;
};

typedef std::shared_ptr<ArchRequestContext> ArchRequestContextShared;


class ArchRequestThreadWorker : public SimpleThreadWorker
{
	Q_OBJECT

public:
	ArchRequestThreadWorker(const QString& projectID, CircularLoggerShared& logger);

	ArchRequestContextShared startNewRequest(ArchRequestParam& param);
	void finalizeRequest(quint32 requestID);

signals:
	void newRequest(ArchRequestContextShared context);

private:
	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

	bool tryConnectToDatabase();

	void onNewRequest(ArchRequestContextShared context);

private:
	QString m_projectID;
	CircularLoggerShared m_logger;

	QSqlDatabase m_db;				// project archive database

	QMutex m_requestContextsMutex;

	QHash<quint32, ArchRequestContextShared> m_requestContexts;		//	requestID => ArchRequestContext

	QQueue<ArchRequestContextShared> m_newRequests;

	ArchRequestContextShared m_currentRequest;
};


class ArchRequestThread : public SimpleThread
{
public:
	ArchRequestThread(const QString &projectID, CircularLoggerShared& logger);

	ArchRequestContextShared startNewRequest(ArchRequestParam& param);
	void finalizeRequest(quint32 requestID);

private:
	ArchRequestThreadWorker* m_worker = nullptr;
};

