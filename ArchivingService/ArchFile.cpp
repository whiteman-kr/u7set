#include "../lib/ConstStrings.h"

#include "ArchFile.h"
#include "ArchWriterThread.h"
#include "BinSearch.h"
#include "ArchRequest.h"

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

	if (record->isNotCorrupted() == false)
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
	QDateTime date = QDateTime::fromMSecsSinceEpoch(partitionStartTime, Qt::UTC);

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


inline bool operator < (const ArchFilePartition::Info& p1, const ArchFilePartition::Info& p2)
{
	if (p1.startTime == p2.startTime)
	{
		return p1.shortTerm;			// if start times is equal short term partitions consider less then long term
	}

	return p1.startTime < p2.startTime;
}


// ----------------------------------------------------------------------------------------------------------------------
//
// ArchFile class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

const int ArchFile::QUEUE_MIN_SIZE = 10;
const int ArchFile::QUEUE_MAX_SIZE = 1280;	// == QUEUE_MIN_SIZE * 128 (2^7)

const double ArchFile::QUEUE_EMERGENCY_LIMIT = 0.7;		// 70%
const double ArchFile::QUEUE_EXPAND_LIMIT = 0.8;		// 80%
const double ArchFile::QUEUE_REDUCTION_LIMIT = 0.2;		// 20%


ArchFile::ArchFile(const Proto::ArchSignal& protoArchSignal, CircularLoggerShared log) :
	m_log(log),
	m_queue(protoArchSignal.isanalog() == true ? QUEUE_MIN_SIZE * 16 : QUEUE_MIN_SIZE)
{
	m_hash = protoArchSignal.hash();
	m_appSignalID = QString::fromStdString(protoArchSignal.appsignalid());
	m_isAnalog = protoArchSignal.isanalog();

	m_lastRecord.state.flags.valid = 0;
}

ArchFile::~ArchFile()
{
}

void ArchFile::setArchFullPath(const QString& archFullPath)
{
	QString lastByteOfHash = (QString("%1").arg(static_cast<int>(m_hash & 0xFF), 2, 16, Latin1Char::ZERO)).toUpper();

	m_path = QString("%1/%2/%3").
					arg(archFullPath).
					arg(lastByteOfHash).
					arg(m_appSignalID.remove(QRegExp("[^0-9A-Za-z_]")));

	m_writablePartition.init(m_path, true);
}

bool ArchFile::pushState(const SimpleAppSignalState& state)
{
	const QThread* thread = QThread::currentThread();

	controlQueueSizeBeforePush(thread);

	ArchFileRecord s;

	s.state.localTime = state.time.local.timeStamp;
	s.state.systemTime = state.time.system.timeStamp;
	s.state.plantTime = state.time.plant.timeStamp;
	s.state.packetNo = state.packetNo;
	s.state.flags = state.flags;
	s.state.value = state.value;
	s.calcCRC16();

	if (m_lastRecordInitialized == false && s.state.flags.valid)
	{
		ArchFileRecord nvp = s;

		nvp.state.value = 0;
		nvp.state.flags.valid = 0;
		nvp.state.flags.autoPoint = 1;
		nvp.state.packetNo = 0;
		nvp.offsetTimes(-1);
		nvp.calcCRC16();

		m_queue.push(nvp, thread);
	}

	m_queue.push(s, thread);

	m_lastRecord = s;
	m_lastRecordInitialized = true;

	return true;
}

bool ArchFile::flush(qint64 curPartition,
					 qint64* totalFushedStatesCount,
					 bool flushAnyway,
					 int minQueueSizeForFlushing,
					 ArchFileRecord* buffer,
					 int bufferSize,
					 const QThread* thread)
{
	TEST_PTR_RETURN_FALSE(totalFushedStatesCount);
	TEST_PTR_RETURN_FALSE(buffer);

	int queueSize = m_queue.size(thread);

	if (queueSize == 0)
	{
		setRequiredImmediatelyFlushing(false);
		return false;
	}

	if (m_requiredImmediatelyFlushing.load() == false && flushAnyway == false &&  queueSize < minQueueSizeForFlushing)
	{
		return false;
	}

	int copiedItemsCount = 0;

	bool result = m_queue.copyToBuffer(buffer, bufferSize, &copiedItemsCount, thread);

	if (result == false || copiedItemsCount == 0)
	{
		setRequiredImmediatelyFlushing(false);
		return false;
	}

	m_writablePartition.write(curPartition, buffer, copiedItemsCount, totalFushedStatesCount);

	setRequiredImmediatelyFlushing(false);

	return true;
}

bool ArchFile::isEmergency() const
{
	const QThread* thread = QThread::currentThread();

	return m_queue.size(thread) >= static_cast<int>(m_queue.queueSize(thread) * QUEUE_EMERGENCY_LIMIT);
}

QVector<ArchFilePartition::Info> ArchFile::getArchPartitionsInfo(const QString& path)
{
	QVector<ArchFilePartition::Info> partitionsInfo;

	// Arch file name format: 2018_12_31_23_59.sta (or lta))

	QRegExp archFileNameTemplate(ARCH_FILE_NAME_TEMPLATE);

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

		if (fileName.endsWith(LONG_TERM_ARCHIVE_EXTENSION) == true)
		{
			pi.shortTerm = false;
		}
		else
		{
			if (fileName.endsWith(SHORT_TERM_ARCHIVE_EXTENSION) == true)
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
	std::sort(partitionsInfo.begin(), partitionsInfo.end());

	//
	for(int i = 0; i < partitionsInfo.count() - 1; /* it is Ok*/)
	{
		const ArchFilePartition::Info& p1 = partitionsInfo[i];
		const ArchFilePartition::Info& p2 = partitionsInfo[i + 1];

		if (p1.startTime == p2.startTime)
		{
			// Two partitions with same startTime is the ANOMALY.
			// Short term partition will taken as more precise.
			//

			assert(p1.shortTerm == true);

			// After sorting partitionsInfo short term partition is first.
			// Remove long term partition (that is second).
			// On next archive maintenance long term partition will be recreated from short term partition.
			//

			QDir().remove(getPartitionFileName(path, p2));

			partitionsInfo.removeAt(i + 1);

			continue;
		}

		i++;
	}

	// remove partitions with same startTime.
	// leave shortTerm partitions

	for(int i = 0; i < partitionsInfo.count(); i++)
	{
		partitionsInfo[i].index = i;
	}

	return partitionsInfo;
}

QString ArchFile::getPartitionFileName(const QString& archFilePath, const ArchFilePartition::Info& pi)
{
	return QString("%1/%2").arg(archFilePath, pi.fileName);
}

void ArchFile::shutdown(qint64 curPartition,
						qint64* totalFlushedStatesCount,
						ArchFileRecord* buffer,
						int bufferSize,
						const QThread* thread)
{
	TEST_PTR_RETURN(totalFlushedStatesCount);
	TEST_PTR_RETURN(buffer);

	if (m_lastRecordInitialized == true && m_lastRecord.state.flags.valid == 1)
	{
		qint64 curSysTime = QDateTime::currentMSecsSinceEpoch();

		qint64 dT = curSysTime - m_lastRecord.state.systemTime;

		if (dT <= 0)
		{
			dT = 1;
		}

		m_lastRecord.state.value = 0;
		m_lastRecord.state.flags.valid = 0;
		m_lastRecord.state.flags.autoPoint = 1;
		m_lastRecord.state.packetNo = 0;
		m_lastRecord.offsetTimes(dT);
		m_lastRecord.calcCRC16();

		m_queue.push(m_lastRecord, thread);
	}

	flush(curPartition,
		  totalFlushedStatesCount,
		  true,
		  Archive::DEFAULT_QUEUE_SIZE_FOR_FLUSHING /* value doesn't matter, because prev param says FlushAnyway */,
		  buffer,
		  bufferSize,
		  thread);
}

bool ArchFile::maintenance(qint64 currentPartition,
						   qint64 shortTermPeriodMs,
						   qint64 msLongTermPeriod,
						   int* deletedCount,
						   int* packedCount,
						   const RunOverrideThread* thread)
{
	TEST_PTR_RETURN_FALSE(deletedCount);
	TEST_PTR_RETURN_FALSE(packedCount);
	TEST_PTR_RETURN_FALSE(thread);

	QVector<ArchFilePartition::Info> partitionsInfo = getArchPartitionsInfo(m_path);

	bool result = packPartitions(partitionsInfo, currentPartition, shortTermPeriodMs, packedCount, thread);

	if (result == false)
	{
		return false;
	}

	result = deleteOldPartitions(partitionsInfo, currentPartition, msLongTermPeriod, deletedCount, thread);

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
								int* packedCount,
								const RunOverrideThread* thread)
{
	TEST_PTR_RETURN_FALSE(packedCount);

	// returns false if maintenance has been breaked!

	for(int i = 0; i < partitionsInfo.count(); i++)
	{
		if (thread->isQuitRequested() == true)
		{
			return false;
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
				result = packAnalogSignalPartition(pi, thread);
			}
			else
			{
				result = packDiscreteSignalPartition(pi);
			}

			if (result == false)
			{
				qDebug() << C_STR(QString("Pack error %1").arg(getPartitionFileName(pi)));
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

bool ArchFile::packAnalogSignalPartition(const ArchFilePartition::Info& pi, const RunOverrideThread* thread)
{
	assert(pi.shortTerm == true);

	// opening short term archive partition *.sta
	//
	QString staFileName = getPartitionFileName(pi);

	QFile staFile(staFileName);

	if (staFile.open(QIODevice::ReadOnly) == false)
	{
		DEBUG_LOG_ERR(m_log, QString("Maintenance: file open error %1").arg(staFileName));
		return false;
	}

	QFileInfo fi(staFile);

	qint64 staFileSize = fi.size();

	staFileSize = (staFileSize / ArchFileRecord::SIZE) * ArchFileRecord::SIZE;

	if (staFileSize < ArchFileRecord::SIZE)
	{
		staFile.close();

		DEBUG_LOG_WRN(m_log, QString("Maintenance: file size less then ARCH_FILE_RECORD_SIZE, %1").arg(staFileName));

		bool res = QDir().remove(staFileName);

		if (res == false)
		{
			DEBUG_LOG_ERR(m_log, QString("Maintenance: file deleting error %1").arg(staFileName));
		}

		return res;
	}

	// creating long term archive partition *.lta
	//
	QString ltaFileName = staFileName;

	ltaFileName.replace(SHORT_TERM_ARCHIVE_EXTENSION, LONG_TERM_ARCHIVE_EXTENSION);

	QFile ltaFile(ltaFileName);

	if (ltaFile.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
	{
		DEBUG_LOG_ERR(m_log, QString("Maintenance: file creation error %1").arg(ltaFileName));
		return false;
	}

	const int BUF_SIZE = 2 * 1024 * 1024;	// buf size ~2Mb

	ArchFileReadBuffer readBuf(BUF_SIZE);
	ArchFileWriteBuffer writeBuf(BUF_SIZE);

	// sta to lta processing
	//

	bool result = true;
	ArchFileRecord record;

	while(true)
	{
		if (thread->isQuitRequested() == true)
		{
			result = false;
			break;
		}

		result = readBuf.fillBuffer(staFile);

		if (result == false)
		{
			break;
		}

		if (readBuf.hasRecordsInBuffer() == false)
		{
			break;
		}

		do
		{
			bool okRecord = readBuf.getRecordAndMoveToNext(&record);

			if (okRecord == false)
			{
				break;
			}

			if (record.hasShortTermArchivingReasonOnly() == true)
			{
				// skip record
				//
			}
			else
			{
				// copy record in writeBuffer
				//
				result = writeBuf.writeNextRecord(ltaFile, record);

				if (result == false)
				{
					break;
				}
			}
		}
		while(true);

		if (result == false)
		{
			break;
		}
	}

	if (result == true)
	{
		result = writeBuf.flushBuffer(ltaFile);
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

	return result;
}

bool ArchFile::writeLtaFile(QFile& ltaFile, const char* buffer, int size)
{
	bool result = true;

	qint64 written = ltaFile.write(buffer, size);

	if (written != size)
	{
		DEBUG_LOG_ERR(m_log, QString("Maintenance: lta-file writing error %1").arg(ltaFile.fileName()));
		result = false;
	}

	return result;
}

bool ArchFile::packDiscreteSignalPartition(const ArchFilePartition::Info& pi)
{
	assert(pi.shortTerm == true);

	QString staFileName = getPartitionFileName(pi);

	QString ltaFileName = staFileName;

	ltaFileName.replace(SHORT_TERM_ARCHIVE_EXTENSION, LONG_TERM_ARCHIVE_EXTENSION);

	QDir dir;

	bool result = dir.rename(staFileName, ltaFileName);

	if (result == false)
	{
		DEBUG_LOG_ERR(m_log, QString("Maintenance: file renaming error %1 to %2").arg(staFileName).arg(ltaFileName));
	}

	return result;
}

bool ArchFile::deleteOldPartitions(const QVector<ArchFilePartition::Info>& partitionsInfo,
								  qint64 currentPartition,
								  qint64 msLongTermPeriod,
								  int* deletedCount,
								  const RunOverrideThread* thread)
{
	TEST_PTR_RETURN_FALSE(deletedCount);

	// returns false if maintenance has been breaked!

	bool result = true;

	for(int i = 0; i < partitionsInfo.count(); i++)
	{
		if (thread->isQuitRequested() == true)
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
			else
			{
				DEBUG_LOG_ERR(m_log, QString("Maintenance: file deleting error %1").arg(fileName));
				result = false;
			}
		}
		else
		{
			break;				// partitions info sorted by pi.startTime ascending
								// so no more partitions to delete
		}
	}

	return result;
}

QString ArchFile::getPartitionFileName(const ArchFilePartition::Info& pi)
{
	return QString("%1/%2").arg(m_path, pi.fileName);
}

void ArchFile::controlQueueSizeBeforePush(const QThread* thread)
{
	int curSize = 0;
	int curMaxSize = 0;
	int queueSize = 0;

	m_queue.getSizes(&curSize, &curMaxSize, &queueSize, thread);

	if (curSize >= static_cast<int>(queueSize * QUEUE_EXPAND_LIMIT) && queueSize < QUEUE_MAX_SIZE)
	{
		int k = (queueSize / QUEUE_MIN_SIZE) * 2;

		if (k == 0)
		{
			k = 1;
		}

		m_queue.nonDestructiveResize(QUEUE_MIN_SIZE * k, thread);

		m_statesCountAfterExpand = 0;

		DEBUG_LOG_MSG(m_log, QString("%1 queue expand to %2").arg(m_appSignalID).arg(QUEUE_MIN_SIZE * k));
	}
	else
	{
		if (m_statesCountAfterExpand >= 0)
		{
			m_statesCountAfterExpand++;

			if (m_statesCountAfterExpand >= queueSize)
			{
				if (curMaxSize <= static_cast<int>(queueSize * QUEUE_REDUCTION_LIMIT))
				{
					int k = (queueSize / QUEUE_MIN_SIZE) / 2;

					if (k == 0)
					{
						k = 1;
					}

					m_queue.nonDestructiveResize(QUEUE_MIN_SIZE * k, thread);

					m_statesCountAfterExpand = 0;

					DEBUG_LOG_MSG(m_log, QString("%1 queue reduce to %2").arg(m_appSignalID).arg(QUEUE_MIN_SIZE * k));
				}
				else
				{
					m_queue.resetMaxSize(thread);

					m_statesCountAfterExpand = 0;
				}
			}
		}
	}
}


