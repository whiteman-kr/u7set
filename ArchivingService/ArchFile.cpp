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
// ArchFile::Partition classimplementation
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


// -----------------------------------------------------------------------------------------------------------------------
//
// ArchFile class implementation
//
// -----------------------------------------------------------------------------------------------------------------------

ArchFile::Record ArchFile::m_buffer[ArchFile::QUEUE_MAX_SIZE];

const QString ArchFile::EXTENSION = "saf";		// Signal Archive File

ArchFile::ArchFile() :
	m_writablePartition(*this, true)
{
}

void ArchFile::init(Archive* archive, ArchSignal* archSignal)
{
	TEST_PTR_RETURN(archive);
	TEST_PTR_RETURN(archSignal);

	m_archive = archive;
	m_archSignal = archSignal;

	int queueSize = QUEUE_MIN_SIZE;

	if (archSignal->isAnalog == true)
	{
		queueSize = QUEUE_MIN_SIZE * 16;
	}

	m_queue = new FastQueue<Record>(queueSize);

	m_path = QString("%1/%2/%3").
					arg(archive->archFullPath()).
					arg(QString().sprintf("%02X", static_cast<int>(archSignal->hash & 0xFF))).
					arg(archSignal->appSignalID.remove(QRegExp("[^0-9A-Za-z_]")));
}


ArchFile::~ArchFile()
{
	for(RequestContext* context : m_requestContexts)
	{
		delete context;
	}

	m_requestContexts.clear();
}

bool ArchFile::pushState(qint64 archID, const SimpleAppSignalState& state)
{
	TEST_PTR_RETURN_FALSE(m_queue);

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
	if (m_requestContexts.contains(param.requestID) == true)
	{
		assert(false);
		return false;
	}

	RequestContext* rc = new RequestContext(param);

	m_requestContexts.insert(param.requestID, rc);

	return privateFindData(rc);
}

void ArchFile::shutdown(qint64 curPartition, qint64* totalFlushedStatesCount)
{
	flush(curPartition, totalFlushedStatesCount, true);
}

bool ArchFile::privateFindData(RequestContext* rc)
{
	TEST_PTR_RETURN_FALSE(rc);

	return true;
}


