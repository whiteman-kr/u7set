#pragma once

#include <QSqlDatabase>

#include "../lib/SimpleThread.h"
#include "../lib/AppSignal.h"
#include "../lib/CircularLogger.h"

#include "Archive.h"


class ArchWriteThreadWorker : public SimpleThreadWorker
{
public:
	ArchWriteThreadWorker(Archive& archive,
						  AppSignalStatesQueue& saveStatesQueue,
						  CircularLoggerShared logger);

private:

	void onThreadStarted() override;
	void onThreadFinished() override;

	void tryConnectToDb();
	bool databaseIsExists(QSqlDatabase& db);
	bool createDatabase(QSqlDatabase& db);

	bool initDatabase();
	bool upgradeDatabase();
	bool checkAndCreateTables();

	int getDatabaseVersion();
	bool updateVersion(const QString& reasone);

	bool getExistingTables(QHash<QString, QString>& existingTables);

	bool createTable(const QString& tableName, Archive::TableType tableType);

	void disconnectFromDb();

	void writeStatesToArchive();
	bool saveAppSignalStateToArchive(SimpleAppSignalState& state, bool isAnalogSignal);
	bool saveAppSignalStatesArrayToArchive(const QString& arrayStr);

private slots:
	void onTimer();
	void onSaveStatesQueueIsNotEmpty();

private:
	Archive& m_archive;
	AppSignalStatesQueue& m_saveStatesQueue;
	CircularLoggerShared m_logger;

	QTimer m_timer;

	QSqlDatabase m_db;				// project archive database

	static const StringPair m_upgradeFiles[];

	//

	qint64 m_saveErrors = 0;

	Times m_lastTime;

	friend class ArchRequestThreadWorker;
};


class ArchWriteThread : public SimpleThread
{
public:
	ArchWriteThread(Archive& archive,
					AppSignalStatesQueue& saveStatesQueue,
					CircularLoggerShared logger);
};

