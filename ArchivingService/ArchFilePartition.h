#pragma once

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
	};

public:
	ArchFilePartition();

	void init(const QString& archFilePath, bool writable);

	virtual ~ArchFilePartition();

	qint64 recordsCount() const;
	qint64 size() const;

	bool write(qint64 partitionSystemTime, ArchFileRecord* buffer, int statesCount, qint64* totalFushedStatesCount);

	//

	bool openForReading(qint64 partitionSystemTime, bool shortTerm);
	bool openForReading(const QString& pathFileName);

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

	inline static const qint64 FIRST_RECORD = 0;
	inline static const qint64 LAST_RECORD = -1;
};

inline bool operator < (const ArchFilePartition::Info& p1, const ArchFilePartition::Info& p2)
{
	if (p1.startTime == p2.startTime)
	{
		return p1.shortTerm;			// if start times is equal short term partitions consider less then long term
	}

	return p1.startTime < p2.startTime;
}
