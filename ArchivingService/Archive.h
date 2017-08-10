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

	static const int TIME_1S = 1000;								// 1000 millisecond
	static const int TIME_TO_EXPAND_REQUEST = 65 * TIME_1S;			// 65 seconds

	static const char* FIELD_PLANT_TIME;
	static const char* FIELD_SYSTEM_TIME;
	static const char* FIELD_ARCH_ID;
	static const char* FIELD_VALUE;
	static const char* FIELD_FLAGS;

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
	static QString getTableName(Hash signalHash);

	const QHash<Hash, ArchSignal>& archSignals() const { return m_archSignals; }

	void appendExistingTable(const QString& tableName);
	bool tableIsExists(const QString& tableName);

	static QString timeTypeStr(TimeType timeType);

	static qint64 localTimeOffsetFromUtc();

	static QString getCmpField(TimeType timeType);

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
