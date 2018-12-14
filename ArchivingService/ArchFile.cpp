#include "ArchFile.h"

#include "FileArchWriter.h"
#include "BinSearch.h"

// -----------------------------------------------------------------------------------------------------------------------
//
// ArchFile::Record struct implementation
//
// -----------------------------------------------------------------------------------------------------------------------

bool ArchFile::Record::isValid() const
{
	return calcCrc16(this, sizeof(ArchFile::Record)) == 0;
}

bool ArchFile::Record::timeLessThen(E::TimeType timeType, qint64 time)
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

bool ArchFile::Record::timeLessOrEqualThen(E::TimeType timeType, qint64 time)
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

bool ArchFile::Record::timeGreateThen(E::TimeType timeType, qint64 time)
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

bool ArchFile::Record::timeGreateOrEqualThen(E::TimeType timeType, qint64 time)
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

qint64 ArchFile::Record::getTime(E::TimeType timeType)
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
// ArchFile::Partition class implementation
//
// -----------------------------------------------------------------------------------------------------------------------

ArchFile::Partition::Partition(const ArchFile& archFile, bool writable) :
	m_archFile(archFile),
	m_isWritable(writable)
{
}

ArchFile::Partition::~Partition()
{
	closeFile();
}

qint64 ArchFile::Partition::recordsCount()
{
	if (m_size < 0)
	{
		assert(false);
		return 0;
	}

	return m_size / sizeof(ArchFile::Record);
}

bool ArchFile::Partition::write(qint64 partition, Record* buffer, int statesCount, qint64* totalFushedStatesCount)
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

			m_pathIsExists = d.mkpath(m_archFile.path());
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

			if ((m_size % sizeof(Record)) != 0)
			{
				m_size = (m_size / sizeof(Record)) * sizeof(Record);

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

	qint64 sizeToWrite = statesCount * sizeof(Record);

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

bool ArchFile::Partition::openForReading(qint64 partitionSystemTime)
{
	closeFile();

	QString fileName = getFileName(partitionSystemTime);

	m_file.setFileName(fileName);

	bool result = m_file.open(QIODevice::ReadOnly);

	if (result == false)
	{
		return false;
	}

	QFileInfo fi(m_file);

	m_size = (fi.size() / sizeof(Record)) * sizeof(Record);

	return result;
}

bool ArchFile::Partition::getFirstAndLastRecords(Record* first, Record* last, bool* noRecords)
{
	TEST_PTR_RETURN_FALSE(first);
	TEST_PTR_RETURN_FALSE(last);
	TEST_PTR_RETURN_FALSE(noRecords);

	if (m_file.isOpen() == false)
	{
		assert(false);
		return false;
	}

	if (m_size < sizeof(Record))
	{
		*noRecords = true;
		return true;
	}

	bool result = true;

	result &= readRecord(FIRST_RECORD, first);
	result &= readRecord(LAST_RECORD, last);

	if (result == false)
	{
		return false;
	}

	return true;
}

bool ArchFile::Partition::readRecord(qint64 recordIndex, Record* record)
{
	TEST_PTR_RETURN_FALSE(record);

	if (recordIndex == LAST_RECORD)
	{
		recordIndex = m_size / sizeof(Record) - 1;
	}

	if (recordIndex < 0)
	{
		recordIndex = 0;
	}

	m_file.seek(recordIndex * sizeof(Record));

	qint64 read =  m_file.read(reinterpret_cast<char*>(record), sizeof(Record));

	if (read != sizeof(Record))
	{
		return false;
	}

	if (record->isValid() == false)
	{
		return false;
	}

	return true;
}

ArchFile::FindResult ArchFile::Partition::findStartPosition(E::TimeType timeType, qint64 startTime, qint64 endTime)
{
	Record firstRecord;
	Record lastRecord;
	bool noRecords = false;

	bool res = getFirstAndLastRecords(&firstRecord, &lastRecord, &noRecords);

	if (res == false)
	{
		return FindResult::ReadError;
	}

	if (noRecords == true)
	{
		return FindResult::NotFound;
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
		return FindResult::NotFound;
	}

	// case 2)

	if (firstRecord.timeGreateThen(timeType, startTime) == true &&
		firstRecord.timeLessOrEqualThen(timeType, endTime) == true)
	{
		moveToRecord(0);
		return FindResult::Found;
	}

	// case 3)

	if (lastRecord.timeLessThen(timeType, startTime) == true)
	{
		return FindResult::NotFound;
	}

	// case 4)

	if (firstRecord.timeLessOrEqualThen(timeType, startTime) == true)
	{
		qint64 startPosition = 0;

		FindResult result = binarySearch(timeType, startTime, &startPosition);

		if (result == FindResult::Found)
		{
			moveToRecord(startPosition);
		}

		return result;
	}

	return FindResult::NotFound;
}

bool ArchFile::Partition::close()
{
	closeFile();

	return true;
}

QString ArchFile::Partition::getFileName(qint64 partitionStartTime)
{
	QDateTime date = QDateTime::fromMSecsSinceEpoch(partitionStartTime, Qt::UTC);

	QString fileName = QString("%1/%2_%3_%4_%5_%6.%7").
							arg(m_archFile.path()).
							arg(date.date().year()).
							arg(QString().sprintf("%02d", date.date().month())).
							arg(QString().sprintf("%02d", date.date().day())).
							arg(QString().sprintf("%02d", date.time().hour())).
							arg(QString().sprintf("%02d", date.time().minute()),
							EXTENSION);

	return fileName;
}

void ArchFile::Partition::moveToRecord(qint64 record)
{
	assert(m_file.isOpen() == true);

	m_file.seek(record * sizeof(ArchFile::Record));
}

ArchFile::FindResult ArchFile::Partition::binarySearch(E::TimeType timeType, qint64 time, qint64* startPosition)
{
	if (startPosition == nullptr)
	{
		assert(false);
		return FindResult::SearchError;
	}

	*startPosition = -1;

	qint64 recordCount  = recordsCount();

	if (recordCount == 0)
	{
		return FindResult::NotFound;
	}

	bool result = true;

	Record leftRecord;
	Record rightRecord;

	result &= readRecord(0, &leftRecord);
	result &= readRecord(recordCount - 1, &rightRecord);

	if (result == false)
	{
		return FindResult::ReadError;
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

				Record requiredRecord;

				result = readRecord(requiredRecordNo, &requiredRecord);

				if (result == false)
				{
					return FindResult::ReadError;
				}

				binSearch.checkNextItem(requiredRecord.getTime(timeType));
			}
			break;

		case BinSearchResult::Found:
			*startPosition = binSearch.foundIndex();
			return FindResult::Found;

		case BinSearchResult::NotFound:
			return FindResult::NotFound;

		case BinSearchResult::SearchError:
			return FindResult::SearchError;

		default:
			assert(false);
			break;
		}
	}
	while(1);

	return FindResult::SearchError;
}

void ArchFile::Partition::closeFile()
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
// ArchFile::RequestData class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

ArchFile::RequestData::RequestData(ArchFile& archFile, const ArchRequestParam& param) :
	requestID(param.requestID),
	timeType(param.timeType),
	startTime(param.startTime),
	endTime(param.endTime),
	partitionToRead(archFile, false)
{
}

// ----------------------------------------------------------------------------------------------------------------------
//
// ArchFile class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

ArchFile::Record ArchFile::m_buffer[ArchFile::QUEUE_MAX_SIZE];

const QString ArchFile::EXTENSION = "saf";		// Signal Archive File

ArchFile::ArchFile(const Proto::ArchSignal& protoArchSignal, const QString& archFullPath) :
	m_writablePartition(*this, true)

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

	m_queue = new FastQueue<Record>(queueSize);

	m_path = QString("%1/%2/%3").
					arg(archFullPath).
					arg(QString().sprintf("%02X", static_cast<int>(m_hash & 0xFF))).
					arg(m_appSignalID.remove(QRegExp("[^0-9A-Za-z_]")));
}

ArchFile::~ArchFile()
{
}

bool ArchFile::pushState(qint64 archID, const SimpleAppSignalState& state)
{
	TEST_PTR_RETURN_FALSE(m_queue);

	Q_UNUSED(archID)

	m_lastState = state;

	Record s;

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

	if (m_requiredImmediatelyFlushing == false && flushAnyway == false && m_queue->size() < 2 /* may be 3 or more? */)
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

ArchFile::FindResult ArchFile::findData(const ArchRequestParam& param)
{
	RequestData* rd = m_requestsData.value(param.requestID, nullptr);

	if (rd != nullptr)
	{
		m_requestsData.remove(param.requestID);
		delete rd;
		rd = nullptr;
	}

	rd = new RequestData(*this, param);

	m_requestsData.insert(param.requestID, rd);

	m_findResult = FindResult::NotFound;

	m_findResult = getArchPartitionsInfo(rd);

	if (m_findResult != FindResult::Found)
	{
		cancelRequest(rd->requestID);
		return m_findResult;
	}

	m_findResult = findStartPosition(rd);

	if (m_findResult != FindResult::Found)
	{
		cancelRequest(rd->requestID);
		return m_findResult;
	}

	return m_findResult;
}

void ArchFile::shutdown(qint64 curPartition, qint64* totalFlushedStatesCount)
{
	flush(curPartition, totalFlushedStatesCount, true);
}

ArchFile::FindResult ArchFile::getArchPartitionsInfo(RequestData* rd)
{
	if (rd == nullptr)
	{
		assert(false);
		return FindResult::SearchError;
	}

	rd->partitionsInfo.clear();

	// Arch file name format: 2018_12_31_23_59.saf (year_month_day_hour_minute.saf)

	QRegExp archFileNameTemplate(QString("2[0-9][0-9][0-9]_[0-1][0-9]_[0-3][0-9]_[0-2][0-9]_[0-5][0-9].%1").arg(ArchFile::EXTENSION));

	QDirIterator di(m_path, QDir::Files);

	while(di.hasNext() == true)
	{
		QString nextFilePath = di.next();

		if (nextFilePath.isEmpty() == true)
		{
			break;
		}

		QFileInfo fi = di.fileInfo();

		if (fi.isFile() == false &&
			fi.fileName().contains(archFileNameTemplate) == false)
		{
			continue;
		}

		PartitionInfo pi;

		pi.fileName = fi.fileName();

		int year = pi.fileName.mid(0, 4).toInt();
		int month = pi.fileName.mid(5, 2).toInt();
		int day = pi.fileName.mid(8, 2).toInt();
		int hour = pi.fileName.mid(11, 2).toInt();
		int minute = pi.fileName.mid(14, 2).toInt();

		pi.date = QDateTime(QDate(year, month, day), QTime(hour, minute, 0, 0), Qt::TimeSpec::UTC);
		pi.startTime = pi.date.toMSecsSinceEpoch();

		rd->partitionsInfo.append(pi);
	}

	if (rd->partitionsInfo.count() > 0)
	{
		return FindResult::Found;
	}

	return FindResult::NotFound;
}

ArchFile::FindResult ArchFile::findStartPosition(RequestData* rd)
{
	if (rd == nullptr)
	{
		assert(false);
		return FindResult::SearchError;
	}

	int partitionsCount = rd->partitionsInfo.count();

	if (partitionsCount == 0)
	{
		return FindResult::NotFound;
	}

	// 1) Sort m_archPartitionsInfo by systemTime ascending
	//

	qSort(rd->partitionsInfo);

	// 2) Find LAST partition where systemTime < m_startTime

	int startPartitionIndex = 0;

	for(int i = 1 /* it is Ok */ ; i < partitionsCount; i++)
	{
		const PartitionInfo& pi = rd->partitionsInfo[i];

		if (pi.startTime >= rd->startTime)
		{
			break;
		}

		startPartitionIndex = i;
	}

	// 3)

	rd->partitionToReadIndex = startPartitionIndex;

	while(rd->partitionToReadIndex < rd->partitionsInfo.count());
	{
		bool res = rd->partitionToRead.openForReading(rd->partitionsInfo[rd->partitionToReadIndex].startTime);

		if (res == false)
		{
			return FindResult::ReadError;
		}

		FindResult result = rd->partitionToRead.findStartPosition(rd->timeType, rd->startTime, rd->endTime);

		switch(result)
		{
		case FindResult::Found:
			return FindResult::Found;

		case FindResult::NotFound:

			rd->partitionToReadIndex++;

			if (rd->partitionToReadIndex >= rd->partitionsInfo.count())
			{
				rd->partitionToRead.close();
				rd->partitionToReadIndex = -1;

				return FindResult::NotFound;
			}

			break;

		case FindResult::ReadError:
		case FindResult::SearchError:

			rd->partitionToRead.close();
			rd->partitionToReadIndex = -1;

			return result;

		default:
			assert(false);
		}

	}

	return FindResult::SearchError;
}

void ArchFile::cancelRequest(quint32 requestID)
{
	RequestData* rd = m_requestsData.value(requestID, nullptr);

	if (rd == nullptr)
	{
		assert(false);
		return;
	}

	m_requestsData.remove(requestID);
	delete rd;
}

