#pragma once

#include <QSqlQuery>

#include "../lib/SimpleThread.h"
#include "../lib/CircularLogger.h"
#include "../lib/TimeStamp.h"
#include "../lib/Hash.h"
#include "../lib/Queue.h"
#include "../lib/SocketIO.h"
#include "../Proto/network.pb.h"
#include "ArchFile.h"

class ArchRequestParam
{
public:
	ArchRequestParam(quint32 requestID, E::TimeType timeType, qint64 startTime, qint64 endTime, const QVector<Hash>& signalHashes);

	quint32 requestID() const { return m_requestID; }

	E::TimeType timeType() const { return m_timeType; }
	void setTimeType(E::TimeType timeType) { m_timeType = timeType; }

	qint64 startTime() const { return m_startTime; }
	void setStartTime(qint64 startTime) { m_startTime = startTime; }

	qint64 endTime() const { return m_endTime; }
	void setEndTime(qint64 endTime) { m_endTime = endTime; }

	const QVector<Hash>& signalHashes() const { return m_signalHashes; }

	void expandTimes(qint64 expandTime);

	QString print();

private:
	quint32 m_requestID = 0;

	E::TimeType m_timeType = E::TimeType::System;

	qint64 m_startTime = 0;
	qint64 m_endTime = 0;

	QVector<Hash> m_signalHashes;
};

class ArchFileRequestData
{
public:
	struct PartitionInfo
	{
		QString fileName;
		QDateTime date;
		qint64 startTime;
	};

public:
	ArchFileRequestData(const ArchFile& archFile, const ArchRequestParam& param);
	~ArchFileRequestData();

	void findData();

	PartitionInfo partitionToReadInfo();
	bool getRecord(ArchFileRecord* record);
	bool gotoNextRecord();

	bool hasData() { return m_hasData; }

	qint64 startTime() const { return m_startTime; }

	ArchFindResult findResult() const { return m_findResult; }

private:
	void getArchPartitionsInfo();
	void findStartPosition();

	bool fillBuffer();

private:
	QString m_archFilePath;
	QString m_appSignalID;
	quint32 m_requestID = 0;
	E::TimeType m_timeType = E::TimeType::System;
	qint64 m_startTime = 0;
	qint64 m_endTime = 0;

	//

	QVector<PartitionInfo> m_partitionsInfo;

	int m_partitionToReadIndex = -1;
	ArchFilePartition m_partitionToRead;
	qint64 m_startRecord = -1;

	ArchFindResult m_findResult = ArchFindResult::NotFound;

	//

	static const int RECORDS_BUFFER_SIZE = 50000;

	ArchFileRecord* m_readBuffer = nullptr;
	int m_recordsInBuffer = 0;
	int m_nextRecordIndex = 0;
	bool m_hasData = true;
};

inline bool operator < (const ArchFileRequestData::PartitionInfo& p1, const ArchFileRequestData::PartitionInfo& p2) { return p1.startTime < p2.startTime; }

//

class Archive;

class ArchRequest : public RunOverrideThread
{
public:
	ArchRequest(Archive& archive, const ArchRequestParam& param, CircularLoggerShared logger);
	virtual ~ArchRequest();

	void run() override;

	quint32 requestID() const { return m_param.requestID(); }

	void nextDataRequired() { m_dataReady.store(false); m_nextDataRequired.store(true);  }

	bool isDataReady() const { return m_dataReady.load(); }

	Network::GetAppSignalStatesFromArchiveNextReply& getNextReply() { return m_reply; }

	int timeElapsed() const { return QDateTime::currentMSecsSinceEpoch() - m_startTime; }

	void setErrorMessage(const QString& errMsg) { m_errMsg = errMsg; }

//	int signalCount() const { return m_param.signalHashes.count(); }

//	Hash signalHash(int index);

//	E::TimeType requestTimeType() const { return m_requestTimeType; }

//	qint64 requestStartTime() const { return m_requestStartTime; }
//	qint64 requestEndTime() const { return m_requestEndTime; }

//	qint64 expandedRequestStartTime() const { return m_expandedRequestStartTime; }
//	qint64 expandedRequestEndTime() const { return m_expandedRequestEndTime; }


private:
	bool isNextDataRequired() { return m_nextDataRequired.load(); }
	void resetNextDataRequired() { m_nextDataRequired.store(false); }

	void prepareArchRequestData();
	bool findData();
	void getNextData();
	bool getNextRecord(Hash* hash, ArchFileRecord* record);

	void reportError();
	void reportNoData();
	void reportNoMoreData();
	void reportDataReady();

	void waitForQuit();

	void finalizeRequest();

	void setDataReady() { m_dataReady.store(true); }

	QString startTimeStr() const { return TimeStamp(m_param.startTime()).toDateTime().toString("yyyy-MM-dd HH:mm:ss"); }
	QString endTimeStr() const { return TimeStamp(m_param.endTime()).toDateTime().toString("yyyy-MM-dd HH:mm:ss"); }

private:

	struct ArchFileData
	{
		bool hasMoreData = false;

		ArchFileRecord record;
		qint64 recordTime = 0;
	};

private:
	Archive& m_archive;
	ArchRequestParam m_param;
	CircularLoggerShared m_logger;
	qint64 m_startTime = 0;

	//

	ArchRequestParam m_execParam;

	QHash<Hash, ArchFileRequestData*> m_requestData;
	QVector<ArchFileRequestData*> m_requestDataArray;
	QVector<ArchFileRequestData*> m_filesWithData;
	QVector<ArchFileData> m_fileData;

	bool m_firstCallOfGetNextRecord = true;
	qint64 m_minTime = -1;
	qint64 m_minTimeIndex = -1;

	std::atomic<bool> m_nextDataRequired = { false };
	std::atomic<bool> m_dataReady = { false };
	bool m_noMoreData = false;
	int m_sentStatesCount = 0;

	Network::GetAppSignalStatesFromArchiveNextReply m_reply;
	QString m_errMsg;

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
