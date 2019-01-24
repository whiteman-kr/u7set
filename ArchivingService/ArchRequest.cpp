#include "../lib/WUtils.h"

#include "ArchRequest.h"
#include "ArchWriterThread.h"
#include "FileArchReader.h"

// ----------------------------------------------------------------------------------------------------------------------
//
// ArchFileRequestData class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

ArchFileRequestData::ArchFileRequestData(const ArchFile& archFile, const ArchRequestParam& param) :
	m_archFilePath(archFile.path()),
	m_appSignalID(archFile.appSignalID()),
	m_requestID(param.requestID()),
	m_timeType(param.timeType()),
	m_startTime(param.startTime()),
	m_endTime(param.endTime())
{
	m_partitionToRead.init(m_archFilePath, false);
}

ArchFileRequestData::~ArchFileRequestData()
{
//	delete m_partitionToRead;
}

void ArchFileRequestData::findData()
{
	getArchPartitionsInfo();

	if (m_partitionsInfo.count() == 0)
	{
		m_findResult = ArchFindResult::NotFound;
		return;
	}

	findStartPosition();

	if (m_findResult == ArchFindResult::Found)
	{
		qDebug() << C_STR(QString("ArchRequest ID=%1, signal %2 data found, partition %3, record=%4").
								arg(m_requestID).
								arg(m_appSignalID).
								arg(partitionToReadInfo().fileName).
								arg(m_startRecord));
	}
	else
	{
		qDebug() << C_STR(QString("ArchRequest ID=%1, signal %2 data NOT found").
								arg(m_requestID).
								arg(m_appSignalID));
	}
}

ArchFileRequestData::PartitionInfo ArchFileRequestData::partitionToReadInfo()
{
	if (m_partitionToReadIndex >= 0 && m_partitionToReadIndex < m_partitionsInfo.count())
	{
		return m_partitionsInfo[m_partitionToReadIndex];
	}

	assert(false);

	return PartitionInfo();
}

bool ArchFileRequestData::getRecord(ArchFileRecord* record)
{
	if (m_hasData == false)
	{
		return false;
	}

	if (m_nextRecordIndex >= m_recordsInBuffer)
	{
		fillBuffer();

		if (m_recordsInBuffer == 0)
		{
			return false;
		}
	}

	*record = m_readBuffer[m_nextRecordIndex];

	return true;
}

bool ArchFileRequestData::gotoNextRecord()
{
	m_nextRecordIndex++;

	if (m_nextRecordIndex > m_recordsInBuffer)
	{
		m_nextRecordIndex = m_recordsInBuffer;
		return false;
	}

	return true;
}

void ArchFileRequestData::getArchPartitionsInfo()
{
	m_partitionsInfo.clear();

	// Arch file name format: 2018_12_31_23_59.saf (year_month_day_hour_minute.saf)

	QRegExp archFileNameTemplate(QString("2[0-9][0-9][0-9]_[0-1][0-9]_[0-3][0-9]_[0-2][0-9]_[0-5][0-9].%1").arg(ArchFile::EXTENSION));

	QDirIterator di(m_archFilePath, QDir::Files);

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

		m_partitionsInfo.append(pi);
	}
}

void ArchFileRequestData::findStartPosition()
{
	m_findResult = ArchFindResult::NotFound;
	m_hasData = false;

	int partitionsCount = m_partitionsInfo.count();

	if (partitionsCount == 0)
	{
		return;
	}

	// 1) Sort m_archPartitionsInfo by systemTime ascending
	//

	qSort(m_partitionsInfo);

	// 2) Find LAST partition where systemTime < m_startTime

	m_partitionToReadIndex = 0;

	for(int i = 1 /* it is Ok */ ; i < partitionsCount; i++)
	{
		const PartitionInfo& pi = m_partitionsInfo[i];

		if (pi.startTime >= m_startTime)
		{
			break;
		}

		m_partitionToReadIndex = i;
	}

	// 3)

	while(m_partitionToReadIndex < partitionsCount)
	{
		const PartitionInfo& partitionToReadInfo = m_partitionsInfo[m_partitionToReadIndex];

		bool res = m_partitionToRead.openForReading(partitionToReadInfo.startTime);

		if (res == false)
		{
			m_partitionToReadIndex++;
			continue;
		}

		ArchFindResult result = m_partitionToRead.findStartPosition(m_timeType, m_startTime, m_endTime,& m_startRecord);

		switch(result)
		{
		case ArchFindResult::Found:
			m_findResult = ArchFindResult::Found;
			m_hasData = true;
			return;

		case ArchFindResult::NotFound:
			m_partitionToReadIndex++;
			break;

		case ArchFindResult::SearchError:

			m_partitionToRead.close();
			m_partitionToReadIndex = -1;
			return;

		default:
			assert(false);
		}
	}

	m_partitionToRead.close();
	m_partitionToReadIndex = -1;
}

bool ArchFileRequestData::fillBuffer()
{
	if (m_hasData == false)
	{
		return false;
	}

	m_recordsInBuffer = 0;
	m_nextRecordIndex = 0;

	do
	{
		int maxRecordsToRead = RECORDS_BUFFER_SIZE - m_recordsInBuffer;
		int readCount = 0;

		m_partitionToRead.read(m_readBuffer + m_recordsInBuffer, maxRecordsToRead, &readCount);

		m_recordsInBuffer += readCount;
		maxRecordsToRead -= readCount;

		if (m_recordsInBuffer < RECORDS_BUFFER_SIZE)
		{
			m_partitionToReadIndex++;

			if (m_partitionToReadIndex >= m_partitionsInfo.count())
			{
				m_hasData = false;
				break;
			}

			bool res = m_partitionToRead.openForReading(m_partitionsInfo[m_partitionToReadIndex].startTime);

			if (res == false)
			{
				m_hasData = false;
				break;
			}
		}
	}
	while(m_recordsInBuffer < RECORDS_BUFFER_SIZE);

	return m_recordsInBuffer != 0;
}



// ---------------------------------------------------------------------------------------------
//
// ArchRequest class implementattion
//
// ---------------------------------------------------------------------------------------------

ArchRequest::ArchRequest(Archive& archive, const ArchRequestParam& param, CircularLoggerShared logger) :
	m_archive(archive),
	m_param(param),
	m_startTime(QDateTime::currentMSecsSinceEpoch()),
	m_logger(logger),
	m_execParam(m_param)
{
	qint64 localTimeOffset = Archive::localTimeOffsetFromUtc();

	switch(m_param.timeType())
	{
	case E::TimeType::Plant:
	case E::TimeType::System:
//	case E::TimeType::ArchiveId:
		break;

	case E::TimeType::Local:
		{
			// convert local time to system time
			//
			m_execParam.setStartTime(m_param.startTime() - localTimeOffset);
			m_execParam.setEndTime(m_param.endTime() - localTimeOffset);
			m_execParam.setTimeType(E::TimeType::System);
		}
		break;

	default:
		assert(false);
	}

	// expand request time from both sides
	//
	m_execParam.expandTimes(Archive::TIME_TO_EXPAND_REQUEST);

	DEBUG_LOG_MSG(m_logger, QString("ArchRequest created: %1").arg(m_param.print()));
	DEBUG_LOG_MSG(m_logger, QString("ArchRequest to exec: %1").arg(m_execParam.print()));
}

ArchRequest::~ArchRequest()
{
	DEBUG_LOG_MSG(m_logger, QString("ArchRequest is deleted: ID = %1").arg(m_param.requestID()));
}

void ArchRequest::run()
{
	prepareArchRequestData();

	bool dataFound = findData();

/*	if (findResult == ArchFindResult::SearchError)
	{
		reportError();
		waitForQuit();
		return;
	} */

	if (dataFound == false)
	{
		reportNoData();
		waitForQuit();
		return;
	}

	nextDataRequired();

	do
	{
		if (isNextDataRequired() == true)
		{
			getNextData();
		}
		else
		{
			usleep(200);
		}
	}

	while(isQuitRequested() == false);

	finalizeRequest();
}

void ArchRequest::prepareArchRequestData()
{
	assert(m_requestData.count() == 0);
	assert(m_requestDataArray.count() == 0);

	// filling m_archFiles and m_archFilesArray
	// and enqueue files for immediately flushing
	//
	for(Hash signalHash : m_param.signalHashes())
	{
		ArchFile* archFile = m_archive.getArchFile(signalHash);

		if (archFile == nullptr)
		{
			assert(false);
			continue;
		}

		ArchFileRequestData* requestData = new ArchFileRequestData(*archFile, m_execParam);

		m_requestData.insert(signalHash, requestData);
		m_requestDataArray.append(requestData);

		m_archive.flushImmediately(archFile);
	}
}

bool ArchRequest::findData()
{
	m_filesWithData.clear();

	for(ArchFileRequestData* requestData : m_requestDataArray)
	{
		requestData->findData();

		if (requestData->findResult() == ArchFindResult::Found)
		{
			m_filesWithData.append(requestData);
		}
	}

	return m_filesWithData.count() > 0;
}

void ArchRequest::getNextData()
{
	if (m_noMoreData == true)
	{
		return;
	}

	int filesWithDataCount = m_filesWithData.count();

	if (m_firstCallOfGetNextData == true)
	{
		m_fileData.resize(filesWithDataCount);

		for(int i = 0; i < filesWithDataCount; i++)
		{
			ArchFileData& afd = m_fileData[i];

			afd.hasMoreData = m_filesWithData[i]->getRecord(&afd.record);

			if (afd.hasMoreData == true)
			{
				afd.recordTime = afd.record.getTime(m_execParam.timeType());
				m_noMoreData = false;
			}
		}

		m_firstCallOfGetNextData = false;
	}

	m_reply.set_requestid(m_param.requestID());

	m_reply.set_error(static_cast<int>(NetworkError::Success));
	m_reply.set_archerror(static_cast<int>(ArchiveError::Success));
	m_reply.clear_errorstring();

	m_reply.Clear();
//	m_reply.appsignalstates.Reserve(ARCH_REQUEST_MAX_STATES);

	int statesInPeplyCount = 0;

	qint64 minTime = -1;

	do
	{
		bool hasData = false;

		for(ArchFileRequestData* requestData : m_filesWithData)
		{
			ArchFileRecord record;

			bool res = requestData->getRecord(&record);

			if (res == false)
			{
				continue;
			}
			zxdcvwevsvd

		}
	}
	while(1);

	m_reply.set_totalstatescount(0);
	m_reply.set_sentstatescount(0);

	m_reply.set_statesinpartcount(statesInPeplyCount);
	m_reply.set_islastpart(m_noMoreData);
	m_reply.set_dataready(true);

	setDataReady();


	resetNextDataRequired();
}

void ArchRequest::reportError()
{
	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: error occured!").arg(m_param.requestID()));

	m_reply.set_requestid(m_param.requestID());

	m_reply.set_error(static_cast<int>(NetworkError::ArchiveError));
	m_reply.set_archerror(static_cast<int>(ArchiveError::SearchError));
	m_reply.set_errorstring(m_errMsg.toStdString());

	m_reply.set_totalstatescount(0);
	m_reply.set_sentstatescount(0);
	m_reply.set_statesinpartcount(0);
	m_reply.set_islastpart(true);
	m_reply.clear_appsignalstates();

	m_reply.set_dataready(true);

	setDataReady();

	m_noMoreData = true;
}

void ArchRequest::reportNoData()
{
	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: data not found!").arg(m_param.requestID()));

	m_reply.set_requestid(m_param.requestID());

	m_reply.set_error(static_cast<int>(NetworkError::Success));
	m_reply.set_archerror(static_cast<int>(ArchiveError::Success));
	m_reply.clear_errorstring();

	m_reply.set_totalstatescount(0);
	m_reply.set_sentstatescount(0);
	m_reply.set_statesinpartcount(0);
	m_reply.set_islastpart(true);
	m_reply.clear_appsignalstates();

	m_reply.set_dataready(true);

	setDataReady();

	m_noMoreData = true;
}

void ArchRequest::reportDataReady()
{

}

void ArchRequest::waitForQuit()
{
	finalizeRequest();

	while(isQuitRequested() == false)
	{
		usleep(200);
	}
}

void ArchRequest::finalizeRequest()
{
}


