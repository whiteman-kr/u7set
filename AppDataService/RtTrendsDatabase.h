#pragma once

#include <QSqlDatabase>

#include "../lib/SimpleThread.h"
#include "../lib/AppSignal.h"
#include "../lib/CircularLogger.h"
#include "../lib/HostAddressPort.h"

class RtTrendsDatabase
{
public:
	RtTrendsDatabase(const QString& equipmentID,
					 const QString& accessStr1,
					 const QString& accessStr2);

	QString user() const;
	QString password() const;

private:
	QString m_equipmentID;
	QString m_accessStr1;
	QString m_accessStr2;
};

class RtTrendsWriteThread : public RunOverrideThread
{
public:
	RtTrendsWriteThread(const HostAddressPort& dbHost,
						  CircularLoggerShared logger);

private:
	void run() override;

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
	CircularLoggerShared m_logger;

	static QString m_format1;
	static QString m_format2;

	QSqlDatabase m_db;				// project archive database

	bool m_firstState = true;

	static const StringPair m_upgradeFiles[];

	//

	qint64 m_saveErrors = 0;

	Times m_lastTime;

	int m_prevMinute = -1;

	friend class ArchRequestThreadWorker;
};

