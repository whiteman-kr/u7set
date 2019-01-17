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

	DEBUG_LOG_MSG(m_logger, QString("ArchRequest ID = %1 is created").arg(m_param.requestID()));

/*	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: %2, time = %3, start = %4, end = %5, signals = %6").
				  arg(context->requestID()).
				  arg(m_archive->getSignalID(context->signalHash(0))).
				  arg(Archive::timeTypeStr(context->timeType())).
				  arg(start.toDateTime().toString("yyyy-MM-dd HH:mm:ss")).
				  arg(end.toDateTime().toString("yyyy-MM-dd HH:mm:ss")).
				  arg(context->signalCount()));*/
}

ArchRequest::~ArchRequest()
{
	DEBUG_LOG_MSG(m_logger, QString("ArchRequest ID = %1 is deleted").arg(m_param.requestID()));
}

void ArchRequest::run()
{
	prepareFiles();

	ArchFindResult findResult = findData();

	if (findResult == ArchFindResult::SearchError)
	{
		reportErrorAndWaitForQuit();
		return;
	}

	if (findResult == ArchFindResult::NotFound)
	{
		reportNoDataAndWaitForQuit();
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

	//
}

void ArchRequest::reportErrorAndWaitForQuit()
{
	waitForQuit();
}

void ArchRequest::reportNoDataAndWaitForQuit()
{
	waitForQuit();
}

void ArchRequest::waitForQuit()
{
	while(isQuitRequested() == false)
	{
		usleep(200);
	}
}

/*

// ---------------------------------------------------------------------------------------------
//
// FileArchRequestContext class implementattion
//
// ---------------------------------------------------------------------------------------------


FileArchRequestContext::FileArchRequestContext(const ArchRequestParam& param, const QTime& startTime, CircularLoggerShared logger) :
	ArchRequestContext(param, startTime, logger)
{
}

FileArchRequestContext::~FileArchRequestContext()
{
}

bool FileArchRequestContext::executeSatesRequest(ArchiveShared archive, QSqlDatabase* db)
{
	Q_UNUSED(db);

	ArchFile::FindResult result = archive->findData(m_param);

	switch(result)
	{
	case ArchFile::FindResult::Found:
		m_totalStates = 0;
		m_sentStates = 0;
		m_dataReady = false;

		DEBUG_LOG_MSG(m_logger, QString("RequestID %1: result %2 records").arg(m_param.requestID).arg(m_totalStates));

		m_reply.set_error(static_cast<int>(NetworkError::Success));
		m_reply.set_archerror(static_cast<int>(ArchiveError::Success));
		m_reply.set_requestid(m_param.requestID);
		m_reply.set_dataready(false);
		m_reply.set_totalstatescount(m_totalStates);
		m_reply.set_sentstatescount(m_sentStates);
		m_reply.set_statesinpartcount(0);
		m_reply.set_islastpart(false);
		m_reply.clear_appsignalstates();

		break;

	case ArchFile::FindResult::NotFound:
		m_totalStates = 0;
		m_sentStates = 0;
		m_dataReady = false;

		DEBUG_LOG_MSG(m_logger, QString("RequestID %1: result %2 records").arg(m_param.requestID).arg(m_totalStates));

		m_reply.set_error(static_cast<int>(NetworkError::Success));
		m_reply.set_archerror(static_cast<int>(ArchiveError::Success));
		m_reply.set_requestid(m_param.requestID);
		m_reply.set_dataready(true);
		m_reply.set_totalstatescount(m_totalStates);
		m_reply.set_sentstatescount(m_sentStates);
		m_reply.set_statesinpartcount(0);
		m_reply.set_islastpart(true);
		m_reply.clear_appsignalstates();

		break;

	default:
		return false;
	}

}

void FileArchRequestContext::getNextStates()
{
}


// ---------------------------------------------------------------------------------------------
//
// ArchRequestThreadWorker class implementattion
//
// ---------------------------------------------------------------------------------------------

ArchRequestThreadWorker::ArchRequestThreadWorker(ArchRequest* request, CircularLoggerShared& logger) :
	m_request(archive),
	m_logger(logger)
{
}

void ArchRequestThreadWorker::onThreadStarted()
{
	DEBUG_LOG_MSG(m_logger, QString("ArchRequestThreadWorker is started (requestID = %1").arg(m_request->requestID()));

	connect(m_request, &ArchRequest::getNextData, this, &ArchRequestThreadWorker::onGetNextData);

	findData();
}

void ArchRequestThreadWorker::onThreadFinished()
{
	DEBUG_LOG_MSG(m_logger, QString("ArchRequestThreadWorker is finished (requestID = %1").arg(m_request->requestID()));
}

void ArchRequestThreadWorker::onNewRequest(quint32 requestID)
{
	m_requestContextsMutex.lock();

	ArchRequestContextShared context = 	m_requestContexts.value(requestID, nullptr);

	m_requestContextsMutex.unlock();

	TEST_PTR_RETURN(context);

	DEBUG_LOG_MSG(m_logger, QString("-----------------------------------------------------------------"));
	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: start processing (elapsed %2)").arg(context->requestID()).arg(context->timeElapsed()));

	TimeStamp start(context->startTime());
	TimeStamp end(context->endTime());

	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: %2, time = %3, start = %4, end = %5, signals = %6").
				  arg(context->requestID()).
				  arg(m_archive->getSignalID(context->signalHash(0))).
				  arg(Archive::timeTypeStr(context->timeType())).
				  arg(start.toDateTime().toString("yyyy-MM-dd HH:mm:ss")).
				  arg(end.toDateTime().toString("yyyy-MM-dd HH:mm:ss")).
				  arg(context->signalCount()));

	context->checkSignalsHashes(m_archive);

	if (context->signalCount() == 0)
	{
		context->setArchError(ArchiveError::NoSignals);
		context->setDataReady(true);
		return;
	}

	result = context->executeSatesRequest(m_archive, &m_db);

	if (result == false)
	{
		context->setArchError(ArchiveError::ExecQueryError);
		context->setDataReady(true);
		return;
	}

	context->setArchError(ArchiveError::Success);

	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: query executed (elapsed %2)").arg(context->requestID()).arg(context->timeElapsed()))

	context->getNextStates();
}

void ArchRequestThreadWorker::onGetNextData(quint32 requestID)
{
	m_requestContextsMutex.lock();

	ArchRequestContextShared context = m_requestContexts.value(requestID, nullptr);

	m_requestContextsMutex.unlock();

	if (context == nullptr)
	{
		assert(false);
		return;
	}

	context->getNextStates();
}

void ArchRequestThreadWorker::onFinalizeRequest(quint32 requestID)
{
	AUTO_LOCK(m_requestContextsMutex);

	if (m_requestContexts.contains(requestID) == false)
	{
		assert(false);
		return;
	}

	m_requestContexts.remove(requestID);
}


// ---------------------------------------------------------------------------------------------
//
// ArchRequestThread class implementattion
//
// ---------------------------------------------------------------------------------------------

ArchRequestThread::ArchRequestThread(ArchiveShared archive, ArchWriterThread* fileArchWriter, CircularLoggerShared logger)
{
	m_worker = new ArchRequestThreadWorker(archive, fileArchWriter, logger);

	addWorker(m_worker);
}

ArchRequestContextShared ArchRequestThread::startNewRequest(ArchRequestParam& param, const QTime& startTime)
{
	return m_worker->startNewRequest(param, startTime);
}

void ArchRequestThread::finalizeRequest(quint32 requestID)
{
	return m_worker->finalizeRequest(requestID);
}

void ArchRequestThread::getNextData(ArchRequestContextShared context)
{
	m_worker->getNextData(context);
}


*/
