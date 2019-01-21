#include "../lib/WUtils.h"

#include "ArchRequest.h"
#include "ArchWriterThread.h"
#include "FileArchReader.h"

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
	prepareFiles();

	ArchFindResult findResult = findData();

	if (findResult == ArchFindResult::SearchError)
	{
		reportError();
		waitForQuit();
		return;
	}

	if (findResult == ArchFindResult::NotFound)
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

void ArchRequest::prepareFiles()
{
	assert(m_archFiles.count() == 0);
	assert(m_archFilesArray.count() == 0);

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

		m_archFiles.insert(signalHash, archFile);
		m_archFilesArray.append(archFile);

		m_archive.flushImmediately(archFile);
	}
}

ArchFindResult ArchRequest::findData()
{
	ArchFindResult result = ArchFindResult::NotFound;

	for(ArchFile* archFile : m_archFilesArray)
	{
		ArchFindResult res = archFile->findData(m_execParam);

		if (res == ArchFindResult::Found)
		{
			result = ArchFindResult::Found;
		}
	}

	return result;
}

void ArchRequest::getNextData()
{
	if (m_noMoreData == true)
	{
		return;
	}

	resetNextDataRequired();



	//
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
	for(ArchFile* archFile : m_archFiles)
	{
		if (archFile == nullptr)
		{
			assert(false);
			continue;
		}

		archFile->finalizeRequest(m_param.requestID());
	}
}


