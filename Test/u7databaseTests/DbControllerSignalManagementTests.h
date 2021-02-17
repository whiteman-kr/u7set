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
																arg(queryStr).arg(q.lastError().text()))

#define TS_EXEC_QUERY_STR(queryStr)				[queryStr]()	\
												{		\
													QSqlQuery q;	\
													if(q.exec(queryStr) == false)	\
													{	\
														return QString("Error execution of query '%1':\n%2").	\
																arg(queryStr).arg(q.lastError().text());	\
													}	\
													return QString();	\
												}()

#define TS_VERIFY(conditionStr)					{	\
													QString errStr = conditionStr;	\
													QVERIFY2(errStr.isEmpty() == true, C_STR(errStr));	\
												}

// For correct error reporting parameter q should NOT be typed as QSqlQuery()
//
#define TS_EXEC_QUERY_RETURN_ERR(q, queryStr)		if (q.exec(queryStr) == false) \
													{	\
														return QString("Error execution of query '%1':\n%2").		\
																	arg(queryStr).arg(q.lastError().text()); \
													}

#define TS_EXEC_QUERY_STR_RETURN_ERR(queryStr)		{	\
														QSqlQuery q; \
														if (q.exec(queryStr) == false) \
														{	\
															return QString("Error execution of query '%1':\n%2").	\
																	arg(queryStr).arg(q.lastError().text()); \
														} \
													}

#define TS_VERIFY_RETURN_ERR(condition, errMsg)		if ((condition) == false) \
													{	\
														return errMsg; \
													}

#define TS_VERIFY_RETURN(resultStr)					if (resultStr.isEmpty() == false) \
													{	\
														return resultStr; \
													}

#define TS_RETURN_SUCCESS()							return QString();

#define TS_TEST_PTR_RETURN(ptr)						if (ptr == NULL) { Q_ASSERT(false); return "NULL pointer"; }



class DbControllerSignalTests : public QObject
{
	Q_OBJECT

public:
	DbControllerSignalTests();

private slots:
	void initTestCase();

	// stored procedures tests
	//
	void test_addSignal();
	void test_checkinSignals();
	void test_checkoutSignals();
	void test_deleteSignal();
	void test_setSignalWorkcopy();
	void test_getSignalsIDs();
	void test_getSignalsIDAppSignalID();

	// DbController methods tests
	//
	void dbcTest_addSignal();


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

	QString applyFutureDatabaseUpgrade();

	QString addSignal(int userID, E::SignalType type, int channelCount, std::vector<ObjectState>* obStates);

	QString check_signalIsExist(	int userID,
								int signalID,
								E::SignalType type,
								int channel,
								int signalGroupID,
								bool isCheckedOut);

	QString getFieldValue(QString valueFiled, int* value, QString table, QString whereField, int whereValue);
	QString getSignalGroupID(int signalID, int* signalGroupID);
	QString getSignalInstanceID(int signalID, int* signalInstanceID);

	QString setSignalWorkcopy(int userID, const Signal& s, ObjectState* obState);

	QString checkinSignals(int userID,
						   const std::vector<int>& ids,
						   const QString&comment,
						   std::vector<ObjectState>* obStates);

	QString checkoutSignals(int userID,
						   const std::vector<int>& ids,
						   std::vector<ObjectState>* obStates);

	QString check_signalIsCheckedIn(int signalID);
	QString check_signalIsCheckedOut(int signalID, VcsItemAction* action = nullptr);

	QString deleteSignal(int userID, int signalID, ObjectState* obState);
	QString deleteSignals(int userID, const std::vector<int>& ids, std::vector<ObjectState>* obStates);

	QString getSignalsIDs(int userID, bool withDeleted, std::vector<int>* ids);

	QString addTestSignals(int userID,
						   E::SignalType signalType,
						   int channelCount,
						   int signalCount, std::vector<int>* addedSignalsIDsSorted);

	QString getAllSignalIDs(std::vector<int>* allSignalIDsSorted);

	int rand0to(int upRange) const;

	template <typename T>
	std::vector<T> sets_difference(std::vector<T> set1, std::vector<T> set2);

	template <typename T>
	std::vector<T> sets_intersection(std::vector<T> set1, std::vector<T> set2);

	template <typename T>
	bool sets_intersect(std::vector<T> set1, std::vector<T> set2);

	template <typename T>
	std::vector<T> sets_union(std::vector<T> set1, std::vector<T> set2);

	template <typename T>
	bool sets_equal(std::vector<T> set1, std::vector<T> set2);

	//

	QString lastError(const QSqlQuery& q) const { return q.lastError().text(); }

private:
	const int ADMIN_ID = 1;
	const int USER2_ID = 2;
	const int USER3_ID = 3;

private:
	DbController* m_db = nullptr;
	QString m_databaseHost;
	QString m_projectName;
	QString m_databaseName;
	QString m_databaseUser;
	QString m_adminPassword;
};

template <typename T>
std::vector<T> DbControllerSignalTests::sets_difference(std::vector<T> set1, std::vector<T> set2)
{
	std::sort(set1.begin(), set1.end());
	std::sort(set2.begin(), set2.end());

	std::vector<T> resultSet;

	std::set_difference(set1.begin(), set1.end(),
						set2.begin(), set2.end(),
						std::inserter(resultSet, resultSet.begin()));
	return resultSet;
}

template <typename T>
std::vector<T> DbControllerSignalTests::sets_intersection(std::vector<T> set1, std::vector<T> set2)
{
	std::sort(set1.begin(), set1.end());
	std::sort(set2.begin(), set2.end());

	std::vector<T> resultSet;

	std::set_intersection(	set1.begin(), set1.end(),
							set2.begin(), set2.end(),
							std::inserter(resultSet, resultSet.begin()));
	return resultSet;
}

template <typename T>
bool DbControllerSignalTests::sets_intersect(std::vector<T> set1, std::vector<T> set2)
{
	return sets_intersection(set1, set2).size() != 0;
}

template <typename T>
std::vector<T> DbControllerSignalTests::sets_union(std::vector<T> set1, std::vector<T> set2)
{
	std::sort(set1.begin(), set1.end());
	std::sort(set2.begin(), set2.end());

	std::vector<T> resultSet;

	std::set_union(	set1.begin(), set1.end(),
							set2.begin(), set2.end(),
							std::inserter(resultSet, resultSet.begin()));
	return resultSet;
}

template <typename T>
bool DbControllerSignalTests::sets_equal(std::vector<T> set1, std::vector<T> set2)
{
	int size = static_cast<int>(set1.size());

	if (size != static_cast<int>(set2.size()))
	{
		return false;
	}

	if (size == 0)
	{
		return true;
	}

	std::sort(set1.begin(), set1.end());
	std::sort(set2.begin(), set2.end());

	for(int i = 0; i < size; i++)
	{
		if (set1[i] != set2[i])
		{
			return false;
		}
	}

	return true;
}

