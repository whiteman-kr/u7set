#pragma once

#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"
#include "../lib/Crc16.h"
#include "../lib/SimpleThread.h"

class ArchRequestParam;

class ArchFile
{
public:
	class PartitionInfo
	{
	public:
		QString fileName;
		QDateTime date;
		qint64 startTime;
	};

private:
#pragma pack(push, 1)

	struct Record
	{
		struct
		{
			qint64 archID;
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
	};

#pragma pack(pop)

	class Partition
	{
	public:
		Partition(const ArchFile& archFile, bool writable);

		qint64 recordsCount();

		bool write(qint64 partitionSystemTime, Record* buffer, int statesCount, qint64* totalFushedStatesCount);

		bool openForReading(qint64 partitionSystemTime);

		bool getFirstAndLastRecords(Record* first, Record* last, bool* noRecords);

		bool readRecord(qint64 recordIndex, Record* record);

		bool findStartPosition(E::TimeType timeType, qint64 startTime, qint64 endTime, bool* positionFound);

		bool close();

	private:
		QString getFileName(qint64 partitionStartTime);

		void moveToRecord(qint64 record);
		qint64 binarySearch(E::TimeType timeType, qint64 time);

		void closeFile();

	private:
		const ArchFile& m_archFile;
		bool m_isWritable = false;

		QFile m_file;

		bool m_pathIsExists = false;
		bool m_fileIsAligned = false;	// partition's file is aligned on sizeof(Record)

		qint64 m_startTime = -1;		// system start time of partition (acquired from partition's file name)
		qint64 m_size = -1;				// partition's file size

		static const qint64 FIRST_RECORD = 0;
		static const qint64 LAST_RECORD = -1;

		static const qint64 POSITION_NOT_FOUND = -999;
		static const qint64 READ_ERROR = -9999;
	};

	class RequestData
	{
	public:
		RequestData(ArchFile& archFile, const ArchRequestParam& param);

	public:
		quint32 requestID = 0;
		E::TimeType timeType = E::TimeType::System;
		qint64 startTime = 0;
		qint64 endTime = 0;

		//

		QVector<PartitionInfo> partitionsInfo;
		Partition partitionToRead;
	};

public:
	ArchFile(const Proto::ArchSignal& protoArchSignal, const QString& archFullPath);
	~ArchFile();

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

	bool findData(const ArchRequestParam& param);

	void shutdown(qint64 curPartition, qint64* totalFlushedStatesCount);

private:
	bool getArchPartitionsInfo(RequestData* rd);
	bool findStartPosition(RequestData* rd);
	void cancelRequest(quint32 requestID);

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

	Partition m_writablePartition;

	FastQueue<Record>* m_queue = nullptr;

	std::atomic<bool> m_requiredImmediatelyFlushing = { false };

	SimpleMutex m_flushMutex;

	//

	QHash<quint32, RequestData*> m_requestsData;

	const double QUEUE_EMERGENCY_LIMIT = 0.7;		// 70%
	const double QUEUE_EXPAND_LIMIT = 0.9;			// 90%

	static const int QUEUE_MIN_SIZE = 20;
	static const int QUEUE_MAX_SIZE = 1280;			// 20 * 2^6

	static Record m_buffer[QUEUE_MAX_SIZE];

	static const QString EXTENSION;
};

inline bool operator < (const ArchFile::PartitionInfo& p1, const ArchFile::PartitionInfo& p2) { return p1.startTime < p2.startTime; }

