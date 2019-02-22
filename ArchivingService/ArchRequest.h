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

class ArchFileToRead
{
public:
	ArchFileToRead(const ArchFile& archFile, const ArchRequestParam& param);
	~ArchFileToRead();

	void findData();

	ArchFilePartition::Info partitionToReadInfo();

	bool fillBuffer();
	bool getRecord(Hash* signalHash, ArchFileRecord* record);
	bool gotoNextRecord();

	bool hasData() { return m_hasDataToRead; }

	qint64 startTime() const { return m_startTime; }

	ArchFindResult findResult() const { return m_findResult; }

private:
	void getArchPartitionsInfo();
	ArchFindResult openPartitionToStartReading();

private:
	QString m_archFilePath;
	QString m_appSignalID;
	Hash m_hash = 0;
	quint32 m_requestID = 0;
	E::TimeType m_timeType = E::TimeType::System;
	qint64 m_startTime = 0;
	qint64 m_endTime = 0;

	//

	QVector<ArchFilePartition::Info> m_partitionsInfo;
	int m_partitionToReadIndex = -1;
	ArchFilePartition m_partitionToRead;
	qint64 m_startReadFromRecord = -1;

	ArchFindResult m_findResult = ArchFindResult::NotFound;

	ArchFileReadBuffer m_readBuffer;

	//

/*	static const int READ_BUFFER_SIZE = 200000;

	ArchFileRecord m_readBuffer[READ_BUFFER_SIZE];
	int m_recordsInBuffer = 0;
	int m_nextRecordIndex = 0;*/
	bool m_hasDataToRead = true;
};


//

class Archive;

class ArchRequest : public RunOverrideThread
{
public:
	ArchRequest(Archive& archive,
				const ArchRequestParam& param,
				std::shared_ptr<Network::GetAppSignalStatesFromArchiveNextReply> getNextReply,
				CircularLoggerShared logger);
	virtual ~ArchRequest() override;

	void run() override;

	quint32 requestID() const { return m_param.requestID(); }

	void nextDataRequired() { m_dataReady.store(false); m_nextDataRequired.store(true);  }

	bool isDataReady() const { return m_dataReady.load(); }

//	Network::GetAppSignalStatesFromArchiveNextReply& getNextReply() { return m_reply; }

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

	bool prepareArchFilesToRead();
	void prepareGetNextReply();
	void getSignalStates();

	bool getSingleFileNextRecord(Hash* hash, ArchFileRecord* record);
	bool getMultipleFilesNextRecord(Hash* hash, ArchFileRecord* record);		// return TRUE! if no more data exists

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
	Archive& m_archive;
	ArchRequestParam m_param;
	std::shared_ptr<Network::GetAppSignalStatesFromArchiveNextReply> m_getNextReply;
	CircularLoggerShared m_logger;
	qint64 m_startTime = 0;

	//

	QVector<ArchFileToRead*> m_archFilesToRead;

	// getNextRecord function variables
	//
	qint64 m_minTime = std::numeric_limits<qint64>::max();
	qint64 m_minTimeIndex = -1;

	int m_lastFileIndex = -1;
	qint64 m_lastRecordTime = 0;

	//

	std::atomic<bool> m_nextDataRequired = { false };
	std::atomic<bool> m_dataReady = { false };
	bool m_noMoreData = false;
	int m_sentStatesCount = 0;

	//Network::GetAppSignalStatesFromArchiveNextReply m_reply;
	QString m_errMsg;
};

typedef std::shared_ptr<ArchRequest> ArchRequestShared;

