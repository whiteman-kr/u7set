#pragma once

#include <QTest>
#include <QSql>
#include <QSqlError>
#include <QDebug>

#include "../../lib/DbController.h"


#define OPEN_DATABASE()		QSqlDatabase db; \
							QVERIFY2(openDatabase(db) == true, qPrintable(QString("Can't connect to database %1, error: %2" ). \
													arg(m_databaseName).arg(db.lastError().text()))); \

// For correct error reporting parameter q should NOT be typed as QSqlQuery()
//
#define TS_EXEC_QUERY(q, queryStr)				(q.exec(queryStr) == true ? QString() : \
													QString("Error execution of query '%1':\n%2").		\
																arg(queryStr).arg(lastError(q)))

#define TS_EXEC_QUERY_STR(queryStr)				(QSqlQuery q(), q.exec(queryStr) == true ? QString() : \
													QString("Error execution of query '%1':\n%2").		\
																arg(queryStr).arg(lastError(q)))

#define TS_VERIFY(conditionStr)					{	\
													QString errStr = conditionStr;	\
													QVERIFY2(errStr.isEmpty() == true, C_STR(errStr));	\
												}

// For correct error reporting parameter q should NOT be typed as QSqlQuery()
//
#define TS_EXEC_QUERY_RETURN_ERR(q, queryStr)		if (q.exec(queryStr) == false) \
													{	\
														return QString("Error execution of query '%1':\n%2").		\
																	arg(queryStr).arg(lastError(q)); \
													}

#define TS_EXEC_QUERY_STR_RETURN_ERR(queryStr)		{	\
														QSqlQuery q; \
														if (q.exec(queryStr) == false) \
														{	\
															return QString("Error execution of query '%1':\n%2").	\
																	arg(queryStr).arg(lastError(q)); \
														} \
													}

#define TS_VERIFY_RETURN_ERR(condition, errMsg)		if ((condition) == false) \
													{	\
														return errMsg; \
													}

#define TS_RETURN_SUCCESS()							return QString();


class DbControllerSignalTests : public QObject
{
	Q_OBJECT

public:
	DbControllerSignalTests();

private slots:
	void initTestCase();

	void addSignalTest();
	void checkinSignalsTest();
	void checkoutSignalsTest();
	void setSignalWorkcopyTest();


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

	QString addSignal(int userID, E::SignalType type, int channelCount, std::vector<ObjectState>* obStates);

	QString check_signalIsExist(	int userID,
								int signalID,
								E::SignalType type,
								int channel,
								int signalGroupID,
								bool isCheckedOut);

	QString setSignalWorkcopy(int userID, const Signal& s, ObjectState* obState);

	QString checkinSignals(int userID,
						   const std::vector<int>& ids,
						   const QString&comment,
						   std::vector<ObjectState>* obStates);

	QString checkoutSignals(int userID,
						   const std::vector<int>& ids,
						   std::vector<ObjectState>* obStates);

	QString check_signalIsCheckedIn(int signalID);
	QString check_signalIsCheckedOut(int signalID);

	//

	QString lastError(const QSqlQuery& q) const { return q.lastError().text(); }

private:
	const int ADMIN_ID = 1;
	const int USER2_ID = 2;
	const int USER3_ID = 3;

private:
	DbController *m_db;
	QString m_databaseHost;
	QString m_projectName;
	QString m_databaseName;
	QString m_databaseUser;
	QString m_adminPassword;
};
