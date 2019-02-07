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
/*	case E::TimeType::Local:
		return state.localTime < time; */

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
/*	case E::TimeType::Local:
		return state.localTime <= time;*/

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
/*	case E::TimeType::Local:
		return state.localTime > time;*/

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
/*	case E::TimeType::Local:
		return state.localTime >= time;*/

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
	case E::TimeType::Local:
		return state.localTime;

	case E::TimeType::System:
		return state.systemTime;

	case E::TimeType::Plant:
		return state.plantTime;

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
	closeFile();

	QString fileName = getFileName(partitionSystemTime, shortTerm);

	m_file.setFileName(fileName);

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

	*readCount = readSize / sizeof(ArchFileRecord);

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
	QDateTime date = QDateTime::fromMSecsSinceEpoch(partitionStartTime, Qt::UTC);

	QString extension;

	if (shortTerm == true)
	{
		extension = ArchFile::SHORT_TERM_ARCHIVE_EXTENSION;
	}
	else
	{
		extension = ArchFile::LONG_TERM_ARCHIVE_EXTENSION;
	}

	QString fileName = QString("%1/%2_%3_%4_%5_%6.%7").
							arg(m_archFilePath).
							arg(date.date().year()).
							arg(QString().sprintf("%02d", date.date().month())).
							arg(QString().sprintf("%02d", date.date().day())).
							arg(QString().sprintf("%02d", date.time().hour())).
							arg(QString().sprintf("%02d", date.time().minute())).
							arg(extension);

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

const QString ArchFile::LONG_TERM_ARCHIVE_EXTENSION = "lta";		// Signal Archive File
const QString ArchFile::SHORT_TERM_ARCHIVE_EXTENSION = "sta";		// Signal Archive File

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

	s.state.localTime = state.time.local.timeStamp;
	s.state.systemTime = state.time.system.timeStamp;
	s.state.plantTime = state.time.plant.timeStamp;
	//	s.state.archID = archID;
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

QVector<ArchFilePartition::Info> ArchFile::getArchPartitionsInfo(const QString& path)
{
	QVector<ArchFilePartition::Info> partitionsInfo;

	// Arch file name format: 2018_12_31_23_59.sta (or lta))

	QRegExp archFileNameTemplate(QString("2[0-9][0-9][0-9]_[0-1][0-9]_[0-3][0-9]_[0-2][0-9]_[0-5][0-9]."));

	QDirIterator di(path, QDir::Files);

	while(di.hasNext() == true)
	{
		QString nextFilePath = di.next();

		if (nextFilePath.isEmpty() == true)
		{
			break;
		}

		QFileInfo fi = di.fileInfo();

		QString fileName = fi.fileName();

		if (fi.isFile() == false &&
			fileName.contains(archFileNameTemplate) == false)
		{
			continue;
		}

		ArchFilePartition::Info pi;

		if (fileName.endsWith(ArchFile::LONG_TERM_ARCHIVE_EXTENSION) == true)
		{
			pi.shortTerm = false;
		}
		else
		{
			if (fileName.endsWith(ArchFile::SHORT_TERM_ARCHIVE_EXTENSION) == true)
			{
				pi.shortTerm = true;
			}
			else
			{
				continue;		// unknown extension
			}
		}

		pi.fileName = fileName;

		int year = pi.fileName.mid(0, 4).toInt();
		int month = pi.fileName.mid(5, 2).toInt();
		int day = pi.fileName.mid(8, 2).toInt();
		int hour = pi.fileName.mid(11, 2).toInt();
		int minute = pi.fileName.mid(14, 2).toInt();

		pi.date = QDateTime(QDate(year, month, day), QTime(hour, minute, 0, 0), Qt::TimeSpec::UTC);
		pi.startTime = pi.date.toMSecsSinceEpoch();

		partitionsInfo.append(pi);
	}

	// Sort m_archPartitionsInfo by systemTime ascending
	//
	qSort(partitionsInfo);

	for(int i = 0; i < partitionsInfo.count(); i++)
	{
		partitionsInfo[i].index = i;
	}

	return partitionsInfo;
}

void ArchFile::shutdown(qint64 curPartition, qint64* totalFlushedStatesCount)
{
	flush(curPartition, totalFlushedStatesCount, true);
}

/*
void ArchFile::breakMaintenance()
{
	m_fileInMaintenanceMutex.lock();

	if (m_fileInMaintenance == true)
	{
		m_breakMaintenanceRequest = true;

		m_fileInMaintenanceMutex.unlock();

		while(m_breakMaintenanceRequest != false)
	}
}

*/

bool ArchFile::maintenance(qint64 currentPartition,
						   qint64 shortTermPeriodMs,
						   qint64 msLongTermPeriod,
						   int* deletedCount,
						   int* packedCount)
{
	QVector<ArchFilePartition::Info> partitionsInfo = getArchPartitionsInfo(m_path);

/*	if (partitionsInfo.count() > 0 && m_isAnalog == false)
	{
		DEBUG_STOP;
	}*/

	bool result = packPartitions(partitionsInfo, currentPartition, shortTermPeriodMs, packedCount);

	if (result == false)
	{
		return false;
	}

	result = deleteOldPartitions(partitionsInfo, currentPartition, msLongTermPeriod, deletedCount);

	return result;
}

void ArchFile::startMaintenance()
{
	AUTO_LOCK(m_fileInMaintenanceMutex);

	m_fileInMaintenance = true;
}

void ArchFile::stopMaintenance()
{
	AUTO_LOCK(m_fileInMaintenanceMutex);

	m_fileInMaintenance = false;
}

bool ArchFile::packPartitions(const QVector<ArchFilePartition::Info>& partitionsInfo,
								qint64 currentPartition,
								qint64 msShortTermPeriod,
								int* packedCount)
{
	TEST_PTR_RETURN_FALSE(packedCount);

	// returns false if maintenance has been breaked!

	*packedCount = 0;

	for(int i = 0; i < partitionsInfo.count(); i++)
	{
		if (isRwAccessRequested() == true)
		{
			return false;			// break maintenance
		}

		const ArchFilePartition::Info& pi = partitionsInfo[i];

		if (pi.shortTerm == false)
		{
			continue;				// partition already packed
		}

		if (currentPartition - pi.startTime > msShortTermPeriod)
		{
			bool result = true;

			if (m_isAnalog == true)
			{
				result = packAnalogSignalPartition(pi);
			}
			else
			{
				result = packDiscreteSignalPartition(pi);
			}

			if (result == false)
			{
				return false;
			}

			(*packedCount)++;
		}
		else
		{
			break;		// partitions info sorted by pi.startTime ascending
						// so no more partitions to packing
		}
	}

	return true;
}

bool ArchFile::packAnalogSignalPartition(const ArchFilePartition::Info& pi)
{
	assert(pi.shortTerm == true);

	const int ARCH_FILE_RECORD_SIZE = sizeof(ArchFileRecord);

	// opening short term archive partition *.sta
	//
	QString staFileName = getPartitionFileName(pi);

	QFile staFile(staFileName);

	if (staFile.open(QIODevice::ReadOnly) == false)
	{
		return false;
	}

	QFileInfo fi(staFile);

	qint64 staFileSize = fi.size();

	staFileSize = (staFileSize / ARCH_FILE_RECORD_SIZE) * ARCH_FILE_RECORD_SIZE;

	if (staFileSize < ARCH_FILE_RECORD_SIZE)
	{
		staFile.close();
		QDir().remove(staFileName);
		return true;
	}

	// creating long term archive partition *.lta
	//
	QString ltaFileName = staFileName;

	ltaFileName.replace(SHORT_TERM_ARCHIVE_EXTENSION, LONG_TERM_ARCHIVE_EXTENSION);

	QFile ltaFile(ltaFileName);

	if (ltaFile.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
	{
		return false;
	}

	// reading buffer allocation
	//
	qint64 bufSize = staFileSize;

	const int MAX_BUF_SIZE = ((10 * 1024 * 1024) / ARCH_FILE_RECORD_SIZE) * ARCH_FILE_RECORD_SIZE;		// ~10 Mb

	if (bufSize > MAX_BUF_SIZE)
	{
		bufSize = MAX_BUF_SIZE;
	}

	char* readBuf = new char[bufSize];
	char* writeBuf = new char[bufSize];

	// sta to lta processing
	//
	int inReadBufSize = 0;
	int inWriteBufSize = 0;

	bool result = true;

	while(true)
	{
		if (isRwAccessRequested() == true)
		{
			result = false;
			break;
		}

		qint64 reads = staFile.read(readBuf + inReadBufSize, bufSize - inReadBufSize);

		if (reads <= 0)
		{
			break;
		}

		inReadBufSize += reads;

		if (inReadBufSize < ARCH_FILE_RECORD_SIZE)
		{
			break;
		}

		int recordStartPos = 0;

		while(inReadBufSize - recordStartPos >= ARCH_FILE_RECORD_SIZE)
		{
			const ArchFileRecord* record = reinterpret_cast<const ArchFileRecord*>(readBuf + recordStartPos);

			if (record->isValid() == true)
			{
				if (record->state.flags.hasShortTermArchivingReasonOnly() == true)
				{
					// skip record
					//
				}
				else
				{
					// copy record in writeBuffer
					//
					memcpy(writeBuf + inWriteBufSize, reinterpret_cast<const char*>(record), ARCH_FILE_RECORD_SIZE);

					inWriteBufSize += ARCH_FILE_RECORD_SIZE;

					if (inWriteBufSize >= bufSize)
					{
						qint64 written = ltaFile.write(writeBuf, inWriteBufSize);

						if (written != inWriteBufSize)
						{
							result = false;
							break;
						}

						inWriteBufSize = 0;
					}
				}

				recordStartPos += ARCH_FILE_RECORD_SIZE;
			}
			else
			{
				// record is not valid, shift on one byte
				//
				recordStartPos++;
			}
		}

		inReadBufSize -= recordStartPos;

		if (inReadBufSize > 0)
		{
			// copy remaining bytes in beginning of buffer
			//
			memcpy(readBuf + 0, readBuf + recordStartPos, inReadBufSize);
		}
	}

	if (result == true && inWriteBufSize > 0)
	{
		qint64 written = ltaFile.write(writeBuf, inWriteBufSize);

		if (written != inWriteBufSize)
		{
			result = false;
		}
	}

	ltaFile.close();
	staFile.close();

	if (result == true)
	{
		QDir().remove(staFileName);
	}
	else
	{
		QDir().remove(ltaFileName);
	}

	delete [] readBuf;
	delete [] writeBuf;

	return result;
}

bool ArchFile::packDiscreteSignalPartition(const ArchFilePartition::Info& pi)
{
	assert(pi.shortTerm == true);

	QString fileName = getPartitionFileName(pi);

	QString newFileName = fileName;

	newFileName.replace(SHORT_TERM_ARCHIVE_EXTENSION, LONG_TERM_ARCHIVE_EXTENSION);

	QDir dir;

	dir.rename(fileName, newFileName);

	return true;
}

bool ArchFile::deleteOldPartitions(const QVector<ArchFilePartition::Info>& partitionsInfo,
								  qint64 currentPartition,
								  qint64 msLongTermPeriod,
								  int* deletedCount)
{
	TEST_PTR_RETURN_FALSE(deletedCount);

	// returns false if maintenance has been breaked!

	*deletedCount = 0;

	for(int i = 0; i < partitionsInfo.count(); i++)
	{
		if (isRwAccessRequested() == true)
		{
			return false;		// break maintenance
		}

		const ArchFilePartition::Info& pi = partitionsInfo[i];

		if (currentPartition - pi.startTime > msLongTermPeriod)
		{
			QDir dir;

			QString fileName = getPartitionFileName(pi);

			if (dir.remove(fileName) == true)
			{
				(*deletedCount)++;
			}
		}
		else
		{
			break;				// partitions info sorted by pi.startTime ascending
								// so no more partitions to delete
		}
	}

	return true;
}

QString ArchFile::getPartitionFileName(const ArchFilePartition::Info& pi)
{
	return QString("%1/%2").arg(m_path, pi.fileName);
}



