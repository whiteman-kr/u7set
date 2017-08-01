#include "Archive.h"

const char* Archive::ARCH_DB_PREFIX = "u7arch_";
const char* Archive::LONG_TERM_TABLE_PREFIX = "lt_";
const char* Archive::SHORT_TERM_TABLE_PREFIX = "st_";


Archive::Archive(CircularLoggerShared logger) :
	m_logger(logger)
{
}

void Archive::clear()
{
	m_projectID.clear();
	m_archSignals.clear();
	m_signalIDs.clear();
	m_existingTables.clear();
}

void Archive::initArchSignals(int count)
{
	m_archSignals.clear();
	m_signalIDs.clear();

	/*m_archSignals.reserve(static_cast<int>(count * 1.2));
	m_signalIDs.reserve(static_cast<int>(count * 1.2));*/
}

void Archive::appendArchSignal(const QString& appSignalID, const ArchSignal& archSignal)
{
	if (m_archSignals.contains(archSignal.hash) == true)
	{
		assert(false);
	}
	else
	{
		m_archSignals.insert(archSignal.hash, archSignal);
		m_signalIDs.insert(archSignal.hash, appSignalID);
	}
}

ArchSignal Archive::getArchSignal(Hash signalHash)
{
	return m_archSignals.value(signalHash, ArchSignal());
}

QString Archive::getSignalID(Hash signalHash)
{
	return m_signalIDs.value(signalHash, QString());
}

bool Archive::canReadWriteSignal(Hash signalHash)
{
	if (m_archSignals.contains(signalHash) == false)
	{
		return false;
	}

	return m_archSignals[signalHash].canReadWrite;
}

void Archive::setCanReadWriteSignal(Hash signalHash, bool canReadWrite)
{
	if (m_archSignals.contains(signalHash) == false)
	{
		assert(false);
		return;
	}

	m_archSignals[signalHash].canReadWrite = canReadWrite;
}

QString Archive::dbName()
{
	assert(m_projectID.isEmpty() == false);

	QString dbName = QString(ARCH_DB_PREFIX) + m_projectID;

	dbName = dbName.toLower();

	return dbName;
}

QString Archive::getTableName(Hash signalHash, Archive::TableType tableType)
{
	QString tableName;

	switch(tableType)
	{
	case TableType::LongTerm:
		tableName = LONG_TERM_TABLE_PREFIX;
		break;

	case TableType::ShortTerm:
		tableName = SHORT_TERM_TABLE_PREFIX;
		break;

	default:
		assert(false);
	}

	if (tableName.isEmpty() == true)
	{
		return tableName;
	}

	tableName += QString().setNum(signalHash, 16).rightJustified(sizeof(qint64) * 2, '0', false);

	return tableName;
}

void Archive::appendExistingTable(const QString& tableName)
{
	if (m_existingTables.contains(tableName) == true)
	{
		assert(false);
		return;
	}

	m_existingTables.insert(tableName, tableName);
}

bool Archive::tableIsExists(const QString& tableName)
{
	return m_existingTables.contains(tableName);
}


QString Archive::timeTypeStr(TimeType timeType)
{
	switch(timeType)
	{
	case TimeType::Plant:
		return QString("Plant");

	case TimeType::System:
		return QString("System");

	case TimeType::Local:
		return QString("Local");

	case TimeType::ArchiveId:
		return QString("ArchiveId");

	default:
		assert(false);
	}

	return QString("???");
}




