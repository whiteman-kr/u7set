#pragma once

#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"
#include "../lib/Crc16.h"
#include "../lib/SimpleThread.h"
#include "../lib/CircularLogger.h"
#include "ArchFileRecord.h"

enum class ArchFindResult
{
	NotFound,
	Found,

	SearchError
};

class ArchFilePartition
{
public:
	struct Info
	{
		int index = -1;
		QString fileName;
		QDateTime date;
		qint64 startTime = 0;
		bool shortTerm = true;
		ArchFileRecord firstRecord;
		ArchFileRecord lastRecord;
	};

public:
	ArchFilePartition();

	void init(const QString& archFilePath, bool writable);

	virtual ~ArchFilePartition();

	qint64 recordsCount();

	bool write(qint64 partitionSystemTime, ArchFileRecord* buffer, int statesCount, qint64* totalFushedStatesCount);

	//

	bool openForReading(qint64 partitionSystemTime, bool shortTerm);
	bool getFirstAndLastRecords(ArchFileRecord* first, ArchFileRecord* last);
	bool gotoFirstRecord();
	bool gotoRecord(qint64 recordIndex);
	bool readRecord(qint64 recordIndex, ArchFileRecord* record);
	bool read(ArchFileRecord* recordBuffer, int maxRecordsToRead, int* readCount);

	bool checkTimesAndGetMoveDirection(E::TimeType requestedTimeType,
									   qint64 startTime,
									   qint64 endTime,
									   bool* hasData,
									   int* moveDirection);

	ArchFindResult binarySearch(E::TimeType timeType, qint64 time, qint64* startPosition);

	bool close();

private:
	QString getFileName(qint64 partitionStartTime, bool shortTerm);

	void moveToRecord(qint64 record);

	void closeFile();

private:
	QString m_archFilePath;
	bool m_isWritable = false;

	QFile m_file;

	bool m_pathIsExists = false;
	bool m_fileIsAligned = false;	// partition's file is aligned on sizeof(Record)

	qint64 m_startTime = -1;		// system start time of partition (acquired from partition's file name)
	qint64 m_size = -1;				// partition's file size in Bytes (multiple to sizeof(Record))
	qint64 m_recordCount = -1;		// partition's record count

	static const qint64 FIRST_RECORD = 0;
	static const qint64 LAST_RECORD = -1;
};

inline bool operator < (const ArchFilePartition::Info& p1, const ArchFilePartition::Info& p2);

class ArchFileReadBuffer
{
public:
	ArchFileReadBuffer(int bufSizeBytes);
	~ArchFileReadBuffer();

	qint64 readDataFromFile(QFile& file);

	bool hasRecordsInBuffer() const;

	bool getNextRecord(ArchFileRecord* record);

private:
	qint64 m_bufSize = 0;
	char* m_buffer = nullptr;

	//

	qint64 m_inBufSize = 0;
	qint64 m_recordStartPos = 0;
};

class ArchFile
{
public:
	ArchFile(const Proto::ArchSignal& protoArchSignal, CircularLoggerShared log);
	~ArchFile();

	void setArchFullPath(const QString& archFullPath);

	bool pushState(qint64 archID, const SimpleAppSignalState& state);
	bool flush(qint64 curPartition, qint64* totalFushedStatesCount, bool flushAnyway);

	void setRequiredImmediatelyFlushing(bool b) { m_requiredImmediatelyFlushing.store(b); }
	bool isRequiredImmediatelyFlushing() const { return m_requiredImmediatelyFlushing.load(); }

	Hash hash() const { return m_hash; }
	QString appSignalID() const { return m_appSignalID; }

	bool isAnalog() const { return m_isAnalog; }

	bool queueIsEmpty() const { return m_queue->isEmpty(); }
	bool isEmergency() const;
	QString path() const { return m_path; }

	static QVector<ArchFilePartition::Info> getArchPartitionsInfo(const QString& path);
	static QString getPartitionFileName(const QString& archFilePath, const ArchFilePartition::Info& pi);

	void shutdown(qint64 curPartition, qint64* totalFlushedStatesCount);

	bool maintenance(qint64 currentPartition,
					 qint64 msShortTermPeriod,
					 qint64 msLongTermPeriod,
					 int* deletedCount,
					 int* packedCount);
private:
	void startMaintenance();
	void stopMaintenance();

	bool packPartitions(const QVector<ArchFilePartition::Info>& partitionsInfo,
							qint64 currentPartition,
							qint64 msShortTermPeriod,
							int* packedCount);
	bool packAnalogSignalPartition(const ArchFilePartition::Info& pi);
	bool writeLtaFile(QFile& ltaFile, const char* buffer, int size);

	bool packDiscreteSignalPartition(const ArchFilePartition::Info& pi);

	bool deleteOldPartitions(const QVector<ArchFilePartition::Info>& partitionsInfo,
								qint64 currentPartition,
								qint64 msLongTermPeriod,
								int* deletedCount);

	bool isRwAccessRequested() { return m_rwAccessRequested.load(); }

	QString getPartitionFileName(const ArchFilePartition::Info& pi);

private:
	CircularLoggerShared m_log;
	Hash m_hash = 0;
	QString m_appSignalID;
	bool m_isAnalog = false;

	//

	bool m_lastRecordInitialized = false;
	ArchFileRecord m_lastRecord;

	QMutex m_fileInMaintenanceMutex;
	bool m_fileInMaintenance = false;
	std::atomic<bool> m_rwAccessRequested = { false };

	//

	QString m_path;

	//

	ArchFilePartition m_writablePartition;

	FastQueue<ArchFileRecord>* m_queue = nullptr;

	std::atomic<bool> m_requiredImmediatelyFlushing = { false };

	SimpleMutex m_flushMutex;

	const double QUEUE_EMERGENCY_LIMIT = 0.7;		// 70%
	const double QUEUE_EXPAND_LIMIT = 0.9;			// 90%

	static const int QUEUE_MIN_SIZE = 20;
	static const int QUEUE_MAX_SIZE = 1280;			// 20 * 2^6

	static ArchFileRecord m_buffer[QUEUE_MAX_SIZE];
};


