#pragma once

#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"
#include "../lib/SimpleThread.h"
#include "../OnlineLib/CircularLogger.h"

#include "ArchFileBuffer.h"

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

	QFile& file() { return m_file; }

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

class ArchFile
{
public:
	static const int QUEUE_MAX_SIZE;

private:
	static const int QUEUE_MIN_SIZE;

	static const double QUEUE_EMERGENCY_LIMIT;
	static const double QUEUE_EXPAND_LIMIT;
	static const double QUEUE_REDUCTION_LIMIT;

public:
	ArchFile(const Proto::ArchSignal& protoArchSignal, CircularLoggerShared log);
	~ArchFile();

	void setArchFullPath(const QString& archFullPath);

	bool pushState(const SimpleAppSignalState& state);

	bool flush(qint64 curPartition,
			   qint64* totalFushedStatesCount,
			   bool flushAnyway,
			   int minQueueSizeForFlushing,
			   ArchFileRecord* buffer,
			   int bufferSize,
			   const QThread* thread);

	void setRequiredImmediatelyFlushing(bool b) { m_requiredImmediatelyFlushing.store(b); }
	bool isRequiredImmediatelyFlushing() const { return m_requiredImmediatelyFlushing.load(); }

	Hash hash() const { return m_hash; }
	QString appSignalID() const { return m_appSignalID; }

	bool isAnalog() const { return m_isAnalog; }
	bool isEmergency() const;

	QString path() const { return m_path; }

	static QVector<ArchFilePartition::Info> getArchPartitionsInfo(const QString& path);
	static QString getPartitionFileName(const QString& archFilePath, const ArchFilePartition::Info& pi);

	void shutdown(qint64 curPartition,
				  qint64* totalFlushedStatesCount,
				  ArchFileRecord* buffer,
				  int bufferSize,
				  const QThread* thread);

	bool maintenance(qint64 currentPartition,
					 qint64 msShortTermPeriod,
					 qint64 msLongTermPeriod,
					 int* deletedCount,
					 int* packedCount,
					 const RunOverrideThread* thread);
private:
	void startMaintenance();
	void stopMaintenance();

	bool packPartitions(const QVector<ArchFilePartition::Info>& partitionsInfo,
							qint64 currentPartition,
							qint64 msShortTermPeriod,
							int* packedCount,
							const RunOverrideThread* thread);

	bool packAnalogSignalPartition(const ArchFilePartition::Info& pi, const RunOverrideThread* thread);
	bool writeLtaFile(QFile& ltaFile, const char* buffer, int size);

	bool packDiscreteSignalPartition(const ArchFilePartition::Info& pi);

	bool deleteOldPartitions(const QVector<ArchFilePartition::Info>& partitionsInfo,
								qint64 currentPartition,
								qint64 msLongTermPeriod,
								int* deletedCount,
								const RunOverrideThread *thread);

	QString getPartitionFileName(const ArchFilePartition::Info& pi);

	void controlQueueSizeBeforePush(const QThread* thread);

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

	int m_statesCountAfterExpand = -1;

	//

	QString m_path;

	//

	ArchFilePartition m_writablePartition;

	FastThreadSafeQueue<ArchFileRecord> m_queue;

	std::atomic<bool> m_requiredImmediatelyFlushing = { false };
};


