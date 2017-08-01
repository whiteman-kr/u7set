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

ArchRequestContext::ArchRequestContext(const ArchRequestParam& param) :
	m_param(param)
{
}

ArchRequestContext::~ArchRequestContext()
{
	if (m_query != nullptr)
	{
		delete m_query;
		m_query = nullptr;
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

void ArchRequestContext::createQuery(QSqlDatabase& db, const QString& queryStr)
{
	assert(db.isOpen() == true);
	assert(m_query == nullptr);

	m_query  = new QSqlQuery(db);

	m_queryStr = queryStr;
}

bool ArchRequestContext::executeQuery(CircularLoggerShared& logger)
{
	TEST_PTR_RETURN_FALSE(logger);
	TEST_PTR_RETURN_FALSE(m_query);

	if (m_queryStr.isEmpty() == true)
	{
		assert(false);
		return false;
	}

	QTime time;

	time.start();

	bool result = m_query->exec(m_queryStr);

	if (result == false)
	{
		DEBUG_LOG_ERR(logger, m_query->lastError().text());
		return false;
	}

	int t = time.elapsed();

	DEBUG_LOG_MSG(logger, QString("Query execution time = %1").arg(t));

	m_totalStates = m_query->size();
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

	return result;
}

void ArchRequestContext::getNextData()
{
	TEST_PTR_RETURN(m_query);

//	assert(m_query->isValid() == true);

	int count = 0;

	m_reply.clear_appsignalstates();

	bool hasNextRecord = false;

	do
	{
		hasNextRecord = m_query->next();

		if (hasNextRecord == false)
		{
			break;
		}

		qint64 archid = m_query->value(0).toLongLong();
		qint64 plantTime = m_query->value(1).toLongLong();
		qint64 systemTime = m_query->value(2).toLongLong();
		qint64 localTime = m_query->value(3).toLongLong();
		double value = m_query->value(4).toDouble();
		qint32 flags = m_query->value(5).toInt();
		quint64 hash = m_query->value(6).toULongLong();

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
	m_reply.set_islastpart(!hasNextRecord);

	m_dataReady = true;
}

const char* ArchRequestThreadWorker::FIELD_PLANT_TIME = "plantTime";
const char* ArchRequestThreadWorker::FIELD_SYSTEM_TIME = "sysTime";
const char* ArchRequestThreadWorker::FIELD_LOCAL_TIME = "locTime";
const char* ArchRequestThreadWorker::FIELD_ARCH_ID = "archID";
const char* ArchRequestThreadWorker::FIELD_VALUE = "val";
const char* ArchRequestThreadWorker::FIELD_FLAGS = "flags";

ArchRequestThreadWorker::ArchRequestThreadWorker(Archive& archive, CircularLoggerShared& logger) :
	m_archive(archive),
	m_logger(logger)
{
	qRegisterMetaType<ArchRequestContextShared>("ArchRequestContextShared");
}

ArchRequestContextShared ArchRequestThreadWorker::startNewRequest(ArchRequestParam &param)
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

	ArchRequestContextShared context = std::make_shared<ArchRequestContext>(param);

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

bool ArchRequestThreadWorker::createQueryStr(ArchRequestContextShared context, QString& queryStr)
{
	TEST_PTR_RETURN_FALSE(context);

	QString cmpField = getCmpField(context->timeType());

	if (cmpField.isEmpty() == true)
	{
		return false;
	}

	int signalHashesCount = context->signalHashesCount();
	const Hash* signalHashes = context->signalHashes();

	int count = 0;

	const QHash<Hash, ArchSignal>& archSignals = m_archive.archSignals();

	for(int i = 0; i < signalHashesCount; i++)
	{
		Hash signalHash = signalHashes[i];

		if (archSignals.contains(signalHash) == false)
		{
			DEBUG_LOG_ERR(m_logger, QString("Unknown signal hash %1 in archive request").arg(signalHash));
			continue;
		}

		const ArchSignal& archSignal = archSignals.value(signalHash);

		for(int i = 0; i < (archSignal.isAnalog == true ? 2 : 1); i++)
		{
			QString tableName;

			if (i == 0)
			{
				// get long term table name
				//
				tableName = m_archive.getTableName(signalHash, Archive::TableType::LongTerm);
			}
			else
			{
				// get short term table name
				//
				tableName = m_archive.getTableName(signalHash, Archive::TableType::ShortTerm);
			}

			QString formatStr;

			if (count == 0)
			{
				formatStr = QString("SELECT %1, %2, %3, %4, %5, %6, %7::bigint AS hash FROM %8 WHERE %9 >= %10 AND %9 <= %11 ");
			}
			else
			{
				formatStr = QString("UNION DISTINCT SELECT %1, %2, %3, %4, %5, %6, %7::bigint AS hash FROM %8 WHERE %9 >= %10 AND %9 <= %11 ");
			}

			qint64 signedSignalHash = *reinterpret_cast<qint64*>(&signalHash);

			queryStr.append(QString(formatStr).
							arg(FIELD_ARCH_ID).
							arg(FIELD_PLANT_TIME).
							arg(FIELD_SYSTEM_TIME).
							arg(FIELD_LOCAL_TIME).
							arg(FIELD_VALUE).
							arg(FIELD_FLAGS).
							arg(signedSignalHash).
							arg(tableName).
							arg(cmpField).
							arg(context->startTime()).
							arg(context->endTime()));

			count++;
		}
	}

	return true;
}

QString ArchRequestThreadWorker::getCmpField(TimeType timeType)
{
	QString cmpField;

	switch(timeType)
	{
	case TimeType::Plant:
		cmpField = FIELD_PLANT_TIME;
		break;

	case TimeType::System:
		cmpField = FIELD_SYSTEM_TIME;
		break;

	case TimeType::Local:
		cmpField = FIELD_LOCAL_TIME;
		break;

	case TimeType::ArchiveId:
		cmpField = FIELD_ARCH_ID;
		break;

	default:
		assert(false);
	}

	return cmpField;
}

void ArchRequestThreadWorker::onNewRequest(ArchRequestContextShared context)
{
	// execute request to archive DB
	//
	TEST_PTR_RETURN(context);

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

	QString queryStr;

	result = createQueryStr(context, queryStr);

	if (result == false)
	{
		context->setArchError(ArchiveError::BuildQueryError);
		context->setDataReady(true);
		return;
	}

	context->createQuery(*m_db, queryStr);

	result = context->executeQuery(m_logger);

	if (result == false)
	{
		context->setArchError(ArchiveError::ExecQueryError);
		context->setDataReady(true);
		return;
	}

	context->setArchError(ArchiveError::Success);

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

ArchRequestContextShared ArchRequestThread::startNewRequest(ArchRequestParam& param)
{
	return m_worker->startNewRequest(param);
}

void ArchRequestThread::finalizeRequest(quint32 requestID)
{
	return m_worker->finalizeRequest(requestID);
}


void ArchRequestThread::getNextData(ArchRequestContextShared context)
{
	m_worker->getNextData(context);
}


