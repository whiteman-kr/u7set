#include "ArchFile.h"

#include "ArchWriterThread.h"
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
	m_recordCount = fi.size() / sizeof(Record);

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

bool ArchFile::Partition::readRecord(qint64 recordIndex, Record* record)
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

bool ArchFile::Partition::read(Record* recordBuffer, int maxRecordsToRead, int* readCount)
{
	TEST_PTR_RETURN_FALSE(recordBuffer);
	TEST_PTR_RETURN_FALSE(readCount);

	// HERE:
	//	do read to intermediate buffer
	//	and do records consistency checking
	//	align to record and return valid records

	qint64 readSize =  m_file.read(reinterpret_cast<char*>(recordBuffer), sizeof(Record) * maxRecordsToRead);

	*readCount = readSize / sizeof(Record);

	return true;
}

ArchFindResult ArchFile::Partition::findStartPosition(RequestData* rd)
{
	if (rd == nullptr)
	{
		assert(false);
		return ArchFindResult::SearchError;
	}

	Record firstRecord;
	Record lastRecord;
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

	E::TimeType timeType = rd->timeType;
	qint64 startTime = rd->startTime;
	qint64 endTime = rd->endTime;

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
		rd->startRecord = 0;
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
		qint64 startPosition = 0;

		ArchFindResult result = binarySearch(timeType, startTime, &startPosition);

		if (result == ArchFindResult::Found)
		{
			rd->startRecord = startPosition;
			moveToRecord(startPosition);
		}

		return result;
	}

	return ArchFindResult::NotFound;
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

ArchFindResult ArchFile::Partition::binarySearch(E::TimeType timeType, qint64 time, qint64* startPosition)
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

	Record leftRecord;
	Record rightRecord;

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

				Record requiredRecord;

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
	requestID(param.requestID()),
	timeType(param.timeType()),
	startTime(param.startTime()),
	endTime(param.endTime()),
	partitionToRead(archFile, false)
{
}

ArchFile::PartitionInfo ArchFile::RequestData::partitionToReadInfo()
{
	if (partitionToReadIndex >= 0 && partitionToReadIndex < partitionsInfo.count())
	{
		return partitionsInfo[partitionToReadIndex];
	}

	assert(false);

	return PartitionInfo();
}

bool ArchFile::RequestData::getRecord(ArchFile::Record* record)
{
	TEST_PTR_RETURN_FALSE(record);

	if (noMoreData == true)
	{
		return false;
	}

	if (recordsInBuffer == 0 || nextRecordIndex >= recordsInBuffer)
	{
		fillBuffer();

		if (recordsInBuffer == 0)
		{
			sdflmsl;dfmslkdfm slakdmf lsakdmf lsakdfm lskmf
			return false;
		}
	}

	*record = records[nextRecordIndex];

	return true;
}

bool ArchFile::RequestData::gotoNextRecord()
{
	nextRecordIndex++;
}

void ArchFile::RequestData::fillBuffer()
{
	int readCount = 0;

	int maxRecordsToRead = RECORDS_BUFFER_SIZE;
	recordsInBuffer = 0;
	nextRecordIndex = 0;

	do
	{
		partitionToRead.read(records + recordsInBuffer, maxRecordsToRead, &readCount);

		recordsInBuffer += readCount;
		maxRecordsToRead -= readCount;

		if (readCount < RECORDS_BUFFER_SIZE)
		{
			partitionToReadIndex++;

			if (partitionToReadIndex >= partitionsInfo.count())
			{
				noMoreData = true;
				return;
			}

			bool res = partitionToRead.openForReading(partitionsInfo[partitionToReadIndex].startTime);

			if (res == false)
			{
				noMoreData = true;
				return;
			}
		}
	}
	while(recordsInBuffer < RECORDS_BUFFER_SIZE);
}


// ----------------------------------------------------------------------------------------------------------------------
//
// ArchFile class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

ArchFile::Record ArchFile::m_buffer[ArchFile::QUEUE_MAX_SIZE];

const QString ArchFile::EXTENSION = "saf";		// Signal Archive File

ArchFile::ArchFile(const Proto::ArchSignal& protoArchSignal) :
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

ArchFindResult ArchFile::findData(const ArchRequestParam& param)
{
	RequestData* rd = createRequestData(param);

	getArchPartitionsInfo(rd);

	if (rd->findResult != ArchFindResult::Found)
	{
		finalizeRequest(rd->requestID);
		return rd->findResult;
	}

	findStartPosition(rd);

	if (rd->findResult == ArchFindResult::Found)
	{
		qDebug() << C_STR(QString("ArchRequest ID=%1, signal %2 data found, partition %3, record=%4").
								arg(param.requestID()).
								arg(m_appSignalID).
								arg(rd->partitionToReadInfo().fileName).
								arg(rd->startRecord));
	}
	else
	{
		qDebug() << C_STR(QString("ArchRequest ID=%1, signal %2 data NOT found").
								arg(param.requestID()).
								arg(m_appSignalID));
	}

	return rd->findResult;
}

void ArchFile::finalizeRequest(quint32 requestID)
{
	clearRequestData(requestID);
}

void ArchFile::shutdown(qint64 curPartition, qint64* totalFlushedStatesCount)
{
	flush(curPartition, totalFlushedStatesCount, true);
}

void ArchFile::getArchPartitionsInfo(RequestData* rd)
{
	if (rd == nullptr)
	{
		assert(false);
		rd->findResult = ArchFindResult::SearchError;
		return;
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
		rd->findResult = ArchFindResult::Found;
	}
	else
	{
		rd->findResult = ArchFindResult::NotFound;
	}

	return;
}

void ArchFile::findStartPosition(RequestData* rd)
{
	if (rd == nullptr)
	{
		assert(false);
		return;
	}

	int partitionsCount = rd->partitionsInfo.count();

	if (partitionsCount == 0)
	{
		rd->findResult = ArchFindResult::NotFound;
		return;
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

	while(rd->partitionToReadIndex < partitionsCount)
	{
		bool res = rd->partitionToRead.openForReading(rd->partitionsInfo[rd->partitionToReadIndex].startTime);

		if (res == false)
		{
			rd->findResult = ArchFindResult::SearchError;
			return;
		}

		ArchFindResult result = rd->partitionToRead.findStartPosition(rd);

		switch(result)
		{
		case ArchFindResult::Found:
			rd->findResult = ArchFindResult::Found;
			return;

		case ArchFindResult::NotFound:
			rd->partitionToReadIndex++;
			break;

		case ArchFindResult::SearchError:

			rd->partitionToRead.close();
			rd->partitionToReadIndex = -1;

			rd->findResult = ArchFindResult::NotFound;
			return;

		default:
			assert(false);
		}
	}

	rd->partitionToRead.close();
	rd->partitionToReadIndex = -1;

	rd->findResult = ArchFindResult::NotFound;
}

ArchFile::RequestData* ArchFile::createRequestData(const ArchRequestParam& param)
{
	m_requestsDataMutex.lock();

	RequestData* rd = m_requestsData.value(param.requestID(), nullptr);

	if (rd != nullptr)
	{
		assert(false);
		m_requestsData.remove(param.requestID());
		delete rd;
		rd = nullptr;
	}

	rd = new RequestData(*this, param);

	m_requestsData.insert(param.requestID(), rd);

	m_requestsDataMutex.unlock();

	return rd;
}

void ArchFile::clearRequestData(quint32 requestID)
{
	m_requestsDataMutex.lock();

	RequestData* rd = m_requestsData.value(requestID, nullptr);

	if (rd == nullptr)
	{
		assert(false);
		return;
	}

	m_requestsData.remove(requestID);
	delete rd;

	m_requestsDataMutex.unlock();
}
