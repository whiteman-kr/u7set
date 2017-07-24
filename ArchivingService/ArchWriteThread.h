#pragma once

#include <QSqlDatabase>

#include "../lib/SimpleThread.h"
#include "../lib/AppSignal.h"
#include "../lib/CircularLogger.h"

typedef QHash<Hash, bool> ArchSignals;

class ArchWriteThreadWorker : public SimpleThreadWorker
{
public:
	enum SignalStatesTableType
	{
		LongTerm,
		ShortTerm
	};

	ArchWriteThreadWorker(const QString& projectID,
						  AppSignalStatesQueue& saveStatesQueue,
						  const ArchSignals& archSignals,
						  CircularLoggerShared logger);

private:

	void onThreadStarted() override;
	void onThreadFinished() override;

	void tryConnectToDb();
	bool databaseIsExists(QSqlDatabase& db);
	bool createDatabase(QSqlDatabase& db);

	bool initDatabase();
	bool upgradeDatabase();

	int getDatabaseVersion();
	bool updateVersion(const QString& reasone);

	bool getSignalsTablesList();

	void appendTable(const QString& tableName, SignalStatesTableType tableType);
	bool createTableIfNotExists(Hash signalHash, bool isAnalogSignal);
	bool createSignalStatesTable(Hash signalHash, SignalStatesTableType tableType);
	static QString getTableName(Hash signalHash, SignalStatesTableType tableType);

	void disconnectFromDb();

	void writeStatesToArchive();
	bool saveAppSignalStateToArchive(SimpleAppSignalState& state, bool isAnalogSignal);
	bool saveAppSignalStatesArrayToArchive(const QString& arrayStr);

	QString projectArchiveDbName();

private slots:
	void onTimer();
	void onSaveStatesQueueIsNotEmpty();

private:
	QString m_projectID;
	AppSignalStatesQueue& m_saveStatesQueue;
	const ArchSignals& m_archSignals;
	CircularLoggerShared m_logger;

	QTimer m_timer;

	static const char* ARCH_DB_PREFIX;
	static const char* LONG_TERM_TABLE_PREFIX;
	static const char* SHORT_TERM_TABLE_PREFIX;

	QSqlDatabase m_db;				// project archive database

	QHash<Hash, QString> m_longTermTables;
	QHash<Hash, QString> m_shortTermTables;

	static const StringPair m_upgradeFiles[];

	//

	qint64 m_saveErrors = 0;

	Times m_lastTime;

	friend class ArchRequestThreadWorker;
};


class ArchWriteThread : public SimpleThread
{
public:
	ArchWriteThread(const QString& projectID,
					AppSignalStatesQueue& saveStatesQueue,
					const ArchSignals& archSignals,
					CircularLoggerShared logger);
};

