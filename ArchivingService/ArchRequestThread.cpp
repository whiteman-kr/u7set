#include "../lib/WUtils.h"

#include "ArchWriteThread.h"
#include "ArchRequestThread.h"


ArchRequestParam::ArchRequestParam()
{
	clearSignalHashes();
}

void ArchRequestParam::clearSignalHashes()
{
	memset(signalHashes, 0, sizeof(signalHashes));
	signalHashesCount = 0;
}

//

ArchRequestContext::ArchRequestContext(const ArchRequestParam& param, const QTime& startTime, CircularLoggerShared logger) :
	m_param(param),
	m_time(startTime),
	m_logger(logger)
{
	m_localTimeOffset = Archive::localTimeOffsetFromUtc();

	m_requestTimeType = param.timeType;

	switch(m_requestTimeType)
	{
	case TimeType::Plant:
	case TimeType::System:
	case TimeType::ArchiveId:

		m_requestStartTime = param.startTime;
		m_requestEndTime = param.endTime;

		break;

	case TimeType::Local:
		{
			// convert local time to system time
			//
			m_requestStartTime = param.startTime - m_localTimeOffset;
			m_requestEndTime = param.endTime - m_localTimeOffset;

			m_requestTimeType = TimeType::System;
		}
		break;

	default:
		assert(false);
	}

	if (m_requestTimeType != TimeType::ArchiveId)
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

	m_cmpField = Archive::getCmpField(m_requestTimeType);
}

ArchRequestContext::~ArchRequestContext()
{
	if (m_statesQuery != nullptr)
	{
		delete m_statesQuery;
		m_statesQuery = nullptr;
	}
}

void ArchRequestContext::checkSignalsHashes(const Archive& arch)
{
	QVector<Hash> existingHashes;

	const QHash<Hash, ArchSignal>& archSignals = arch.archSignals();

	for(int i = 0; i < m_param.signalHashesCount; i++)
	{
		Hash signalHash = m_param.signalHashes[i];

		if (archSignals.contains(signalHash) == true)
		{
			existingHashes.append(signalHash);
		}
	}

	m_param.clearSignalHashes();

	for(int i = 0; i < existingHashes.count(); i++)
	{
		m_param.signalHashes[i] = existingHashes[i];
	}

	m_param.signalHashesCount = existingHashes.count();
}

Hash ArchRequestContext::signalHash(int index)
{
	if (index < 0 || index >= ARCH_REQUEST_MAX_SIGNALS)
	{
		assert(false);
		return -1;
	}

	return m_param.signalHashes[index];
}

bool ArchRequestContext::createGetSignalStatesQueryStr(const Archive& archive)
{

//SELECT archid FROM timemarks WHERE systime < 1501754760132 ORDER BY systime DESC LIMIT 1;

	m_statesQueryStr.clear();

	int signalHashesCount = m_param.signalHashesCount;

	const QHash<Hash, ArchSignal>& archSignals = archive.archSignals();

	QString formatStr0 = QString("SELECT %1, %2, %3, %4, %5, %6::bigint AS hash FROM %7 WHERE %8 >= %9::bigint AND %8 <= %10::bigint ");

	QString formatStrOthers = QString("UNION ALL SELECT %1, %2, %3, %4, %5, %6::bigint AS hash FROM %7 WHERE %8 >= %9::bigint AND %8 <= %10::bigint ");

	int count = 0;

	for(int i = 0; i < signalHashesCount; i++)
	{
		Hash signalHash = m_param.signalHashes[i];

		if (archSignals.contains(signalHash) == false)
		{
			DEBUG_LOG_ERR(m_logger, QString("Unknown signal hash %1 in archive request").arg(signalHash));
			continue;
		}

		QString tableName = archive.getTableName(signalHash);

		QString& formatStr = formatStr0;

		if (count > 0)
		{
			formatStr = formatStrOthers;
		}

		qint64 signedSignalHash = *reinterpret_cast<qint64*>(&signalHash);

//		m_statesQueryStr.append(QString(formatStr).
//										arg(Archive::FIELD_ARCH_ID).
//										arg(Archive::FIELD_PLANT_TIME).
//										arg(Archive::FIELD_SYSTEM_TIME).
//										arg(Archive::FIELD_VALUE).
//										arg(Archive::FIELD_FLAGS).
//										arg(signedSignalHash).
//										arg(tableName).
//										arg(m_cmpField).
//										arg(m_requestStartTime).
//										arg(m_requestEndTime));

		m_statesQueryStr.append(QString(formatStr).
										arg(Archive::FIELD_ARCH_ID).
										arg(Archive::FIELD_PLANT_TIME).
										arg(Archive::FIELD_SYSTEM_TIME).
										arg(Archive::FIELD_VALUE).
										arg(Archive::FIELD_FLAGS).
										arg(signedSignalHash).
										arg(tableName).
										arg(Archive::FIELD_ARCH_ID).
										arg(m_startArchID).
										arg(m_endArchID));

		count++;
	}

	if (m_statesQueryStr.isEmpty() == false)
	{
		if (signalHashesCount == 1)
		{
			m_statesQueryStr.append(QString("ORDER BY %1").arg(Archive::FIELD_ARCH_ID));
		}
		else
		{
			m_statesQueryStr.append(QString("ORDER BY %1").arg(m_cmpField));
		}
	}

	return true;
}

bool ArchRequestContext::executeQuery(const Archive& archive, QSqlDatabase& db, CircularLoggerShared& logger)
{
	TEST_PTR_RETURN_FALSE(logger);

	assert(db.isOpen() == true);
	assert(m_statesQuery == nullptr);

	bool result = initArchId(db, logger);

	if (result == false)
	{
		DEBUG_LOG_ERR(logger, "initArchId() error");
		return false;
	}

	DEBUG_LOG_ERR(logger, QString("RequestID %1: StartArchID = %2, EndArchID = %3").
									arg(requestID()).
									arg(m_startArchID).
									arg(m_endArchID));

	result = createGetSignalStatesQueryStr(archive);

	if (result == false)
	{
		return false;
	}

	if (m_statesQueryStr.isEmpty() == true)
	{
		assert(false);
		return false;
	}

	m_statesQuery = new QSqlQuery(db);

	m_statesQuery->setForwardOnly(true);

	DEBUG_LOG_MSG(logger, QString("Execute: %1").arg(m_statesQueryStr));

	result = m_statesQuery->exec(m_statesQueryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(logger, m_statesQuery->lastError().text());
		db.close();
		return false;
	}

	m_totalStates = m_statesQuery->size();
	m_sentStates = 0;
	m_dataReady = false;

	m_reply.set_error(static_cast<int>(NetworkError::Success));
	m_reply.set_archerror(static_cast<int>(ArchiveError::Success));
	m_reply.set_requestid(m_param.requestID);
	m_reply.set_dataready(false);
	m_reply.set_totalstatescount(m_totalStates);
	m_reply.set_sentstatescount(m_sentStates);
	m_reply.set_statesinpartcount(0);
	m_reply.set_islastpart(false);

	m_reply.clear_appsignalstates();

	DEBUG_LOG_MSG(logger, QString("RequestID %1: result %2 records").arg(m_param.requestID).arg(m_totalStates));

	return result;
}

bool ArchRequestContext::initArchId(QSqlDatabase& db, CircularLoggerShared& logger)
{
	if (m_requestTimeType == TimeType::ArchiveId)
	{
		m_startArchID = m_requestStartTime;
		m_endArchID = m_requestEndTime;

		return true;
	}

	m_startArchID = 0;
	m_endArchID = std::numeric_limits<qint64>::max();

	QSqlQuery q(db);

	// ------------------------- init m_startArchID -----------------------------------
	//
	QString getStartQueryStr = QString("SELECT MAX(%1) FROM timemarks WHERE %2 < %3::bigint").
											arg(Archive::FIELD_ARCH_ID).
											arg(m_cmpField).
											arg(m_expandedRequestStartTime);

	bool result = q.exec(getStartQueryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(logger, m_statesQuery->lastError().text());
		db.close();
		return false;
	}

	if (q.next() == true && q.value(0).isNull() == false)
	{
		m_startArchID = q.value(0).toULongLong();
	}
	else
	{
		getStartQueryStr = QString("SELECT MIN(%1) FROM timemarks").
											arg(Archive::FIELD_ARCH_ID);

		result = q.exec(getStartQueryStr);

		if (result == false)
		{
			DEBUG_LOG_ERR(logger, m_statesQuery->lastError().text());
			db.close();
			return false;
		}

		if (q.next() == true && q.value(0).isNull() == false)
		{
			m_startArchID = q.value(0).toULongLong();
		}
	}

	// ------------------------- init m_startArchID -----------------------------------
	//
	QString getEndQueryStr = QString("SELECT MIN(%1) FROM timemarks WHERE %2 > %3::bigint").
											arg(Archive::FIELD_ARCH_ID).
											arg(m_cmpField).
											arg(m_expandedRequestEndTime);
	result = q.exec(getEndQueryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(logger, m_statesQuery->lastError().text());
		db.close();
		return false;
	}

	if (q.next() == true && q.value(0).isNull() == false)
	{
		m_endArchID = q.value(0).toULongLong();
	}
/*	else
	{		getEndQueryStr = QString("SELECT MAX(%1) FROM timemarks").
											arg(Archive::FIELD_ARCH_ID);
		result = q.exec(getEndQueryStr);

		if (result == false)
		{
			DEBUG_LOG_ERR(logger, m_statesQuery->lastError().text());
			db.close();
			return false;
		}

		if (q.next() == true && q.value(0).isNull() == false)
		{
			m_endArchID = q.value(0).toULongLong();
		}
	} */

	return true;
}

void ArchRequestContext::getNextData()
{
	TEST_PTR_RETURN(m_statesQuery);

	int count = 0;

	m_reply.clear_appsignalstates();

	bool hasNextRecord = false;

	int skipRecords = 0;

	do
	{
		hasNextRecord = m_statesQuery->next();

		if (hasNextRecord == false)
		{
			break;
		}

		qint64 archid = m_statesQuery->value(0).toLongLong();
		qint64 plantTime = m_statesQuery->value(1).toLongLong();
		qint64 systemTime = m_statesQuery->value(2).toLongLong();

		bool skipRecord = false;

		switch(m_requestTimeType)
		{
		case TimeType::ArchiveId:
			if (archid < m_requestStartTime || archid > m_requestEndTime)
			{
				skipRecord = true;
			}
			break;

		case TimeType::Plant:
			if (plantTime < m_requestStartTime || plantTime > m_requestEndTime)
			{
				skipRecord = true;
			}
			break;

		case TimeType::System:
			if (systemTime < m_requestStartTime || systemTime > m_requestEndTime)
			{
				skipRecord = true;
			}
			break;

		default:
			// no other times can't be in m_requestTimeType!
			assert(false);
		}

		if (skipRecord == true)
		{
			skipRecords++;
			continue;
		}

		qint64 localTime = systemTime + m_localTimeOffset;

		double value = m_statesQuery->value(3).toDouble();
		qint32 flags = m_statesQuery->value(4).toInt();
		quint64 hash = m_statesQuery->value(5).toULongLong();

		Proto::AppSignalState* state = m_reply.add_appsignalstates();

		state->set_archiveid(archid);
		state->set_planttime(plantTime);
		state->set_systemtime(systemTime);
		state->set_localtime(localTime);
		state->set_value(value);
		state->set_flags(flags);
		state->set_hash(hash);

		count++;
	}
	while(count < ARCH_REQUEST_MAX_STATES);

	m_reply.set_sentstatescount(m_reply.sentstatescount() + count);
	m_reply.set_statesinpartcount(count);
	m_reply.set_dataready(true);

	if (hasNextRecord == true)
	{
		m_reply.set_islastpart(false);
	}
	else
	{
		m_reply.set_islastpart(true);

		delete m_statesQuery;
		m_statesQuery = nullptr;
	}

	m_dataReady = true;

	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: next data read Ok, skiped %2 (elapsed %3) ").
				  arg(requestID()).
				  arg(skipRecords).
				  arg(timeElapsed()))
}

ArchRequestThreadWorker::ArchRequestThreadWorker(Archive& archive, CircularLoggerShared& logger) :
	m_archive(archive),
	m_logger(logger)
{
	qRegisterMetaType<ArchRequestContextShared>("ArchRequestContextShared");
}

ArchRequestContextShared ArchRequestThreadWorker::startNewRequest(ArchRequestParam &param, const QTime& startTime)
{
	// function should be called in context of param.archRequestServer thread!
	//
	AUTO_LOCK(m_requestContextsMutex);

	if (param.signalHashesCount == 0)
	{
		assert(false);
		return nullptr;
	}

	if (m_requestContexts.contains(param.requestID) == true)
	{
		assert(false);
		return nullptr;
	}

	ArchRequestContextShared context = std::make_shared<ArchRequestContext>(param, startTime, m_logger);

	context->setDataReady(false);

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

void ArchRequestThreadWorker::getNextData(ArchRequestContextShared context)
{
	TEST_PTR_RETURN(context);

	context->setDataReady(false);

	emit getNextDataSignal(context->requestID());
}

void ArchRequestThreadWorker::onThreadStarted()
{
	connect(this, &ArchRequestThreadWorker::newRequest, this, &ArchRequestThreadWorker::onNewRequest);
	connect(this, &ArchRequestThreadWorker::getNextDataSignal, this, &ArchRequestThreadWorker::onGetNextData);

	DEBUG_LOG_MSG(m_logger, QString("ArchRequestThreadWorker is started"));

	m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL", "readArchConnection"));

	tryConnectToDatabase();
}

void ArchRequestThreadWorker::onThreadFinished()
{
	DEBUG_LOG_MSG(m_logger, "ArchRequestThreadWorker is finished");
}

bool ArchRequestThreadWorker::tryConnectToDatabase()
{
	TEST_PTR_RETURN_FALSE(m_db);

	if (m_db->isOpen() == true)
	{
		return true;
	}

	m_db->setHostName("127.0.0.1");
	m_db->setPort(5432);
	m_db->setDatabaseName(m_archive.dbName());
	m_db->setUserName("u7arch");
	m_db->setPassword("arch876436");

	bool result = m_db->open();

	if (result == false)
	{
		DEBUG_LOG_ERR(m_logger, m_db->lastError().text());
		return false;
	}

	return result;
}


void ArchRequestThreadWorker::onNewRequest(ArchRequestContextShared context)
{
	// execute request to archive DB
	//
	TEST_PTR_RETURN(context);

	DEBUG_LOG_MSG(m_logger, QString("-----------------------------------------------------------------"));
	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: start processing (elapsed %2)").arg(context->requestID()).arg(context->timeElapsed()));

	TimeStamp start(context->startTime());
	TimeStamp end(context->endTime());

	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: %2, time = %3, start = %4, end = %5, signals = %6").
				  arg(context->requestID()).
				  arg(m_archive.getSignalID(context->signalHash(0))).
				  arg(Archive::timeTypeStr(context->timeType())).
				  arg(start.toDateTime().toString("yyyy-MM-dd HH:mm:ss")).
				  arg(end.toDateTime().toString("yyyy-MM-dd HH:mm:ss")).
				  arg(context->signalCount()));

	bool result = tryConnectToDatabase();

	if (result == false)
	{
		context->setArchError(ArchiveError::DbConnectionError);
		context->setDataReady(true);
		return;
	}

	context->checkSignalsHashes(m_archive);

	if (context->signalCount() == 0)
	{
		context->setArchError(ArchiveError::NoSignals);
		context->setDataReady(true);
		return;
	}

	result = context->executeQuery(m_archive, *m_db, m_logger);

	if (result == false)
	{
		context->setArchError(ArchiveError::ExecQueryError);
		context->setDataReady(true);
		return;
	}

	context->setArchError(ArchiveError::Success);

	DEBUG_LOG_MSG(m_logger, QString("RequestID %1: query executed (elapsed %2)").arg(context->requestID()).arg(context->timeElapsed()))

	context->getNextData();
}

void ArchRequestThreadWorker::onGetNextData(quint32 requestID)
{
	ArchRequestContextShared context = m_requestContexts.value(requestID, ArchRequestContextShared());

	if (context == nullptr)
	{
		assert(false);
		return;
	}

	context->getNextData();
}

//

ArchRequestThread::ArchRequestThread(Archive& archive, CircularLoggerShared& logger)
{
	m_worker = new ArchRequestThreadWorker(archive, logger);

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


