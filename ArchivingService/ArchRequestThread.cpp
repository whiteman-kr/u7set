#include "ArchRequestThread.h"


ArchRequestThreadWorker::ArchRequestThreadWorker(const QString& projectID, CircularLoggerShared& logger) :
	m_projectID(projectID),
	m_logger(logger)
{
}

void ArchRequestThreadWorker::onThreadStarted()
{
	DEBUG_LOG_MSG(m_logger, "ArchRequestThreadWorker is started");
}

void ArchRequestThreadWorker::onThreadFinished()
{
	DEBUG_LOG_MSG(m_logger, "ArchRequestThreadWorker is finished");
}



ArchRequestThread::ArchRequestThread(const QString& projectID, CircularLoggerShared& logger)
{
	ArchRequestThreadWorker* worker = new ArchRequestThreadWorker(projectID, logger);

	addWorker(worker);
}
