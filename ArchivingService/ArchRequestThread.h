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
class FileArchWriter;

class ArchRequestContext
{
public:
	ArchRequestContext(const ArchRequestParam& param, const QTime& startTime, CircularLoggerShared logger);
	virtual ~ArchRequestContext();

	virtual bool executeSatesRequest(ArchiveShared archive, QSqlDatabase* db) = 0;
	virtual void getNextStates() = 0;

public:
	quint32 requestID() const { return m_param.requestID; }
	bool isDataReady() const { return m_dataReady; }
	ArchiveError archError() const { return m_archError; }

	int timeElapsed() const { return m_time.elapsed(); }

	Network::GetAppSignalStatesFromArchiveNextReply& getNextReply() { return m_reply; }

protected:
	void checkSignalsHashes(ArchiveShared arch);

	int totalStates() const { return m_totalStates; }
	int sentStates() const { return m_sentStates; }

	int signalCount() const { return m_param.signalHashes.count(); }

	Hash signalHash(int index);

	E::TimeType requestTimeType() const { return m_requestTimeType; }

	qint64 requestStartTime() const { return m_requestStartTime; }
	qint64 requestEndTime() const { return m_requestEndTime; }

	qint64 expandedRequestStartTime() const { return m_expandedRequestStartTime; }
	qint64 expandedRequestEndTime() const { return m_expandedRequestEndTime; }

	void setArchError(ArchiveError err) { m_archError = err; }
	void setDataReady(bool ready) { m_dataReady = ready; }

private:
	E::TimeType timeType() const { return m_param.timeType; }

	qint64 startTime() const { return m_param.startTime; }
	QString startTimeStr() const { return TimeStamp(m_param.startTime).toDateTime().toString("yyyy-MM-dd HH:mm:ss"); }

	qint64 endTime() const { return m_param.endTime; }
	QString endTimeStr() const { return TimeStamp(m_param.endTime).toDateTime().toString("yyyy-MM-dd HH:mm:ss"); }

	const QVector<Hash>& signalHashes() const { return m_param.signalHashes; }

protected:
	ArchRequestParam m_param;
	QTime m_time;
	CircularLoggerShared m_logger;

	//

	qint64 m_localTimeOffset = 0;

	E::TimeType m_requestTimeType = E::TimeType::System;

	qint64 m_requestStartTime = 0;
	qint64 m_requestEndTime = 0;

	qint64 m_expandedRequestStartTime = 0;
	qint64 m_expandedRequestEndTime = 0;

	QString m_cmpField;

	qint64 m_startArchID = 0;
	qint64 m_endArchID = 0;

	ArchiveError m_archError = ArchiveError::Success;

	bool m_dataReady = false;

	int m_totalStates = 0;
	int m_sentStates = 0;

	Network::GetAppSignalStatesFromArchiveNextReply m_reply;

	friend class ArchRequestThreadWorker;

};

typedef std::shared_ptr<ArchRequestContext> ArchRequestContextShared;

class DbArchRequestContext : public ArchRequestContext
{
public:
	DbArchRequestContext(const ArchRequestParam& param, const QTime& startTime, CircularLoggerShared logger);
	virtual ~DbArchRequestContext();

	bool executeSatesRequest(ArchiveShared archive, QSqlDatabase* db) override;
	void getNextStates() override;

private:
	bool initArchId(QSqlDatabase& db);
	bool createGetSignalStatesQueryStr(ArchiveShared archive);

	bool execQuery(QSqlDatabase& db, const QString& queryStr);
	bool execQuery(QSqlQuery& query, const QString& queryStr);

private:
	//

	QSqlQuery* m_statesQuery = nullptr;
	QString m_statesQueryStr;
};

class FileArchRequestContext : public ArchRequestContext
{
public:
	FileArchRequestContext(const ArchRequestParam& param, const QTime& startTime, CircularLoggerShared logger);
	virtual ~FileArchRequestContext();

	bool executeSatesRequest(ArchiveShared archive, QSqlDatabase* db) override;
	void getNextStates() override;

private:

};

class ArchRequestThreadWorker : public SimpleThreadWorker
{
	Q_OBJECT

public:
	ArchRequestThreadWorker(ArchiveShared archive, FileArchWriter* fileArchWriter, CircularLoggerShared& logger);

	ArchRequestContextShared startNewRequest(ArchRequestParam& param, const QTime &startTime);
	void finalizeRequest(quint32 requestID);

	void getNextData(ArchRequestContextShared context);

signals:
	void newRequestSignal(quint32 requestID);
	void getNextDataSignal(quint32 requestID);
	void finalizeRequestSignal(quint32 requestID);

private:
	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

	bool tryConnectToDatabase();

private slots:
	void onNewRequest(quint32 requestID);
	void onGetNextData(quint32 requestID);
	void onFinalizeRequest(quint32 requestID);

private:
	ArchiveShared m_archive;
	FileArchWriter* m_fileArchWriter = nullptr;
	CircularLoggerShared m_logger;

	QSqlDatabase m_db;

	QMutex m_requestContextsMutex;

	QHash<quint32, ArchRequestContextShared> m_requestContexts;		//	requestID => ArchRequestContext

	QQueue<ArchRequestContextShared> m_newRequests;

	ArchRequestContextShared m_currentRequest;
};


class ArchRequestThread : public SimpleThread
{
public:
	ArchRequestThread(ArchiveShared archive, FileArchWriter* fileArchWriter, CircularLoggerShared logger);

	ArchRequestContextShared startNewRequest(ArchRequestParam& param, const QTime& startTime);
	void finalizeRequest(quint32 requestID);

	void getNextData(ArchRequestContextShared context);

private:
	ArchRequestThreadWorker* m_worker = nullptr;
};

