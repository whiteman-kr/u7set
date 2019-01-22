#include "ArchFile.h"

#include "ArchWriterThread.h"
#include "BinSearch.h"
#include "ArchRequest.h"

// -----------------------------------------------------------------------------------------------------------------------
//
// ArchFileRecord struct implementation
//
// -----------------------------------------------------------------------------------------------------------------------

bool ArchFileRecord::isValid() const
{
	return calcCrc16(this, sizeof(ArchFileRecord)) == 0;
}

bool ArchFileRecord::timeLessThen(E::TimeType timeType, qint64 time)
{
	assert(isValid());

	switch(timeType)
	{
	case E::TimeType::System:
		return state.systemTime < time;

	case E::TimeType::Plant:
		return state.plantTime < time;

	default:
		assert(false);
	}

	return false;
}

bool ArchFileRecord::timeLessOrEqualThen(E::TimeType timeType, qint64 time)
{
	assert(isValid());

	switch(timeType)
	{
	case E::TimeType::System:
		return state.systemTime <= time;

	case E::TimeType::Plant:
		return state.plantTime <= time;

	default:
		assert(false);
	}

	return false;
}

bool ArchFileRecord::timeGreateThen(E::TimeType timeType, qint64 time)
{
	assert(isValid());

	switch(timeType)
	{
	case E::TimeType::System:
		return state.systemTime > time;

	case E::TimeType::Plant:
		return state.plantTime > time;

	default:
		assert(false);
	}

	return false;
}

bool ArchFileRecord::timeGreateOrEqualThen(E::TimeType timeType, qint64 time)
{
	assert(isValid());

	switch(timeType)
	{
	case E::TimeType::System:
		return state.systemTime >= time;

	case E::TimeType::Plant:
		return state.plantTime >= time;

	default:
		assert(false);
	}

	return false;
}

qint64 ArchFileRecord::getTime(E::TimeType timeType)
{
	switch(timeType)
	{
	case E::TimeType::Plant:
		return state.plantTime;

	case E::TimeType::System:
		return state.systemTime;

	case E::TimeType::Local:
	case E::TimeType::ArchiveId:
		assert(false);				// not implemented now
		return 0;
	}

	assert(false);					// unknown time type
	return 0;
}

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

qint64 ArchFilePartition::recordsCount()
{
	if (m_size < 0)
	{
		assert(false);
		return 0;
	}

	return m_size / sizeof(ArchFileRecord);
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

		QString fileName = getFileName(partition);

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

bool ArchFilePartition::openForReading(qint64 partitionSystemTime)
{
	closeFile();

	QString fileName = getFileName(partitionSystemTime);

	m_file.setFileName(fileName);

	bool result = m_file.open(QIODevice::ReadOnly);

	if (result == false)
	{
		return false;
	}

	qDebug() << "Open for reading" << C_STR(fileName);

	QFileInfo fi(m_file);

	m_size = (fi.size() / sizeof(ArchFileRecord)) * sizeof(ArchFileRecord);
	m_recordCount = fi.size() / sizeof(ArchFileRecord);

	return result;
}

bool ArchFilePartition::getFirstAndLastRecords(ArchFileRecord* first, ArchFileRecord* last, bool* noRecords)
{
	TEST_PTR_RETURN_FALSE(first);
	TEST_PTR_RETURN_FALSE(last);
	TEST_PTR_RETURN_FALSE(noRecords);

	if (m_file.isOpen() == false)
	{
		assert(false);
		return false;
	}

	assert(m_recordCount >= 0);

	if (m_recordCount == 0)
	{
		*noRecords = true;
		return true;
	}

	bool result = true;

	result &= readRecord(FIRST_RECORD, first);
	result &= readRecord(LAST_RECORD, last);

	return result;
}

bool ArchFilePartition::readRecord(qint64 recordIndex, ArchFileRecord* record)
{
	TEST_PTR_RETURN_FALSE(record);

	if (recordIndex == LAST_RECORD)
	{
		recordIndex = m_recordCount - 1;
	}

	if (recordIndex < 0)
	{
		recordIndex = 0;
	}

	m_file.seek(recordIndex * sizeof(ArchFileRecord));

	qint64 read =  m_file.read(reinterpret_cast<char*>(record), sizeof(ArchFileRecord));

	if (read != sizeof(ArchFileRecord))
	{
		return false;
	}

	if (record->isValid() == false)
	{
		return false;
	}

	return true;
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

	*readCount = readSize / static_cast<int>(sizeof(ArchFileRecord));

	return true;
}

ArchFindResult ArchFilePartition::findStartPosition(E::TimeType timeType, qint64 startTime, qint64 endTime, qint64* startRecord)
{
	if (startRecord == nullptr)
	{
		assert(false);
		return ArchFindResult::SearchError;
	}

	ArchFileRecord firstRecord;
	ArchFileRecord lastRecord;
	bool noRecords = false;

	bool res = getFirstAndLastRecords(&firstRecord, &lastRecord, &noRecords);

	if (res == false)
	{
		return ArchFindResult::SearchError;
	}

	if (noRecords == true)
	{
		return ArchFindResult::NotFound;
	}

	//	S - request start time
	//	E - request endTime
	//	F - partition first record time
	//	L - partition last record time
	//
	//							S					E
	//							|<---- REQUEST ---->|
	//
	//													F				L
	// 1)												[== PARTITION ==]	F > E				noPos
	//
	//								F				L
	// 2)							[== PARTITION ==]						F > S && F <= E		pos == 0
	//
	//		F				L
	// 3)	[== PARTITION ==]												L < S				goto next partition
	//
	//					F				L
	// 4)				[== PARTITION ==]									F <= S				binary search pos
	//

	// case 1)

	if (firstRecord.timeGreateThen(timeType, endTime) == true)
	{
		return ArchFindResult::NotFound;
	}

	// case 2)

	if (firstRecord.timeGreateThen(timeType, startTime) == true &&
		firstRecord.timeLessOrEqualThen(timeType, endTime) == true)
	{
		*startRecord = 0;
		moveToRecord(0);
		return ArchFindResult::Found;
	}

	// case 3)

	if (lastRecord.timeLessThen(timeType, startTime) == true)
	{
		return ArchFindResult::NotFound;
	}

	// case 4)

	if (firstRecord.timeLessOrEqualThen(timeType, startTime) == true)
	{
		ArchFindResult result = binarySearch(timeType, startTime, startRecord);

		if (result == ArchFindResult::Found)
		{
			moveToRecord(*startRecord);
		}

		return result;
	}

	return ArchFindResult::NotFound;
}

bool ArchFilePartition::close()
{
	closeFile();

	return true;
}

QString ArchFilePartition::getFileName(qint64 partitionStartTime)
{
	QDateTime date = QDateTime::fromMSecsSinceEpoch(partitionStartTime, Qt::UTC);

	QString fileName = QString("%1/%2_%3_%4_%5_%6.%7").
							arg(m_archFilePath).
							arg(date.date().year()).
							arg(QString().sprintf("%02d", date.date().month())).
							arg(QString().sprintf("%02d", date.date().day())).
							arg(QString().sprintf("%02d", date.time().hour())).
							arg(QString().sprintf("%02d", date.time().minute()),
							ArchFile::EXTENSION);

	return fileName;
}

void ArchFilePartition::moveToRecord(qint64 record)
{
	assert(m_file.isOpen() == true);

	m_file.seek(record * sizeof(ArchFileRecord));
}

ArchFindResult ArchFilePartition::binarySearch(E::TimeType timeType, qint64 time, qint64* startPosition)
{
	if (startPosition == nullptr)
	{
		assert(false);
		return ArchFindResult::SearchError;
	}

	*startPosition = -1;

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
			*startPosition = binSearch.foundIndex();
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
	while(1);

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


// ----------------------------------------------------------------------------------------------------------------------
//
// ArchFile class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

ArchFileRecord ArchFile::m_buffer[ArchFile::QUEUE_MAX_SIZE];

const QString ArchFile::EXTENSION = "saf";		// Signal Archive File

ArchFile::ArchFile(const Proto::ArchSignal& protoArchSignal)
{
	m_hash = protoArchSignal.hash();
	m_appSignalID = QString::fromStdString(protoArchSignal.appsignalid());
	m_isAnalog = protoArchSignal.isanalog();

	m_lastState.flags.valid = 0;

	int queueSize = QUEUE_MIN_SIZE;

	if (m_isAnalog == true)
	{
		queueSize = QUEUE_MIN_SIZE * 16;
	}

	m_queue = new FastQueue<ArchFileRecord>(queueSize);
}

ArchFile::~ArchFile()
{
}

void ArchFile::setArchFullPath(const QString& archFullPath)
{
	m_path = QString("%1/%2/%3").
					arg(archFullPath).
					arg(QString().sprintf("%02X", static_cast<int>(m_hash & 0xFF))).
					arg(m_appSignalID.remove(QRegExp("[^0-9A-Za-z_]")));

	m_writablePartition.init(m_path, true);
}

bool ArchFile::pushState(qint64 archID, const SimpleAppSignalState& state)
{
	TEST_PTR_RETURN_FALSE(m_queue);

	Q_UNUSED(archID)

	m_lastState = state;

	ArchFileRecord s;

//	s.state.archID = archID;
	s.state.plantTime = state.time.plant.timeStamp;
	s.state.systemTime = state.time.system.timeStamp;
	s.state.flags = state.flags;
	s.state.value = state.value;
	s.calcCRC16();

	SimpleMutexLocker locker(&m_flushMutex);

	Q_UNUSED(locker);

	m_queue->push(s);

	return true;
}

bool ArchFile::flush(qint64 curPartition, qint64* totalFushedStatesCount, bool flushAnyway)
{
	TEST_PTR_RETURN_FALSE(totalFushedStatesCount);

	SimpleMutexLocker locker(&m_flushMutex);

	Q_UNUSED(locker);

	TEST_PTR_RETURN_FALSE(m_queue);

	if (m_queue->isEmpty() == true)
	{
		setRequiredImmediatelyFlushing(false);
		return false;
	}

	if (m_requiredImmediatelyFlushing.load() == false && flushAnyway == false && m_queue->size() < 3 /* may be 4 or more? */)
	{
		return false;
	}

	int copiedItemsCount = 0;

	bool result = m_queue->copyToBuffer(m_buffer, QUEUE_MAX_SIZE, &copiedItemsCount);

	if (result == false || copiedItemsCount == 0)
	{
		setRequiredImmediatelyFlushing(false);
		return false;
	}

	m_writablePartition.write(curPartition, m_buffer, copiedItemsCount, totalFushedStatesCount);

	setRequiredImmediatelyFlushing(false);

	return true;
}

bool ArchFile::isEmergency() const
{
	TEST_PTR_RETURN_FALSE(m_queue);

	return m_queue->size() >= static_cast<int>(m_queue->queueSize() * QUEUE_EMERGENCY_LIMIT);
}

void ArchFile::shutdown(qint64 curPartition, qint64* totalFlushedStatesCount)
{
	flush(curPartition, totalFlushedStatesCount, true);
}
