#include "../lib/WUtils.h"
#include "ArchRequestThread.h"



ArchRequestContext::ArchRequestContext(const ArchRequestParam& param) :
	m_param(param)
{

}

ArchRequestThreadWorker::ArchRequestThreadWorker(const QString& projectID, CircularLoggerShared& logger) :
	m_projectID(projectID),
	m_logger(logger)
{
}

ArchRequestContextShared ArchRequestThreadWorker::startNewRequest(ArchRequestParam &param)
{
	// function should be called in context of param.archRequestServer thread!
	//
	AUTO_LOCK(m_requestContextsMutex);

	if (param.archRequestServer == nullptr ||
		param.hashesCount == 0)
	{
		assert(false);
		return nullptr;
	}

	if (m_requestContexts.contains(param.requestID) == true)
	{
		assert(false);
		return nullptr;
	}

	ArchRequestContextShared context = std::make_shared<ArchRequestContext>(param);

	m_requestContexts.insert(param.requestID, context);

	emit newRequest(context);

	return context;
}

void ArchRequestThreadWorker::finalizeRequest(quint32 requestID)
{
	// function should be called in context of param.archRequestServer thread!
	//
	AUTO_LOCK(m_requestContextsMutex);

	if (m_requestContexts.contains(requestID) == false)
	{
		assert(false);
		return;
	}

	m_requestContexts.remove(requestID);
}

void ArchRequestThreadWorker::onThreadStarted()
{
	connect(this, &ArchRequestThreadWorker::newRequest, this, &ArchRequestThreadWorker::onNewRequest);

	DEBUG_LOG_MSG(m_logger, "ArchRequestThreadWorker is started");
}

void ArchRequestThreadWorker::onThreadFinished()
{
	DEBUG_LOG_MSG(m_logger, "ArchRequestThreadWorker is finished");
}

void ArchRequestThreadWorker::onNewRequest(ArchRequestContextShared context)
{
	// execute request to archive DB
}

//

ArchRequestThread::ArchRequestThread(const QString& projectID, CircularLoggerShared& logger)
{
	m_worker = new ArchRequestThreadWorker(projectID, logger);

	addWorker(m_worker);
}

ArchRequestContextShared ArchRequestThread::startNewRequest(ArchRequestParam& param)
{
	return m_worker->startNewRequest(param);
}

void ArchRequestThread::finalizeRequest(quint32 requestID)
{
	return m_worker->finalizeRequest(requestID);
}

