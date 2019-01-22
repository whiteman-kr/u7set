#pragma once

#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"
#include "../lib/Crc16.h"
#include "../lib/SimpleThread.h"

#pragma pack(push, 1)

struct ArchFileRecord
{
	struct
	{
//			qint64 archID;
		qint64 systemTime;
		qint64 plantTime;

		AppSignalStateFlags flags;
		double value;
	} state;

	quint16 crc16;

	void calcCRC16() { crc16 = calcCrc16(&state, sizeof(state)); }
	bool isValid() const;

	bool timeLessThen(E::TimeType timeType, qint64 time);
	bool timeLessOrEqualThen(E::TimeType timeType, qint64 time);

	bool timeGreateThen(E::TimeType timeType, qint64 time);
	bool timeGreateOrEqualThen(E::TimeType timeType, qint64 time);

	qint64 getTime(E::TimeType timeType);
};

#pragma pack(pop)

enum class ArchFindResult
{
	NotFound,
	Found,

	SearchError
};

class ArchFilePartition
{
public:
	ArchFilePartition();

	void init(const QString& archFilePath, bool writable);

	virtual ~ArchFilePartition();

	qint64 recordsCount();

	bool write(qint64 partitionSystemTime, ArchFileRecord* buffer, int statesCount, qint64* totalFushedStatesCount);

	//

	bool openForReading(qint64 partitionSystemTime);
	bool getFirstAndLastRecords(ArchFileRecord* first, ArchFileRecord* last, bool* noRecords);
	bool readRecord(qint64 recordIndex, ArchFileRecord* record);
	bool read(ArchFileRecord* recordBuffer, int maxRecordsToRead, int* readCount);

	ArchFindResult findStartPosition(E::TimeType timeType, qint64 startTime, qint64 endTime, qint64* startRecord);

	bool close();

private:
	QString getFileName(qint64 partitionStartTime);

	void moveToRecord(qint64 record);
	ArchFindResult binarySearch(E::TimeType timeType, qint64 time, qint64* startPosition);

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

class ArchFile
{
public:
	ArchFile(const Proto::ArchSignal& protoArchSignal);
	~ArchFile();

	void setArchFullPath(const QString& archFullPath);

	bool pushState(qint64 archID, const SimpleAppSignalState& state);
	bool flush(qint64 curPartition, qint64* totalFushedStatesCount, bool flushAnyway);

	void setRequiredImmediatelyFlushing(bool b) { m_requiredImmediatelyFlushing.store(b); }
	bool isRequiredImmediatelyFlushing() const { return m_requiredImmediatelyFlushing.load(); }

	Hash hash() const { return m_hash; }
	QString appSignalID() const { return m_appSignalID; }

	bool canReadWrite() const { return m_canReadWrite; }
	void setCanReadWrite(bool canReadWrite) { m_canReadWrite = canReadWrite; }

	bool isInitialized() const { return m_isInitialized; }
	void setInitialized(bool initialized) { m_isInitialized = initialized; }

	bool isAnalog() const { return m_isAnalog; }

	bool queueIsEmpty() const { return m_queue->isEmpty(); }
	bool isEmergency() const;
	QString path() const { return m_path; }

	void shutdown(qint64 curPartition, qint64* totalFlushedStatesCount);

public:
	static const QString EXTENSION;

private:
	Hash m_hash = 0;
	QString m_appSignalID;
	bool m_isAnalog = false;

	//

	bool m_isInitialized = false;
	bool m_canReadWrite = false;

	//

	SimpleAppSignalState m_lastState;

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
