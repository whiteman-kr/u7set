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
								arg(m_startReadFromRecord));
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

bool ArchFileToRead::fillBuffer()
{
	if (m_hasDataToRead == false)
	{
		return false;
	}

	m_recordsInBuffer = 0;
	m_nextRecordIndex = 0;

	do
	{
		int maxRecordsToRead = READ_BUFFER_SIZE - m_recordsInBuffer;
		int readCount = 0;

		m_partitionToRead.read(m_readBuffer + m_recordsInBuffer, maxRecordsToRead, &readCount);

		m_recordsInBuffer += readCount;

		if (m_recordsInBuffer < READ_BUFFER_SIZE)
		{
			m_partitionToReadIndex++;

			if (m_partitionToReadIndex >= m_partitionsInfo.count())
			{
				m_hasDataToRead = false;
				break;
			}

			bool res = m_partitionToRead.openForReading(m_partitionsInfo[m_partitionToReadIndex].startTime);

			if (res == false)
			{
				m_hasDataToRead = false;
				break;
			}
		}
	}
	while(m_recordsInBuffer < READ_BUFFER_SIZE);

	return m_recordsInBuffer != 0;
}

bool ArchFileToRead::getRecord(Hash* signalHash, ArchFileRecord* record)
{
	if (m_nextRecordIndex >= m_recordsInBuffer)
	{
		bool hasRecordsInBuffer = fillBuffer();

		if (hasRecordsInBuffer == false)
		{
			return false;
		}
	}

	*signalHash = m_hash;
	*record = m_readBuffer[m_nextRecordIndex];

	return true;			// record is valid
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
	m_hasDataToRead = false;

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

		bool noNeedReadNextPartitions = false;

		ArchFindResult result = m_partitionToRead.findStartPosition(m_timeType,
																	m_startTime,
																	m_endTime,
																	&m_startReadFromRecord,
																	&noNeedReadNextPartitions);

		switch(result)
		{
		case ArchFindResult::Found:
			m_findResult = ArchFindResult::Found;
			m_hasDataToRead = true;
			return;

		case ArchFindResult::NotFound:

			if (noNeedReadNextPartitions == true)
			{
				m_findResult = ArchFindResult::NotFound;
				return;
			}

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


// ---------------------------------------------------------------------------------------------
//
// ArchRequest class implementattion
//
// ---------------------------------------------------------------------------------------------

ArchRequest::ArchRequest(Archive& archive,
						 const ArchRequestParam& param,
						 std::shared_ptr<Network::GetAppSignalStatesFromArchiveNextReply> getNextReply,
						 CircularLoggerShared logger) :
	m_archive(archive),
	m_param(param),
	m_getNextReply(getNextReply),
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
	DEBUG_LOG_MSG(m_logger, QString("ArchRequest is deleted: ID = %1, states sent = %2").
							arg(m_param.requestID()).arg(m_sentStatesCount));
}

void ArchRequest::run()
{
/*	if (m_execParam.print().contains(" 15:00:31") == true)
	{
		DEBUG_STOP;
	}*/

	bool dataFound = prepareArchFilesToRead();

	if (dataFound == false)
	{
		reportNoData();
		waitForQuit();
		return;
	}

	// revert m_execParam times to initial values!
	//
	m_execParam.expandTimes(-Archive::TIME_TO_EXPAND_REQUEST);		// minus is OK!

	prepareGetNextReply();

	while(isQuitRequested() == false)
	{
		if (isNextDataRequired() == true)
		{
			prepareGetNextReply();

			resetNextDataRequired();
		}
		else
		{
			usleep(200);
		}
	}

	finalizeRequest();
}

bool ArchRequest::prepareArchFilesToRead()
{
	assert(m_archFilesToRead.count() == 0);

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
		if (isQuitRequested() == true)
		{
			break;
		}

		ArchFileToRead* archFileToRead = new ArchFileToRead(*fileToRead, m_execParam);

		archFileToRead->findData();

		if (archFileToRead->findResult() == ArchFindResult::Found)
		{
			// initial fileToread buffer filling
			//
			bool hasRecordsInBuffer = archFileToRead->fillBuffer();

			if (hasRecordsInBuffer == true)
			{
				m_archFilesToRead.append(archFileToRead);
				continue;
			}
		}

		delete archFileToRead;							// nothing to read
	}

	return m_archFilesToRead.count() > 0;
}

void ArchRequest::prepareGetNextReply()
{
	if (m_noMoreData == true)
	{
		reportNoMoreData();
		return;
	}

	if (m_sentStatesCount > 0)
	{
		DEBUG_STOP;
	}

	m_getNextReply->Clear();

	getSignalStates();		// fills m_getNextReply->appsignalstates

	int statesInReply = m_getNextReply->appsignalstates_size();

	m_getNextReply->set_requestid(m_param.requestID());

	m_getNextReply->set_error(static_cast<int>(NetworkError::Success));
	m_getNextReply->set_archerror(static_cast<int>(ArchiveError::Success));
	m_getNextReply->clear_errorstring();

	m_sentStatesCount += statesInReply;

	m_getNextReply->set_totalstatescount(0);
	m_getNextReply->set_sentstatescount(m_sentStatesCount);

	m_getNextReply->set_statesinpartcount(statesInReply);
	m_getNextReply->set_islastpart(m_noMoreData);
	m_getNextReply->set_dataready(true);

	setDataReady();
}

void ArchRequest::getSignalStates()
{
	::google::protobuf::RepeatedPtrField< ::Proto::AppSignalState >* states = m_getNextReply->mutable_appsignalstates();

	if (states == nullptr)
	{
		assert(false);
		return;
	}

	//if (m_sentStates)

	states->Reserve(ARCH_REQUEST_MAX_STATES);

	int stateIndex = 0;

	ArchFileRecord record;
	Hash signalHash = 0;

	bool singleFileRequest = m_archFilesToRead.count() == 1;

	do
	{
		if (singleFileRequest == true)
		{
			m_noMoreData = getSingleFileNextRecord(&signalHash, &record);
		}
		else
		{
			m_noMoreData = getMultipleFilesNextRecord(&signalHash, &record);
		}

		if (m_noMoreData == true)
		{
			break;
		}

		if (record.isValid() == false)
		{
			continue;
		}

		qint64 recordTime = record.getTime(m_execParam.timeType());

		if (recordTime < m_execParam.startTime())
		{
			continue;
		}

		if (recordTime > m_execParam.endTime())
		{
			m_noMoreData = true;
			break;
		}

		// add record to reply
		//
		Proto::AppSignalState* state = states->Add();

		if (state == nullptr)
		{
			assert(false);
			break;
		}

		state->set_hash(signalHash);
		state->set_value(record.state.value);
		state->set_flags(record.state.flags.all);
		state->set_planttime(record.state.plantTime);
		state->set_systemtime(record.state.systemTime);

		// conversion from UTC to localtime
		//
		QDateTime localDateTime = QDateTime::fromMSecsSinceEpoch(record.state.systemTime);

		localDateTime.setTimeSpec(Qt::UTC);		// to prevent time shifting in next convertion toMSecsSinceEpoch

		qint64 localTime = localDateTime.toMSecsSinceEpoch();

		state->set_localtime(localTime);

		state->set_archiveid(0);

		stateIndex++;

		if (stateIndex >= ARCH_REQUEST_MAX_STATES)
		{
			break;
		}
	}
	while(1);
}

bool ArchRequest::getSingleFileNextRecord(Hash* hash, ArchFileRecord* record)
{
	assert(m_archFilesToRead.count() == 1);

	ArchFileToRead* archFileToRead = m_archFilesToRead[0];

	bool recordIsValid = archFileToRead->getRecord(hash, record);

	if (recordIsValid == false)
	{
		return true;		// TRUE - if no more data exists
	}

	archFileToRead->gotoNextRecord();

	return false;			// FALSE - if more data exists
}

bool ArchRequest::getMultipleFilesNextRecord(Hash* hash, ArchFileRecord* record)
{
	assert(m_archFilesToRead.count() > 1);

	// function returns TRUE! if NO MORE data exists

	if (m_lastFileIndex != -1)
	{
		// records of one file (signal) with same time should be returned sequentlial
		// checking next record of previously read file

		ArchFileToRead* archFileToRead = m_archFilesToRead[m_lastFileIndex];

		bool recordIsValid = archFileToRead->getRecord(hash, record);

		if (recordIsValid == true)
		{
			if (record->getTime(m_execParam.timeType()) == m_lastRecordTime)
			{
				// m_lastFileIndex no change
				// m_lastRecordTime no change
				//
				archFileToRead->gotoNextRecord();
				return false;
			}
		}
	}

	//

	Hash h;
	ArchFileRecord rec;
	qint64 minTime = std::numeric_limits<qint64>::max();
	bool noMoreData = true;

	m_lastFileIndex = -1;

	int archFileToReadCount = m_archFilesToRead.count();

	for(int i = 0 ; i < archFileToReadCount; i++)
	{
		ArchFileToRead* archFileToRead = m_archFilesToRead[i];

		bool recordIsValid = archFileToRead->getRecord(&h, &rec);

		if (recordIsValid == false)
		{
			continue;
		}

		noMoreData = false;

		qint64 recordTime = record->getTime(m_execParam.timeType());

		if (recordTime < minTime)
		{
			minTime = recordTime;

			m_lastRecordTime = recordTime;
			m_lastFileIndex = i;

			*record = rec;
			*hash = h;
		}
	}

	if (noMoreData == false)
	{
		assert(m_lastFileIndex != -1);
		m_archFilesToRead[m_lastFileIndex]->gotoNextRecord();
	}

	return noMoreData;
}

void ArchRequest::reportError()
{
	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: error occured!").arg(m_param.requestID()));

	m_getNextReply->set_requestid(m_param.requestID());

	m_getNextReply->set_error(static_cast<int>(NetworkError::ArchiveError));
	m_getNextReply->set_archerror(static_cast<int>(ArchiveError::SearchError));
	m_getNextReply->set_errorstring(m_errMsg.toStdString());

	m_getNextReply->set_totalstatescount(0);
	m_getNextReply->set_sentstatescount(0);
	m_getNextReply->set_statesinpartcount(0);
	m_getNextReply->set_islastpart(true);
	m_getNextReply->clear_appsignalstates();

	m_getNextReply->set_dataready(true);

	setDataReady();

	m_noMoreData = true;
}

void ArchRequest::reportNoData()
{
	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: data not found!").arg(m_param.requestID()));

	m_getNextReply->set_requestid(m_param.requestID());

	m_getNextReply->set_error(static_cast<int>(NetworkError::Success));
	m_getNextReply->set_archerror(static_cast<int>(ArchiveError::Success));
	m_getNextReply->clear_errorstring();

	m_getNextReply->set_totalstatescount(0);
	m_getNextReply->set_sentstatescount(0);
	m_getNextReply->set_statesinpartcount(0);
	m_getNextReply->set_islastpart(true);
	m_getNextReply->clear_appsignalstates();

	m_getNextReply->set_dataready(true);

	setDataReady();

	m_noMoreData = true;
}

void ArchRequest::reportNoMoreData()
{
	assert(m_noMoreData == true);

	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: has no more data!").arg(m_param.requestID()));

	m_getNextReply->set_requestid(m_param.requestID());

	m_getNextReply->set_error(static_cast<int>(NetworkError::Success));
	m_getNextReply->set_archerror(static_cast<int>(ArchiveError::Success));
	m_getNextReply->clear_errorstring();

	m_getNextReply->set_totalstatescount(0);
	m_getNextReply->set_sentstatescount(m_sentStatesCount);
	m_getNextReply->set_statesinpartcount(0);
	m_getNextReply->set_islastpart(true);
	m_getNextReply->clear_appsignalstates();

	m_getNextReply->set_dataready(true);

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
	for(ArchFileToRead* fileToRead : m_archFilesToRead)
	{
		delete fileToRead;
	}

	m_archFilesToRead.clear();
}


