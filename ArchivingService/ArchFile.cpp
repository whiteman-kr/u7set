#include "ArchFile.h"

#include "FileArchWriter.h"

// -----------------------------------------------------------------------------------------------------------------------
//
// ArchFile::Record struct implementation
//
// -----------------------------------------------------------------------------------------------------------------------

bool ArchFile::Record::isValid() const
{
	return calcCrc16(this, sizeof(ArchFile::Record)) == 0;
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

		QDateTime date = QDateTime::fromMSecsSinceEpoch(partition, Qt::UTC);

		QString fileName = QString("%1/%2_%3_%4_%5_%6.%7").
								arg(m_archFile.path()).
								arg(date.date().year()).
								arg(QString().sprintf("%02d", date.date().month())).
								arg(QString().sprintf("%02d", date.date().day())).
								arg(QString().sprintf("%02d", date.time().hour())).
								arg(QString().sprintf("%02d", date.time().minute()),
								EXTENSION);

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

bool ArchFile::Partition::close()
{
	closeFile();

	return true;
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

	m_lastState = state;

	Record s;

	s.state.archID = archID;
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

bool ArchFile::findData(const ArchRequestParam& param)
{
	RequestData* rd = m_requestsData.value(param.requestID, nullptr);

	if (rd != nullptr)
	{
		m_requestsData.remove(param.requestID);
		delete rd;
	}

	m_requestsData.insert(param.requestID, rd);

	rd = new RequestData(*this, param);

	bool result = false;

	result = getArchPartitionsInfo(rd);

	if (result == false)
	{
		cancelRequest(rd->requestID);
		return false;
	}

	result = findStartPosition(rd);

	if (result == false)
	{
		cancelRequest(rd->requestID);
		return false;
	}

	return true;
}

void ArchFile::shutdown(qint64 curPartition, qint64* totalFlushedStatesCount)
{
	flush(curPartition, totalFlushedStatesCount, true);
}

bool ArchFile::getArchPartitionsInfo(RequestData* rd)
{
	TEST_PTR_RETURN_FALSE(rd);

	rd->partitionsInfo.clear();

	// Arch file name format: 2018_12_31_23_59.saf (year_month_day_hour_minute.saf)

	QRegExp archFileNameTemplate(QString("2[0-9][0-9][0-9]_[0-1][0-9]_[0-3][0-9]_[0-2][0-9]_[0-5][0-9].%1").arg(ArchFile::EXTENSION));

	QDirIterator di(m_path);

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

	return rd->partitionsInfo.count() > 0;
}

bool ArchFile::findStartPosition(RequestData* rd)
{
	TEST_PTR_RETURN_FALSE(rd);

	int partitionsCount = rd->partitionsInfo.count();

	if (partitionsCount == 0)
	{
		return false;
	}

	// 1) Sort m_archPartitionsInfo by systemTime ascending
	//

	qSort(rd->partitionsInfo);

	// 2) Find LAST partition where systemTime < m_startTime

	int startPartitionIndex = -1;

	for(int i = 0; i < partitionsCount; i++)
	{
		const PartitionInfo& pi = rd->partitionsInfo[i];

		if (pi.startTime >= rd->startTime)
		{
			break;
		}

		startPartitionIndex = i;
	}

	if (startPartitionIndex == -1)
	{
		startPartitionIndex = 0;
	}

	return true;
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

