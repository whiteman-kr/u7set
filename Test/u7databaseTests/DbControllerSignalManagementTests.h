#pragma once

#include <QTest>
#include <QSql>
#include <QSqlError>
#include <QDebug>

#include "../../lib/DbController.h"

#define OPEN_DATABASE()		QSqlDatabase db; \
							QVERIFY2(openDatabase(db) == true, qPrintable(QString("Can't connect to database %1, error: %2" ). \
													arg(m_databaseName).arg(db.lastError().databaseText()))); \
							QSqlQuery q(db);


class DbControllerSignalTests : public QObject
{
	Q_OBJECT

public:
	DbControllerSignalTests();

private slots:
	void initTestCase();
//	void sql_add_signal
	void addSignalTest();
/*	void getSignalIdsTest();
	void checkInCheckOutSignalsTest();
	void getLatestSignalTest();
	void setSignalWorkCopyTest();
	void undoSignalChangesTest();
	void deleteSignalTest();
	void autoAddSignalsTest();
	void getSignalsTest();
	void getSignalHistoryTest();
	void getSpecificSignalsTest();*/
	void cleanupTestCase();

private:
	bool openDatabase(QSqlDatabase& db);
	QString lastError(const QSqlQuery& q) const { return q.lastError().databaseText(); }

	bool exec_add_signal(QSqlQuery& q, int userID, E::SignalType type, int channelCount);

private:

	int ADMIN_USER_ID = 1;

private:
	DbController *m_db;
	QString m_databaseHost;
	QString m_projectName;
	QString m_databaseName;
	QString m_databaseUser;
	QString m_adminPassword;
};
