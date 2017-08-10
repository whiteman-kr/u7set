#pragma once

#include <QSqlQuery>

#include "../lib/SimpleThread.h"
#include "../lib/CircularLogger.h"
#include "../lib/TimeStamp.h"
#include "../lib/Hash.h"
#include "../lib/Queue.h"
#include "../lib/SocketIO.h"

#include "../Proto/network.pb.h"

#include "Archive.h"

class TcpArchRequestsServer;

struct ArchRequestParam
{
	ArchRequestParam();

	void clearSignalHashes();

	// params from ARCHS_GET_APP_SIGNALS_STATES_START request
	//
	TimeType timeType = TimeType::System;

	qint64 startTime = 0;
	qint64 endTime = 0;

	int signalHashesCount = 0;

	Hash signalHashes[ARCH_REQUEST_MAX_SIGNALS];

	// requestID generated by TcpArchiveRequestServer
	//
	quint32 requestID = 0;
};


class ArchRequestContext
{
public:
	ArchRequestContext(const ArchRequestParam& param, const QTime& startTime, CircularLoggerShared logger);
	~ArchRequestContext();

	void checkSignalsHashes(const Archive& arch);

	quint32 requestID() const { return m_param.requestID; }
	bool isDataReady() const { return m_dataReady; }

	int totalStates() const { return m_totalStates; }
	int sentStates() const { return m_sentStates; }

	int signalCount() const { return m_param.signalHashesCount; }

	ArchiveError archError() const { return m_archError; }

	Network::GetAppSignalStatesFromArchiveNextReply& getNextReply() { return m_reply; }

	Hash signalHash(int index);

	int timeElapsed() const { return m_time.elapsed(); }

	TimeType requestTimeType() const { return m_requestTimeType; }

	qint64 requestStartTime() const { return m_requestStartTime; }
	qint64 requestEndTime() const { return m_requestEndTime; }

	qint64 expandedRequestStartTime() const { return m_expandedRequestStartTime; }
	qint64 expandedRequestEndTime() const { return m_expandedRequestEndTime; }

	bool createGetSignalStatesQueryStr(const Archive& archive);

	bool executeQuery(const Archive& archive, QSqlDatabase& db, CircularLoggerShared& logger);

	void getNextData();

private:
	void setArchError(ArchiveError err) { m_archError = err; }
	void setDataReady(bool ready) { m_dataReady = ready; }

	void createQuery(QSqlDatabase& db, const QString& queryStr);

	TimeType timeType() const { return m_param.timeType; }

	qint64 startTime() const { return m_param.startTime; }
	QString startTimeStr() const { return TimeStamp(m_param.startTime).toDateTime().toString("yyyy-MM-dd HH:mm:ss"); }

	qint64 endTime() const { return m_param.endTime; }
	QString endTimeStr() const { return TimeStamp(m_param.endTime).toDateTime().toString("yyyy-MM-dd HH:mm:ss"); }

	int signalHashesCount() const { return m_param.signalHashesCount; }
	const Hash* signalHashes() const { return m_param.signalHashes; }

	void clearSignalHashes();

private:
	ArchRequestParam m_param;
	QTime m_time;
	CircularLoggerShared m_logger;

	//

	qint64 m_localTimeOffset = 0;

	TimeType m_requestTimeType = TimeType::System;

	qint64 m_requestStartTime = 0;
	qint64 m_requestEndTime = 0;

	qint64 m_expandedRequestStartTime = 0;
	qint64 m_expandedRequestEndTime = 0;

	QString m_cmpField;

	//

	QSqlQuery* m_statesQuery = nullptr;
	QString m_statesQueryStr;

	ArchiveError m_archError = ArchiveError::Success;

	bool m_dataReady = false;

	int m_totalStates = 0;
	int m_sentStates = 0;

	Network::GetAppSignalStatesFromArchiveNextReply m_reply;

	friend class ArchRequestThreadWorker;

};

typedef std::shared_ptr<ArchRequestContext> ArchRequestContextShared;


class ArchRequestThreadWorker : public SimpleThreadWorker
{
	Q_OBJECT

public:
	ArchRequestThreadWorker(Archive& archive, CircularLoggerShared& logger);

	ArchRequestContextShared startNewRequest(ArchRequestParam& param, const QTime &startTime);
	void finalizeRequest(quint32 requestID);

	void getNextData(ArchRequestContextShared context);

signals:
	void newRequest(ArchRequestContextShared context);
	void getNextDataSignal(quint32 requestID);

private:
	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

	bool tryConnectToDatabase();

private slots:
	void onNewRequest(ArchRequestContextShared context);
	void onGetNextData(quint32 requestID);

private:
	Archive& m_archive;
	CircularLoggerShared m_logger;

	QSqlDatabase* m_db = nullptr;				// project archive database

	QMutex m_requestContextsMutex;

	QHash<quint32, ArchRequestContextShared> m_requestContexts;		//	requestID => ArchRequestContext

	QQueue<ArchRequestContextShared> m_newRequests;

	ArchRequestContextShared m_currentRequest;
};


class ArchRequestThread : public SimpleThread
{
public:
	ArchRequestThread(Archive& archive, CircularLoggerShared& logger);

	ArchRequestContextShared startNewRequest(ArchRequestParam& param, const QTime& startTime);
	void finalizeRequest(quint32 requestID);

	void getNextData(ArchRequestContextShared context);

private:
	ArchRequestThreadWorker* m_worker = nullptr;
};

