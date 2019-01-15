#include "../lib/WUtils.h"

#include "ArchRequest.h"
#include "ArchWriterThread.h"
#include "FileArchReader.h"

// ---------------------------------------------------------------------------------------------
//
// ArchRequestContext class implementattion
//
// ---------------------------------------------------------------------------------------------

ArchRequest::ArchRequest(const ArchRequestParam& param, const QTime& startTime, CircularLoggerShared logger) :
	m_param(param),
	m_time(startTime),
	m_logger(logger)
{
	m_localTimeOffset = Archive::localTimeOffsetFromUtc();

	m_requestTimeType = param.timeType;

	switch(m_requestTimeType)
	{
	case E::TimeType::Plant:
	case E::TimeType::System:
	case E::TimeType::ArchiveId:

		m_requestStartTime = param.startTime;
		m_requestEndTime = param.endTime;

		break;

	case E::TimeType::Local:
		{
			// convert local time to system time
			//
			m_requestStartTime = param.startTime - m_localTimeOffset;
			m_requestEndTime = param.endTime - m_localTimeOffset;

			m_requestTimeType = E::TimeType::System;
		}
		break;

	default:
		assert(false);
	}

	if (m_requestTimeType != E::TimeType::ArchiveId)
	{
		// expand request time from both sides
		//
		m_expandedRequestStartTime = m_requestStartTime - Archive::TIME_TO_EXPAND_REQUEST;

		if (m_expandedRequestStartTime < 0)
		{
			m_expandedRequestStartTime = 0;
		}

		m_expandedRequestEndTime = m_requestEndTime + Archive::TIME_TO_EXPAND_REQUEST;
	}
}

ArchRequest::~ArchRequest()
{
}

void ArchRequest::startRequestProcessing()
{
	m_requestThread = new SimpleThread(new ArchRequestThreadWorker(this, m_logger));

	m_requestThread->start();
}

void ArchRequest::stopRequestProcessing()
{
	if (m_requestThread != nullptr)
	{
		m_requestThread->quitAndWait();
	}
	else
	{
		assert(false);
	}
}

ArchFindResult ArchRequest::findData()
{
	if (m_requestContexts.contains(param.requestID) == true)
	{
		assert(false);
		return ArchFindResult::SearchError;
	}

	RequestContext* reqContext = new RequestContext(param);

	m_requestContexts.insert(param.requestID, reqContext);

	// enqueue files for immediately flushing
	//
	for(Hash signalHash : param.signalHashes)
	{
		ArchFile* archFile = m_archFiles.value(signalHash, nullptr);

		if (archFile == nullptr)
		{
			assert(false);
			continue;
		}

		reqContext->appendArchFile(archFile);

		flushImmediately(archFile);
	}

	ArchFindResult result = reqContext->findData();

	if (result == ArchFindResult::NotFound)
	{
		m_requestContexts.remove(reqContext->requestID());
		delete reqContext;
	}

	return result;
}



Hash ArchRequest::signalHash(int index)
{
	if (index < 0 || index >= m_param.signalHashes.count())
	{
		assert(false);
		return UNDEFINED_HASH;
	}

	return m_param.signalHashes[index];
}

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


