#include <CommonLib/ConstStrings.h>

#include <QDirIterator>
#include <QRegularExpression>

#include "ArchFile.h"
#include "ArchRequest.h"
#include "ArchWriterThread.h"
#include <ArchSignal.pb.h>

// ----------------------------------------------------------------------------------------------------------------------
//
// ArchFile class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

ArchFile::ArchFile(const Proto::ArchSignal& protoArchSignal, const QString& archFullPath, CircularLoggerShared log) :
	m_log(log),
	m_queue(static_cast<E::SignalType>(protoArchSignal.signaltype()) == E::SignalType::Analog ?
																QUEUE_MIN_SIZE * 16 : QUEUE_MIN_SIZE)
{
	m_appSignalID = QString::fromStdString(protoArchSignal.appsignalid());
	m_hash = calcHash(m_appSignalID);
	m_isAnalog = static_cast<E::SignalType>(protoArchSignal.signaltype()) == E::SignalType::Analog;
	m_lastRecord.state.flags.valid = 0;

	QString lastByteOfHash = (QString("%1").arg(static_cast<int>(m_hash & 0xFF), 2, 16, Latin1Char::ZERO)).toUpper();

	m_path = QString("%1/%2/%3").
					arg(archFullPath).
					arg(lastByteOfHash).
					arg(m_appSignalID.remove(QRegularExpression("[^0-9A-Za-z_]")));

	m_writablePartition.init(m_path, true);
}

ArchFile::~ArchFile()
{
}

bool ArchFile::pushState(const SimpleAppSignalState& state)
{
	controlQueueSizeBeforePush();

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

		m_queue.push(nvp);
	}

	m_queue.push(s);

	m_lastRecord = s;
	m_lastRecordInitialized = true;

	return true;
}

bool ArchFile::flush(qint64 curPartition,
					 qint64* totalFushedStatesCount,
					 bool flushAnyway,
					 int minQueueSizeForFlushing,
					 ArchFileRecord* buffer,
					 int bufferSize)
{
	TEST_PTR_RETURN_FALSE(totalFushedStatesCount);
	TEST_PTR_RETURN_FALSE(buffer);

	int queueSize = m_queue.size();

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

	bool result = m_queue.popToBuffer(buffer, bufferSize, &copiedItemsCount);

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
	return m_queue.size() >= static_cast<int>(m_queue.queueSize() * QUEUE_EMERGENCY_LIMIT);
}

QVector<ArchFilePartition::Info> ArchFile::getArchPartitionsInfo(const QString& path)
{
	QVector<ArchFilePartition::Info> partitionsInfo;

	// Arch file name format: 2018_12_31_23_59.sta (or *.lta)

	QRegularExpression archFileNameTemplate(ARCH_FILE_NAME_TEMPLATE);

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

		pi.date = QDateTime(QDate(year, month, day), QTime(hour, minute, 0, 0), TIME_ZONE_UTC);
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
						int bufferSize)
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

		m_queue.push(m_lastRecord);
	}

	flush(curPartition,
		  totalFlushedStatesCount,
		  true,
		  Archive::DEFAULT_QUEUE_SIZE_FOR_FLUSHING /* value doesn't matter, because prev param says FlushAnyway */,
		  buffer,
		  bufferSize);
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

void ArchFile::controlQueueSizeBeforePush()
{
	int curSize = 0;
	int curMaxSize = 0;
	int queueSize = 0;

	m_queue.getSizes(&curSize, &curMaxSize, &queueSize);

	if (curSize >= static_cast<int>(queueSize * QUEUE_EXPAND_LIMIT) && queueSize < QUEUE_MAX_SIZE)
	{
		int k = (queueSize / QUEUE_MIN_SIZE) * 2;

		if (k == 0)
		{
			k = 1;
		}

		m_queue.nonDestructiveResize(QUEUE_MIN_SIZE * k);

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

					m_queue.nonDestructiveResize(QUEUE_MIN_SIZE * k);

					m_statesCountAfterExpand = 0;

					DEBUG_LOG_MSG(m_log, QString("%1 queue reduce to %2").arg(m_appSignalID).arg(QUEUE_MIN_SIZE * k));
				}
				else
				{
					m_queue.resetMaxSize();

					m_statesCountAfterExpand = 0;
				}
			}
		}
	}
}


