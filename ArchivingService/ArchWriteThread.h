#pragma once

#include <QSqlDatabase>

#include "../lib/SimpleThread.h"
#include "../lib/AppSignal.h"
#include "../lib/CircularLogger.h"
#include "../lib/HostAddressPort.h"

#include "TimeFilter.h"

#include "Archive.h"


class ArchWriteThreadWorker : public SimpleThreadWorker
{
public:
	ArchWriteThreadWorker(const HostAddressPort& dbHost,
						  Archive& archive,
						  AppSignalStatesQueue& saveStatesQueue,
						  CircularLoggerShared logger);

private:

	void onThreadStarted() override;
	void onThreadFinished() override;

	bool tryConnectToDb();
	bool databaseIsExists(QSqlDatabase& db);
	bool createDatabase(QSqlDatabase& db);

	bool initDatabase();
	bool upgradeDatabase();
	bool checkAndCreateTables();

	int getDatabaseVersion();
	bool updateVersion(const QString& reasone);

	bool getExistingTables(QHash<QString, QString>& existingTables);

	bool createTable(const QString& tableName);

	void disconnectFromDb();

	void writeStatesToArchive(bool writeNow);
	void appendToArray(const SimpleAppSignalState& state, QString& arrayStr);

	bool saveAppSignalStateToArchive(SimpleAppSignalState& state, bool isAnalogSignal);
	bool saveAppSignalStatesArrayToArchive(const QString& arrayStr);

	bool writeTimeMark();

private slots:
	void onTimer();
	void onSaveStatesQueueIsNotEmpty();

private:
	HostAddressPort m_dbHost;

	Archive& m_archive;
	AppSignalStatesQueue& m_saveStatesQueue;
	CircularLoggerShared m_logger;

	static QString m_format1;
	static QString m_format2;

	QTimer m_timer;

	QSqlDatabase m_db;				// project archive database

	bool m_firstState = true;

	static const StringPair m_upgradeFiles[];

	//

	qint64 m_saveErrors = 0;

	TimeFilter m_timeFilter;

	Times m_lastTime;

	int m_prevMinute = -1;

	friend class ArchRequestThreadWorker;
};


class ArchWriteThread : public SimpleThread
{
public:
	ArchWriteThread(const HostAddressPort& dbHost,
					Archive& archive,
					AppSignalStatesQueue& saveStatesQueue,
					CircularLoggerShared logger);
};

