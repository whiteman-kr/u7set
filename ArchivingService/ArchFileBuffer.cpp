#include "../lib/WUtils.h"

#include "ArchFileBuffer.h"

// ----------------------------------------------------------------------------------------------------------------------
//
// ArchFileBuffer class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

ArchFileBuffer::ArchFileBuffer(int bufSizeBytes) :
	m_bufSize(bufSizeBytes)
{
	const int MIN_BUF_SIZE = 1000 * ArchFileRecord::SIZE;			// ~38 Kb
	const int MAX_BUF_SIZE = 100 * 1000 * ArchFileRecord::SIZE;		// ~3.8 Mb

	if (m_bufSize < MIN_BUF_SIZE)
	{
		m_bufSize = MIN_BUF_SIZE;
	}
	else
	{
		if (m_bufSize > MAX_BUF_SIZE)
		{
			m_bufSize = MAX_BUF_SIZE;
		}
	}

	m_bufSize = (m_bufSize / ArchFileRecord::SIZE) * ArchFileRecord::SIZE;		// make m_bufSize multiple to ArchFileRecord::SIZE

	m_buffer = new char[m_bufSize];
}

ArchFileBuffer::~ArchFileBuffer()
{
	if (m_buffer != nullptr)
	{
		delete [] m_buffer;
	}
}

// ----------------------------------------------------------------------------------------------------------------------
//
// ArchFileReadBuffer class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

ArchFileReadBuffer::ArchFileReadBuffer(int bufSizeBytes) :
	ArchFileBuffer(bufSizeBytes)
{
}

bool ArchFileReadBuffer::fillBuffer(QFile& file)
{
	qint64 read = file.read(m_buffer + m_inBufSize, m_bufSize - m_inBufSize);

	if (read == -1)
	{
		// error
		return false;
	}

	m_inBufSize += read;

	return true;
}

bool ArchFileReadBuffer::hasRecordsInBuffer() const
{
	return (m_inBufSize - m_recordStartPos) >= ArchFileRecord::SIZE;
}

bool ArchFileReadBuffer::getRecord(ArchFileRecord* record)
{
	TEST_PTR_RETURN_FALSE(record);

	if (m_recordStartPos + ArchFileRecord::SIZE > m_inBufSize)
	{
		return false;
	}

	while(m_recordStartPos + ArchFileRecord::SIZE <= m_inBufSize)
	{
		ArchFileRecord* recordPtr = reinterpret_cast<ArchFileRecord*>(m_buffer + m_recordStartPos);

		if (recordPtr->isNotCorrupted() == true)
		{
			memcpy(record, recordPtr, ArchFileRecord::SIZE);
			return true;
		}

		// record is corrupted
		// shift m_recordStartPos on 1 byte

		m_recordStartPos++;
	}

	moveRemainingDataInBeginningOfBuffer();

	return false;
}

bool ArchFileReadBuffer::moveToNextRecord()
{
	if (m_recordStartPos + ArchFileRecord::SIZE > m_inBufSize)
	{
		return false;
	}

	m_recordStartPos += ArchFileRecord::SIZE;

	if (m_recordStartPos + ArchFileRecord::SIZE <= m_inBufSize)
	{
		return true;
	}

	moveRemainingDataInBeginningOfBuffer();

	return false;
}

bool ArchFileReadBuffer::getRecordAndMoveToNext(ArchFileRecord* record)
{
	bool result = getRecord(record);

	moveToNextRecord();

	return result;
}

void ArchFileReadBuffer::moveRemainingDataInBeginningOfBuffer()
{
	qint64 remainigDataSize = m_inBufSize - m_recordStartPos;

	assert(remainigDataSize >= 0);
	assert(remainigDataSize < ArchFileRecord::SIZE);

	if (remainigDataSize > 0)
	{
		memcpy(m_buffer + 0, m_buffer + m_recordStartPos, remainigDataSize);
	}

	m_inBufSize = remainigDataSize;
	m_recordStartPos = 0;
}

// ----------------------------------------------------------------------------------------------------------------------
//
// ArchFileWriteBuffer class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

ArchFileWriteBuffer::ArchFileWriteBuffer(int bufSizeBytes) :
	ArchFileBuffer(bufSizeBytes)
{
}

bool ArchFileWriteBuffer::writeNextRecord(QFile& file, const ArchFileRecord& record)
{
	assert((m_inBufSize % ArchFileRecord::SIZE) == 0);
	assert(m_inBufSize <= m_bufSize);

	if (m_inBufSize + ArchFileRecord::SIZE > m_bufSize)
	{
		bool res = flushBuffer(file);

		if (res == false)
		{
			return false;
		}
	}

	memcpy(m_buffer + m_inBufSize, &record, ArchFileRecord::SIZE);

	m_inBufSize += ArchFileRecord::SIZE;

	return true;
}

bool ArchFileWriteBuffer::flushBuffer(QFile& file)
{
	assert((m_inBufSize % ArchFileRecord::SIZE) == 0);

	if (m_inBufSize > 0)
	{
		qint64 written = file.write(m_buffer, m_inBufSize);

		if (written != m_inBufSize)
		{
			return false;
		}

		m_inBufSize = 0;
	}

	return true;
}

