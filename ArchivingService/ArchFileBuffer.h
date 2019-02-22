#pragma once

#include <QFile>

#include "ArchFileRecord.h"


class ArchFileBuffer
{
public:
	ArchFileBuffer(int bufSizeBytes);
	virtual ~ArchFileBuffer();

protected:
	qint64 m_bufSize = 0;
	char* m_buffer = nullptr;

	qint64 m_inBufSize = 0;
};

class ArchFileReadBuffer : public ArchFileBuffer
{
public:
	ArchFileReadBuffer(int bufSizeBytes);

	bool fillBuffer(QFile& file);
	bool hasRecordsInBuffer() const;

	bool getRecord(ArchFileRecord* record);
	bool moveToNextRecord();

	bool getRecordAndMoveToNext(ArchFileRecord* record);

private:
	void moveRemainingDataInBeginningOfBuffer();

private:
	qint64 m_recordStartPos = 0;
};

class ArchFileWriteBuffer : public ArchFileBuffer
{
public:
	ArchFileWriteBuffer(int bufSizeBytes);

	bool writeNextRecord(QFile& file, const ArchFileRecord& record);
	bool flushBuffer(QFile& file);
};



