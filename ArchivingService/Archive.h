#pragma once

#include <QHash>

#include "../lib/Hash.h"
#include "../lib/TimeStamp.h"
#include "../lib/CircularLogger.h"

struct ArchSignal
{
	// no append hard fields in this struct (like QString) !

	Hash hash = 0;
	bool isAnalog = false;
	bool isInitialized = false;

	//

	bool canReadWrite = false;
};

class Archive
{
public:
	enum TableType
	{
		LongTerm,
		ShortTerm
	};

public:
	Archive(CircularLoggerShared logger);

	void clear();
	void setProject(const QString& projectID) { m_projectID = projectID; }

	void initArchSignals(int count);
	void appendArchSignal(const QString& appSignalID, const ArchSignal& archSignal);
	ArchSignal getArchSignal(Hash signalHash);
	QString getSignalID(Hash signalHash);

	bool canReadWriteSignal(Hash signalHash);
	void setCanReadWriteSignal(Hash signalHash, bool canWrite);

	QString dbName();
	QString getTableName(Hash signalHash, Archive::TableType tableType);

	const QHash<Hash, ArchSignal>& archSignals() const { return m_archSignals; }

	void appendExistingTable(const QString& tableName);
	bool tableIsExists(const QString& tableName);

	static QString timeTypeStr(TimeType timeType);

	static qint64 localTimeOffsetFromUtc();

	void setSignalInitialized(Hash signalHash, bool initilaized);

private:
	static const char* ARCH_DB_PREFIX;
	static const char* LONG_TERM_TABLE_PREFIX;
	static const char* SHORT_TERM_TABLE_PREFIX;


private:
	CircularLoggerShared m_logger;

	QString m_projectID;

	QHash<Hash, ArchSignal> m_archSignals;

	QHash<QString, QString> m_existingTables;

	QHash<Hash, QString> m_signalIDs;
};
