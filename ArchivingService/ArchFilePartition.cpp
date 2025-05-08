#include <QDir>
#include <CommonLib/Times.h>
#include <CommonLib/ConstStrings.h>

#include "ArchFilePartition.h"
#include "BinSearch.h"
#include "../UtilsLib/WUtils.h"

// -----------------------------------------------------------------------------------------------------------------------
//
// ArchFilePartition class implementation
//
// -----------------------------------------------------------------------------------------------------------------------

ArchFilePartition::ArchFilePartition()
{
}

void ArchFilePartition::init(const QString& archFilePath, bool writable)
{
	m_archFilePath = archFilePath;
	m_isWritable = writable;
}

ArchFilePartition::~ArchFilePartition()
{
	closeFile();
}

qint64 ArchFilePartition::recordsCount() const
{
	if (m_size < 0)
	{
		assert(false);
		return 0;
	}

	return m_size / ArchFileRecord::SIZE;
}

qint64 ArchFilePartition::size() const
{
	return m_size;
}

bool ArchFilePartition::write(qint64 partition, ArchFileRecord* buffer, int statesCount, qint64* totalFushedStatesCount)
{
	TEST_PTR_RETURN_FALSE(buffer);
	TEST_PTR_RETURN_FALSE(totalFushedStatesCount);

	if (m_isWritable == false)
	{
		assert(false);
		return false;
	}

	if (m_startTime == -1)
	{
		m_startTime = partition;
	}
	else
	{
		if (m_startTime != partition)
		{
			closeFile();

			m_startTime = partition;
		}
	}

	if (m_file.isOpen() == false)
	{
		if (m_pathIsExists == false)
		{
			QDir d;

			m_pathIsExists = d.mkpath(m_archFilePath);
		}

		QString fileName = getFileName(partition, true);

		m_file.setFileName(fileName);

		if (m_file.open(QIODevice::Append) == false)
		{
			return false;
		}

		if (m_fileIsAligned == false)
		{
			QFileInfo fi(m_file);

			m_size = fi.size();

			if ((m_size % sizeof(ArchFileRecord)) != 0)
			{
				m_size = (m_size / sizeof(ArchFileRecord)) * sizeof(ArchFileRecord);

				bool res = m_file.seek(m_size);

				if (res == true)
				{
					m_fileIsAligned = true;
				}
			}
			else
			{
				m_fileIsAligned = true;
			}
		}
	}

	qint64 sizeToWrite = statesCount * sizeof(ArchFileRecord);

	qint64 written = m_file.write(reinterpret_cast<const char*>(buffer), sizeToWrite);

	if (written == -1)
	{
		return false;
	}

	m_file.flush();

	if (sizeToWrite != written)
	{
		m_fileIsAligned = false;
	}
	else
	{
		m_size += written;
	}

	*totalFushedStatesCount += statesCount;

	//	qDebug() << C_STR(QString("Flush %1 states %2").arg(m_file.fileName()).arg(statesCount));

	return true;
}

bool ArchFilePartition::openForReading(qint64 partitionSystemTime, bool shortTerm)
{
	QString pathFileName = getFileName(partitionSystemTime, shortTerm);

	return openForReading(pathFileName);
}

bool ArchFilePartition::openForReading(const QString& pathFileName)
{
	closeFile();

	m_file.setFileName(pathFileName);

	bool result = m_file.open(QIODevice::ReadOnly);

	if (result == false)
	{
		return false;
	}

	//	qDebug() << "Open for reading" << C_STR(fileName);

	QFileInfo fi(m_file);

	m_size = (fi.size() / sizeof(ArchFileRecord)) * sizeof(ArchFileRecord);
	m_recordCount = fi.size() / sizeof(ArchFileRecord);

	return result;
}

bool ArchFilePartition::getFirstAndLastRecords(ArchFileRecord* first, ArchFileRecord* last)
{
	TEST_PTR_RETURN_FALSE(first);
	TEST_PTR_RETURN_FALSE(last);

	if (m_file.isOpen() == false)
	{
		assert(false);
		return false;
	}

	assert(m_recordCount >= 0);

	if (m_recordCount == 0)
	{
		return false;
	}

	bool result = true;

	result &= readRecord(FIRST_RECORD, first);
	result &= readRecord(LAST_RECORD, last);

	return result;
}

bool ArchFilePartition::gotoFirstRecord()
{
	return gotoRecord(FIRST_RECORD);
}

bool ArchFilePartition::gotoRecord(qint64 recordIndex)
{
	if (recordIndex == LAST_RECORD)
	{
		assert(m_recordCount != -1);
		recordIndex = m_recordCount - 1;
	}

	if (recordIndex < 0)
	{
		recordIndex = 0;
	}

	return m_file.seek(recordIndex * sizeof(ArchFileRecord));
}


bool ArchFilePartition::readRecord(qint64 recordIndex, ArchFileRecord* record)
{
	TEST_PTR_RETURN_FALSE(record);

	bool res = gotoRecord(recordIndex);

	if (res == false)
	{
		return false;
	}

	qint64 read =  m_file.read(reinterpret_cast<char*>(record), sizeof(ArchFileRecord));

	if (read != sizeof(ArchFileRecord))
	{
		return false;
	}

	if (record->isNotCorrupted())
	{
		return true;
	}

	return false;
}

bool ArchFilePartition::read(ArchFileRecord* recordBuffer, int maxRecordsToRead, int* readCount)
{
	TEST_PTR_RETURN_FALSE(recordBuffer);
	TEST_PTR_RETURN_FALSE(readCount);

	// HERE:
	//	do read to intermediate buffer
	//	and do records consistency checking
	//	align to record and return valid records

	qint64 readSize =  m_file.read(reinterpret_cast<char*>(recordBuffer), sizeof(ArchFileRecord) * maxRecordsToRead);

	*readCount = static_cast<int>(readSize / sizeof(ArchFileRecord));

	return true;
}

bool ArchFilePartition::checkTimesAndGetMoveDirection(	E::TimeType requestedTimeType,
	qint64 startTime,
	qint64 endTime,
	bool* hasData,
	int* moveDirection)
{
	TEST_PTR_RETURN_FALSE(hasData);
	TEST_PTR_RETURN_FALSE(moveDirection);

	ArchFileRecord firstRecord;
	ArchFileRecord lastRecord;

	bool res = getFirstAndLastRecords(&firstRecord, &lastRecord);

	if (res == false)
	{
		return false;
	}

	//

	qint64 firstRecordTime =  firstRecord.getTime(requestedTimeType);
	qint64 lastRecordTime =  lastRecord.getTime(requestedTimeType);

	//

	if ((startTime >= firstRecordTime && startTime <= lastRecordTime) ||
		(endTime >= firstRecordTime && endTime <= lastRecordTime) ||
		(startTime < firstRecordTime && endTime > lastRecordTime))
	{
		*hasData = true;
	}

	//

	if (startTime < firstRecordTime)
	{
		*moveDirection = -1;			// move to previous partition
		return true;
	}

	if (startTime > lastRecordTime)
	{
		*moveDirection = 1;			// move to next partition
		return true;
	}

	//	firstRecord.getTime <=	requestedTime  <= lastRecord.getTime

	*moveDirection = 0;
	return true;
}

bool ArchFilePartition::close()
{
	closeFile();

	return true;
}

QString ArchFilePartition::getFileName(qint64 partitionStartTime, bool shortTerm)
{
	QDateTime date = QDateTime::fromMSecsSinceEpoch(partitionStartTime, TIME_ZONE_UTC);

	QString extension;

	if (shortTerm == true)
	{
		extension = SHORT_TERM_ARCHIVE_EXTENSION;
	}
	else
	{
		extension = LONG_TERM_ARCHIVE_EXTENSION;
	}

	QString fileName = QString("%1/%2_%3_%4_%5_%6.%7").
					   arg(m_archFilePath).
					   arg(date.date().year()).
					   arg(date.date().month(), 2, 10, Latin1Char::ZERO).
					   arg(date.date().day(), 2, 10, Latin1Char::ZERO).
					   arg(date.time().hour(), 2, 10, Latin1Char::ZERO).
					   arg(date.time().minute(), 2, 10, Latin1Char::ZERO).
					   arg(extension);

	return fileName;
}

void ArchFilePartition::moveToRecord(qint64 record)
{
	assert(m_file.isOpen() == true);

	m_file.seek(record * sizeof(ArchFileRecord));
}

ArchFindResult ArchFilePartition::binarySearch(E::TimeType timeType, qint64 time, qint64* recordIndex)
{
	if (recordIndex == nullptr)
	{
		assert(false);
		return ArchFindResult::SearchError;
	}

	*recordIndex = -1;

	qint64 recordCount  = recordsCount();

	if (recordCount == 0)
	{
		return ArchFindResult::NotFound;
	}

	bool result = true;

	ArchFileRecord leftRecord;
	ArchFileRecord rightRecord;

	result &= readRecord(0, &leftRecord);
	result &= readRecord(recordCount - 1, &rightRecord);

	if (result == false)
	{
		return ArchFindResult::SearchError;
	}

	BinSearch<qint64> binSearch(time, recordCount, leftRecord.getTime(timeType), rightRecord.getTime(timeType));

	do
	{
		BinSearchResult bsResult = binSearch.result();

		switch(bsResult)
		{
		case BinSearchResult::RequireNextItem:
		{
			qint64 requiredRecordNo = binSearch.nextItemIndex();

			ArchFileRecord requiredRecord;

			result = readRecord(requiredRecordNo, &requiredRecord);

			if (result == false)
			{
				return ArchFindResult::SearchError;
			}

			binSearch.checkNextItem(requiredRecord.getTime(timeType));
		}
		break;

		case BinSearchResult::Found:
			*recordIndex = binSearch.foundIndex();
			return ArchFindResult::Found;

		case BinSearchResult::NotFound:
			return ArchFindResult::NotFound;

		case BinSearchResult::SearchError:
			return ArchFindResult::SearchError;

		default:
			assert(false);
			break;
		}
	}
	while(true);

	return ArchFindResult::SearchError;
}

void ArchFilePartition::closeFile()
{
	if (m_file.isOpen() == true)
	{
		m_file.close();
	}

	//	m_pathIsExists = false;
	m_fileIsAligned = false;

	m_startTime = -1;
	m_size = -1;
}
