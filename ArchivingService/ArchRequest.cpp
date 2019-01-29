#include "../lib/WUtils.h"

#include "ArchRequest.h"
#include "ArchWriterThread.h"
#include "FileArchReader.h"

// ----------------------------------------------------------------------------------------------------------------------
//
// ArchFileRequestData class implementation
//
// ----------------------------------------------------------------------------------------------------------------------

ArchFileToRead::ArchFileToRead(const ArchFile& archFile, const ArchRequestParam& param) :
	m_archFilePath(archFile.path()),
	m_appSignalID(archFile.appSignalID()),
	m_hash(archFile.hash()),
	m_requestID(param.requestID()),
	m_timeType(param.timeType()),
	m_startTime(param.startTime()),
	m_endTime(param.endTime())
{
	m_partitionToRead.init(m_archFilePath, false);
}

ArchFileToRead::~ArchFileToRead()
{
//	delete m_partitionToRead;
}

void ArchFileToRead::findData()
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

ArchFileToRead::PartitionInfo ArchFileToRead::partitionToReadInfo()
{
	if (m_partitionToReadIndex >= 0 && m_partitionToReadIndex < m_partitionsInfo.count())
	{
		return m_partitionsInfo[m_partitionToReadIndex];
	}

	assert(false);

	return PartitionInfo();
}

bool ArchFileToRead::getRecord(ArchFileRecord* record)
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

bool ArchFileToRead::gotoNextRecord()
{
	m_nextRecordIndex++;

	if (m_nextRecordIndex > m_recordsInBuffer)
	{
		m_nextRecordIndex = m_recordsInBuffer;
		return false;
	}

	return true;
}

void ArchFileToRead::getArchPartitionsInfo()
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

void ArchFileToRead::findStartPosition()
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

bool ArchFileToRead::fillBuffer()
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
	m_logger(logger),
	m_startTime(QDateTime::currentMSecsSinceEpoch()),
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
	bool dataFound = prepareArchFilesToRead();

	if (dataFound == false)
	{
		reportNoData();
		waitForQuit();
		return;
	}

	getNextData();

	do
	{
		if (isNextDataRequired() == true)
		{
			getNextData();

			resetNextDataRequired();
		}
		else
		{
			usleep(200);
		}
	}

	while(isQuitRequested() == false);

	finalizeRequest();
}

bool ArchRequest::prepareArchFilesToRead()
{
	assert(m_archFileToRead.count() == 0);

	QVector<ArchFile*> filesToRead;

	filesToRead.reserve(m_param.signalHashes().count());

	// enqueue arch files to immediately flushing
	//
	for(Hash signalHash : m_param.signalHashes())
	{
		ArchFile* archFile = m_archive.getArchFile(signalHash);

		if (archFile == nullptr)
		{
			assert(false);
			continue;
		}

		m_archive.flushImmediately(archFile);

		filesToRead.append(archFile);
	}

	// find in arch files start positions to read
	//
	for(ArchFile* fileToRead : filesToRead)
	{
		ArchFileToRead* archFileToRead = new ArchFileToRead(*fileToRead, m_execParam);

		archFileToRead->findData();

		if (archFileToRead->findResult() == ArchFindResult::Found)
		{
			m_archFileToRead.append(archFileToRead);		//
		}
		else
		{
			delete archFileToRead;							// nothing to read
		}
	}

	return m_archFileToRead.count() > 0;
}

void ArchRequest::getNextData()
{
	if (m_noMoreData == true)
	{
		reportNoMoreData();
		return;
	}

	m_reply.Clear();

	getSignalStates();		// fills m_reply.appsignalstates

	int statesInReply = m_reply.appsignalstates_size();

	m_reply.set_requestid(m_param.requestID());

	m_reply.set_error(static_cast<int>(NetworkError::Success));
	m_reply.set_archerror(static_cast<int>(ArchiveError::Success));
	m_reply.clear_errorstring();

	m_sentStatesCount += statesInReply;

	m_reply.set_totalstatescount(0);
	m_reply.set_sentstatescount(m_sentStatesCount);

	m_reply.set_statesinpartcount(statesInReply);
	m_reply.set_islastpart(m_noMoreData);
	m_reply.set_dataready(true);

	setDataReady();
}

bool ArchRequest::getSignalStates()
{
	::google::protobuf::RepeatedPtrField< ::Proto::AppSignalState >* states = m_reply.mutable_appsignalstates();

	if (states == nullptr)
	{
		assert(false);
		return false;
	}

	states->Reserve(ARCH_REQUEST_MAX_STATES);

	int stateIndex = 0;

	do
	{
		for(ArchFileToRead* file : m_archFileToRead)
		{

			bool hasMoreData = getNextRecord(&signalHash, &record);

			if (hasMoreData == false)
			{
				break;
			}
		}

		Proto::AppSignalState* state = states->Mutable(stateIndex);

		if (state == nullptr)
		{
			assert(false);
			break;
		}

		stateIndex++;

		if (stateIndex >= ARCH_REQUEST_MAX_STATES)
		{
			break;
		}
	}
	while(1);

	if (hasMoreData == false)
	{
		m_noMoreData = true;
	}


}

bool ArchRequest::getNextRecord(Hash* hash, ArchFileRecord* record)
{

	int filesWithDataCount = m_filesWithData.count();

	if (m_firstCallOfGetNextRecord == true)
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

		m_firstCallOfGetNextRecord = false;
	}



	for(ArchFileToRead* requestData : m_filesWithData)
	{
		ArchFileRecord record;

		bool res = requestData->getRecord(&record);

		if (res == false)
		{
			continue;
		}


	}

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

void ArchRequest::reportNoMoreData()
{
	assert(m_noMoreData == true);

	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: has no more data!").arg(m_param.requestID()));

	m_reply.set_requestid(m_param.requestID());

	m_reply.set_error(static_cast<int>(NetworkError::Success));
	m_reply.set_archerror(static_cast<int>(ArchiveError::Success));
	m_reply.clear_errorstring();

	m_reply.set_totalstatescount(0);
	m_reply.set_sentstatescount(m_sentStatesCount);
	m_reply.set_statesinpartcount(0);
	m_reply.set_islastpart(true);
	m_reply.clear_appsignalstates();

	m_reply.set_dataready(true);

	setDataReady();
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


