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

class ArchRequestParam;

class ArchRequest : public RunOverrideThread
{
public:
	ArchRequest(Archive& archive, const ArchRequestParam& param, CircularLoggerShared logger);
	virtual ~ArchRequest();

	void run() override;

	quint32 requestID() const { return m_param.requestID(); }

	void nextDataRequired() { m_nextDataRequired.store(true); }
	bool isDataReady() const { return m_dataReady.load(); }

	Network::GetAppSignalStatesFromArchiveNextReply& getNextReply() { return m_reply; }

	ArchiveError archError() const { return m_archError; }

	int timeElapsed() const { return QDateTime::currentMSecsSinceEpoch() - m_startTime; }

protected:
	int totalStates() const { return m_totalStates; }
	int sentStates() const { return m_sentStates; }

//	int signalCount() const { return m_param.signalHashes.count(); }

//	Hash signalHash(int index);

//	E::TimeType requestTimeType() const { return m_requestTimeType; }

//	qint64 requestStartTime() const { return m_requestStartTime; }
//	qint64 requestEndTime() const { return m_requestEndTime; }

//	qint64 expandedRequestStartTime() const { return m_expandedRequestStartTime; }
//	qint64 expandedRequestEndTime() const { return m_expandedRequestEndTime; }

	void setArchError(ArchiveError err) { m_archError = err; }
	void setDataReady(bool ready) { m_dataReady = ready; }

	bool isNextDataRequired() { return m_nextDataRequired.load(); }

private:

	void prepareFiles();
	ArchFindResult findData();
	void getNextData();

	void reportErrorAndWaitForQuit();
	void reportNoDataAndWaitForQuit();

	void waitForQuit();

//	E::TimeType timeType() const { return m_param.timeType; }

//	qint64 startTime() const { return m_param.startTime; }
	QString startTimeStr() const { return TimeStamp(m_param.startTime()).toDateTime().toString("yyyy-MM-dd HH:mm:ss"); }

//	qint64 endTime() const { return m_param.endTime; }
	QString endTimeStr() const { return TimeStamp(m_param.endTime()).toDateTime().toString("yyyy-MM-dd HH:mm:ss"); }

//	const QVector<Hash>& signalHashes() const { return m_param.signalHashes; }

protected:
	Archive& m_archive;
	ArchRequestParam m_param;
	CircularLoggerShared m_logger;
	qint64 m_startTime;

	//

	ArchRequestParam m_execParam;

	QHash<Hash, ArchFile*> m_archFiles;
	QVector<ArchFile*> m_archFilesArray;

	std::atomic<bool> m_nextDataRequired = { false };
	std::atomic<bool> m_dataReady = { false };
	bool m_noMoreData = false;

	ArchiveError m_archError = ArchiveError::Success;
	Network::GetAppSignalStatesFromArchiveNextReply m_reply;

	int m_totalStates = 0;
	int m_sentStates = 0;
};

typedef std::shared_ptr<ArchRequest> ArchRequestShared;

/*
class ArchRequestThreadWorker : public SimpleThreadWorker
{
	Q_OBJECT

public:
	ArchRequestThreadWorker(ArchRequest* request, CircularLoggerShared& logger);

	void finalizeRequest(quint32 requestID);

	void getNextData();

private:
	virtual void onThreadStarted() override;
	virtual void onThreadFinished() override;

private slots:
	void onNewRequest(quint32 requestID);
	void onGetNextData(quint32 requestID);
	void onFinalizeRequest(quint32 requestID);

private:
	ArchRequest* m_request = nullptr;
	CircularLoggerShared m_logger;
};*/
