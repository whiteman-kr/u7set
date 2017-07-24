#include "../lib/WUtils.h"

#include "ArchWriteThread.h"
#include "ArchRequestThread.h"


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

bool ArchRequestContext::createQuery(QSqlDatabase& db, const QString& queryStr)
{
	assert(db.isOpen() == true);
	assert(m_query == nullptr);

	m_query  = new QSqlQuery(db);

	m_queryStr = queryStr;

	return true;
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
	else
	{
		int t = time.elapsed();

		DEBUG_LOG_MSG(logger, QString("Query execution time = %1").arg(t));
	}

	return result;
}

const char* ArchRequestThreadWorker::FIELD_PLANT_TIME = "plantTime";
const char* ArchRequestThreadWorker::FIELD_SYSTEM_TIME = "sysTime";
const char* ArchRequestThreadWorker::FIELD_LOCAL_TIME = "locTime";
const char* ArchRequestThreadWorker::FIELD_ARCH_ID = "archID";
const char* ArchRequestThreadWorker::FIELD_VALUE = "val";
const char* ArchRequestThreadWorker::FIELD_FLAGS = "flags";

ArchRequestThreadWorker::ArchRequestThreadWorker(const QString& projectID, const QHash<Hash, bool>& archSignals, CircularLoggerShared& logger) :
	m_projectID(projectID),
	m_archSignals(archSignals),
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

void ArchRequestThreadWorker::onThreadStarted()
{
	connect(this, &ArchRequestThreadWorker::newRequest, this, &ArchRequestThreadWorker::onNewRequest);

	DEBUG_LOG_MSG(m_logger, QString("ArchRequestThreadWorker is started"));

	Qt::HANDLE h = QThread::currentThreadId();

	m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL", "readArchConnection"));

	tryConnectToDatabase();

	// CODE_FOR_DEBUG

//	ArchRequestParam param;

//	param.timeType = TimeType::ArchiveId;

//	param.startTime = 39;
//	param.endTime = 129500;

//	param.signalHashesCount = 2;

//	param.signalHashes[0] = 0xf8214123bc1e4f9d;
//	param.signalHashes[1] = 0xf8214a17eb8566a4;

//	startNewRequest(param);

	// CODE_FOR_DEBUG
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
	m_db->setDatabaseName("u7arch_" + m_projectID);
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

	bool result = tryConnectToDatabase();

	if (result == false)
	{
		context->setArchError(ArchiveError::DbConnectionError);
		context->setDataReady(true);
		return;
	}

	QString queryStr;

	result = createQueryStr(context, queryStr);

	if (result == false)
	{
		return;
	}

	result = context->createQuery(*m_db, queryStr);

	if (result == false)
	{
		return;
	}

	result = context->executeQuery(m_logger);

	if (result == false)
	{
		context->setArchError(ArchiveError::ExecQueryError);
		context->setDataReady(true);
	}
	else
	{
		context->setArchError(ArchiveError::Success);
		context->setDataReady(true);
	}
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

	for(int i = 0; i < signalHashesCount; i++)
	{
		Hash signalHash = signalHashes[i];

		if (m_archSignals.contains(signalHash) == false)
		{
			assert(false);
			continue;
		}

		bool isAnalog = m_archSignals.contains(signalHash);

		for(int i = 0; i < (isAnalog == true ? 2 : 1); i++)
		{
			QString tableName;

			if (i == 0)
			{
				// get long term table name
				//
				tableName = ArchWriteThreadWorker::getTableName(signalHash, ArchWriteThreadWorker::SignalStatesTableType::LongTerm);
			}
			else
			{
				// get short term table name
				//
				tableName = ArchWriteThreadWorker::getTableName(signalHash, ArchWriteThreadWorker::SignalStatesTableType::ShortTerm);
			}

			QString formatStr;

			if (count == 0)
			{
				formatStr = QString("SELECT %1, %2, %3, %4, %5, %6 FROM %7 WHERE %8 >= %9 AND %8 <= %10 ");
			}
			else
			{
				formatStr = QString("UNION DISTINCT SELECT %1, %2, %3, %4, %5, %6 FROM %7 WHERE %8 >= %9 AND %8 <= %10 ");
			}

			queryStr.append(QString(formatStr).
							arg(FIELD_ARCH_ID).
							arg(FIELD_PLANT_TIME).
							arg(FIELD_SYSTEM_TIME).
							arg(FIELD_LOCAL_TIME).
							arg(FIELD_VALUE).
							arg(FIELD_FLAGS).
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

//

ArchRequestThread::ArchRequestThread(const QString& projectID, const QHash<Hash, bool>& archSignals, CircularLoggerShared& logger)
{
	m_worker = new ArchRequestThreadWorker(projectID, archSignals, logger);

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

