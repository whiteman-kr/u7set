#pragma once

#include "../lib/AppSignal.h"
#include "../lib/WUtils.h"
#include "../lib/Crc16.h"
#include "../lib/SimpleThread.h"

class ArchSignal;
class Archive;
class ArchRequestParam;

class FileArchWriter;

class ArchFile
{
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

	};

#pragma pack(pop)

	class Partition
	{
	public:
		Partition(const ArchFile& archFile, bool writable);

		qint64 recordsCount();

		bool write(qint64 partitionSystemTime, Record* buffer, int statesCount, qint64* totalFushedStatesCount);
		bool close();

	private:
		void closeFile();

	private:
		const ArchFile& m_archFile;
		bool m_isWritable = false;

		QFile m_file;

		bool m_pathIsExists = false;
		bool m_fileIsAligned = false;	// partition's file is aligned on sizeof(Record)

		qint64 m_startTime = -1;		// system start time of partition (acquired from partition's file name)
		qint64 m_size = -1;				// partition's file size
	};

	class RequestContext
	{
		ArchRequestParam param;

		SimpleAppSignalState statesBuffer[1000];
	};

public:
	ArchFile();
	~ArchFile();

	void init(Archive* archive, ArchSignal* archSignal);

	bool pushState(qint64 archID, const SimpleAppSignalState& state);

	bool flush(qint64 curPartition, qint64* totalFushedStatesCount, bool flushAnyway);

	bool queueIsEmpty() const { return m_queue->isEmpty(); }

	bool isEmergency() const;

	bool findData(const ArchRequestParam& param);

	void shutdown(qint64 curPartition, qint64* totalFlushedStatesCount);

	void setRequiredImmediatelyFlushing(bool b) { m_requiredImmediatelyFlushing.store(b); }
	bool isRequiredImmediatelyFlushing() const { return m_requiredImmediatelyFlushing.load(); }

	QString path() const { return m_path; }

private:
	bool privateFindData(RequestContext* rc);

private:
	Archive* m_archive = nullptr;
	ArchSignal* m_archSignal = nullptr;

	QString m_path;

	//

	Partition m_writablePartition;

	FastQueue<Record>* m_queue = nullptr;

	std::atomic<bool> m_requiredImmediatelyFlushing = { false };

	SimpleMutex m_flushMutex;

	const double QUEUE_EMERGENCY_LIMIT = 0.7;		// 70%
	const double QUEUE_EXPAND_LIMIT = 0.9;			// 90%

	static const int QUEUE_MIN_SIZE = 20;
	static const int QUEUE_MAX_SIZE = 1280;			// 20 * 2^6

	static Record m_buffer[QUEUE_MAX_SIZE];

	static const QString EXTENSION;

	//

	QHash<quint32, RequestContext*> m_requestContexts;
};
