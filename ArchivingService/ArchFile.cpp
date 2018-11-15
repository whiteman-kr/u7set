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
// ArchFile class implementation
//
// -----------------------------------------------------------------------------------------------------------------------

ArchFile::Record ArchFile::m_buffer[ArchFile::QUEUE_MAX_SIZE];

const QString ArchFile::EXTENSION = "saf";		// Signal Archive File

ArchFile::ArchFile(Archive* archive, ArchSignal* archSignal) :
	m_archive(archive),
	m_archSignal(archSignal)
{
	TEST_PTR_RETURN(archive);
	TEST_PTR_RETURN(archSignal);

	int queueSize = QUEUE_MIN_SIZE;

	if (archSignal->isAnalog == true)
	{
		queueSize = QUEUE_MIN_SIZE * 16;
	}

	m_queue = new FastQueue<Record>(queueSize);

	m_path = QString("%1/%2/%3").
					arg(archive->archFullPath()).
					arg(QString().sprintf("%02X", static_cast<int>(archSignal->hash & 0xFF))).
					arg(archSignal->appSignalID.remove(QRegExp("[^A-Z][^a-z][^0-9][^_]")));
}

ArchFile::~ArchFile()
{
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
	s.crc16 = calcCrc16(&s.state, sizeof(s.state));

	SimpleMutexLocker locker(&m_flushMutex);

	Q_UNUSED(locker);

	m_queue->push(s);

	return true;
}

bool ArchFile::flush(qint64 curPartition, qint64* totalFushedStatesCount)
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

	int copiedItemsCount = 0;

	bool result = m_queue->copyToBuffer(m_buffer, QUEUE_MAX_SIZE, &copiedItemsCount);

	if (result == false || copiedItemsCount == 0)
	{
		setRequiredImmediatelyFlushing(false);
		return false;
	}

	if (curPartition != m_prevPartition)
	{
		closeFile();

		m_prevPartition = curPartition;
	}

	writeFile(curPartition, m_buffer, copiedItemsCount, totalFushedStatesCount);

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
	flush(curPartition, totalFlushedStatesCount);
	closeFile();
}

bool ArchFile::writeFile(qint64 partition, Record* buffer, int statesCount, qint64* totalFushedStatesCount)
{
	TEST_PTR_RETURN_FALSE(buffer);
	TEST_PTR_RETURN_FALSE(totalFushedStatesCount);

	if (m_fileIsOpened == false)
	{
		if (m_pathIsExists == false)
		{
			QDir d;

			m_pathIsExists = d.mkpath(m_path);
		}

		QDateTime date = QDateTime::fromMSecsSinceEpoch(partition, Qt::UTC);

		QString fileName = QString("%1/%2_%3_%4_%5_%6.%7").
								arg(m_path).
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

		m_fileIsOpened = true;

		if (m_fileIsAligned == false)
		{
			QFileInfo fi(m_file);

			qint64 fileSize = fi.size();

			if ((fileSize % sizeof(Record)) != 0)
			{
				m_file.seek((fileSize / sizeof(Record)) * sizeof(Record));
			}

			m_fileIsAligned = true;
		}
	}

	qint64 sizeToWrite = statesCount * sizeof(Record);

	qint64 written = m_file.write(reinterpret_cast<const char*>(buffer), sizeToWrite);

	if (written == -1)
	{
		return false;
	}

	if (sizeToWrite != written)
	{
		m_fileIsAligned = false;
	}

	*totalFushedStatesCount += statesCount;

//	qDebug() << C_STR(QString("Flush %1 states %2").arg(m_file.fileName()).arg(statesCount));

	return true;
}

void ArchFile::closeFile()
{
	if (m_fileIsOpened == false)
	{
		return;
	}

	m_file.close();

	m_fileIsOpened = false;
}

