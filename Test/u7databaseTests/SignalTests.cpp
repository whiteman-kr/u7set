#include <QtSql>
#include <QTest>
#include "SignalTests.h"


SignalTests::SignalTests()
{
}

void SignalTests::initTestCase()
{
	QSqlQuery query;

	// Testers
	//

	bool ok = query.exec("SElECT create_user(1, 'signalTestUser1', 'FIRSTTEST', 'FIRSTTEST', '12341234', false, false, false);");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	SignalTests::m_firstUserForTest = query.value("create_user").toInt();

	ok = query.exec("SElECT create_user(1, 'signalTestUser2', 'SECONDTEST', 'SECONDTEST', '12341234', false, false, false);");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	SignalTests::m_secondUserForTest = query.value("create_user").toInt();
}

void SignalTests::add_signalTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	// Values for function
	//

	int userId = 1;
	int signalType = 0;
	int channelCount = 4;

	bool ok = query.exec(QString("SELECT * FROM add_signal(%1, %2, %3)").arg(userId).arg(signalType).arg(channelCount));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Value that will count all channels
	//

	int channelNumber = 0;

	// Select signalGroupId of signals
	//

	ok = tempQuery.exec("SELECT MAX(signalGroupId) FROM signalGroup");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	int signalGroupId = tempQuery.value(0).toInt();

	// Check every created signal
	//

	while (query.next())
	{

		//Check error code and userId
		//

		QVERIFY2(query.value("errCode").toInt() == 0, qPrintable(QString("Error: error code %1").arg(query.value("errCode").toInt())));
		QVERIFY2(query.value("userId").toInt() == userId, qPrintable("Error: wrong user"));

		// Set right channel number for every signal (actually - number of the signal)
		//

		channelNumber += 1;

		// Select all signal data from table "checkOut" and check it
		//

		ok = tempQuery.exec(QString("SELECT * FROM checkOut WHERE signalId = %1").arg(query.value("id").toInt()));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("userId").toInt() == userId, qPrintable("Error: value userId in table checkout not match"));

		// Select all signal data from table "signal" and check it
		//

		ok = tempQuery.exec(QString("SELECT * FROM signal WHERE signalId = %1").arg(query.value("id").toInt()));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(signalGroupId == tempQuery.value("signalGroupId").toInt(), qPrintable("Error: signalGroupId in table signal not match"));
		QVERIFY2(channelNumber == tempQuery.value("channel").toInt(), qPrintable("Error: value channel in table signal not match"));
		QVERIFY2(signalType == tempQuery.value("type").toInt(), qPrintable("Error: value type in table signal not match"));
		QVERIFY2(query.value("deleted").toBool() == tempQuery.value("deleted").toBool(), qPrintable("Error: value deleted in table signal not match"));
		QVERIFY2(userId == tempQuery.value("userId").toInt(), qPrintable("Error: value userId in table signal not match"));

		int checkOutInstanceId = tempQuery.value("checkedOutInstanceId").toInt();

		// Select all signal data from table "signalInstance" and check it
		//

		ok = tempQuery.exec(QString("SELECT * FROM signalInstance WHERE signalId = %1").arg(query.value("id").toInt()));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("strId").toString() == QString("#SIGNAL" + query.value("id").toString() + "_" + char(64 + channelNumber)), qPrintable("Error: value strId in table signalInstance not match"));
		QVERIFY2(tempQuery.value("extStrId").toString() == QString("SIGNAL" + query.value("id").toString() + "_" + char(64 + channelNumber)), qPrintable("Error: value extStrId in table signalInstance not match"));
		QVERIFY2(tempQuery.value("name").toString() == QString("SIGNAL" + query.value("id").toString() + "_" + char(64 + channelNumber)), qPrintable("Error: value name in table signalInstance not match"));

		// Check data size to be correct
		//

		if (signalType == 0)
			QVERIFY2(tempQuery.value("dataSize").toInt() == 16, qPrintable("Error: wrong dataSize in table signalInstance"));
		else
			QVERIFY2(tempQuery.value("dataSize").toInt() == 0, qPrintable("Error: wrong dataSize in table signalInstance"));
		QVERIFY2(tempQuery.value("action").toInt() == 1, qPrintable("Error: value action in table signalInstance not match"));
		QVERIFY2(tempQuery.value("signalInstanceId").toInt() == checkOutInstanceId, qPrintable("Error: wrong checkedOutInstanceId"));
	}

	// Check count of channels must be equal channel count
	//

	QVERIFY2(channelNumber == channelCount, qPrintable("Error: number of channels is not similar to channelCount value"));

	// Check function with no admin user, different signal Type and different channel Count
	//

	userId = m_firstUserForTest;
	signalType = 1;
	channelCount = 0;

	ok = query.exec(QString("SELECT * FROM add_signal(%1, %2, %3)").arg(userId).arg(signalType).arg(channelCount));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	// Check single-channel signals must be in group 0
	//

	channelNumber = 1;
	signalGroupId = 0;

	QVERIFY2(query.value("errCode").toInt() == 0, qPrintable(QString("Error: error code %1").arg(query.value("errCode").toInt())));
	QVERIFY2(query.value("userId").toInt() == userId, qPrintable("Error: wrong user"));



	ok = tempQuery.exec(QString("SELECT * FROM checkOut WHERE signalId = %1").arg(query.value("id").toInt()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(tempQuery.value("userId").toInt() == userId, qPrintable("Error: value userId in table checkout not match"));



	ok = tempQuery.exec(QString("SELECT * FROM signal WHERE signalId = %1").arg(query.value("id").toInt()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(signalGroupId == tempQuery.value("signalGroupId").toInt(), qPrintable("Error: signalGroupId in table signal not match"));
	QVERIFY2(channelNumber == tempQuery.value("channel").toInt(), qPrintable("Error: value channel in table signal not match"));
	QVERIFY2(signalType == tempQuery.value("type").toInt(), qPrintable("Error: value type in table signal not match"));
	QVERIFY2(query.value("deleted").toBool() == tempQuery.value("deleted").toBool(), qPrintable("Error: value deleted in table signal not match"));
	QVERIFY2(userId == tempQuery.value("userId").toInt(), qPrintable("Error: value userId in table signal not match"));

	int checkOutInstanceId = tempQuery.value("checkedOutInstanceId").toInt();



	ok = tempQuery.exec(QString("SELECT * FROM signalInstance WHERE signalId = %1").arg(query.value("id").toInt()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(tempQuery.value("strId").toString() == QString("#SIGNAL" + query.value("id").toString()), qPrintable("Error: value strId in table signalInstance not match"));
	QVERIFY2(tempQuery.value("extStrId").toString() == QString("SIGNAL" + query.value("id").toString()), qPrintable("Error: value extStrId in table signalInstance not match"));
	QVERIFY2(tempQuery.value("name").toString() == QString("SIGNAL" + query.value("id").toString()), qPrintable("Error: value name in table signalInstance not match"));
	QVERIFY2(tempQuery.value("dataSize").toInt() == 1, qPrintable("Error: wrong dataSize in table signalInstance"));
	QVERIFY2(tempQuery.value("action").toInt() == 1, qPrintable("Error: value action in table signalInstance not match"));
	QVERIFY2(tempQuery.value("signalInstanceId").toInt() == checkOutInstanceId, qPrintable("Error: wrong checkedOutInstanceId"));

	// Check must be only one signal when signal count is 0
	//

	QVERIFY2(query.next() == false, qPrintable("Error: only one signal expected"));

	// Call invalid channel count error
	//

	userId = 1;
	signalType = 0;
	channelCount = maxValueId;

	// Call invalid userId error

	ok = query.exec(QString("SELECT * FROM add_signal(%1, %2, %3)").arg(userId).arg(signalType).arg(channelCount));
	QVERIFY2(ok == false, qPrintable("Too big channel_count error expected"));

	userId = maxValueId;
	signalType = 0;
	channelCount = 1;

	ok = query.exec(QString("SELECT * FROM add_signal(%1, %2, %3)").arg(userId).arg(signalType).arg(channelCount));
	QVERIFY2(ok == false, qPrintable("Wrong user error expected"));
}

void SignalTests::get_signal_IdsTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	int userId = 1;

	// Create few signals for test
	//

	bool ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 4)").arg(userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 4)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Match one record as deleted
	//

	ok = query.exec("UPDATE signal SET deleted = true WHERE signalId = 1");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Check all signals with deleted
	//

	ok = query.exec(QString("SELECT * FROM get_signals_Ids(%1, true) ORDER BY get_signals_Ids").arg(userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec("SELECT signalId FROM signal ORDER BY signalId");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next())
	{
		QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(query.value(0).toInt() == tempQuery.value(0).toInt(), qPrintable(QString("Error: id's not match (function id = %1, table id = %2").arg(query.value(0).toInt()).arg(tempQuery.value(0).toInt())));
	}

	// Check all records without deleted
	//

	ok = query.exec(QString("SELECT * FROM get_signals_Ids(%1, false) ORDER BY get_signals_Ids").arg(userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec("SELECT signalId FROM signal WHERE deleted = false ORDER BY signalId");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next())
	{
		QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(query.value(0).toInt() == tempQuery.value(0).toInt(), qPrintable(QString("Error: id's not match (function id = %1, table id = %2").arg(query.value(0).toInt()).arg(tempQuery.value(0).toInt())));
	}

	// Check all records of firstUserForTest
	//

	ok = query.exec(QString("SELECT * FROM get_signals_Ids(%1, true) ORDER BY get_signals_Ids").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT signalId FROM signal WHERE userId = %1 ORDER BY signalId").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));

	while (query.next())
	{
		QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(query.value(0).toInt() == tempQuery.value(0).toInt(), qPrintable(QString("Error: id's not match (function id = %1, table id = %2").arg(query.value(0).toInt()).arg(tempQuery.value(0).toInt())));
	}

	// Check no records of invalid user
	//

	ok = query.exec(QString("SELECT * FROM get_signals_Ids(%1, true) ORDER BY get_signals_Ids").arg(maxValueId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("No rows expected"));
}

void SignalTests::get_signal_countTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	// Create few signals for test
	//

	bool ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 4)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 4)").arg(m_secondUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Execute function as admin
	//

	ok = query.exec("SELECT * FROM get_signal_count(1)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Get count of all signals
	//

	ok = tempQuery.exec("SELECT COUNT(*) FROM signal");
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));

	// Check results between result of function and data from table
	//

	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value(0).toInt() == tempQuery.value(0).toInt(), qPrintable(QString("Error: result not match (function: %1, table: %2)").arg(query.value(0).toInt()).arg(tempQuery.value(0).toInt())));

	// Execute function as ordinary user
	//

	ok = query.exec(QString("SELECT * FROM get_signal_count(%1)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Get count of all signals owned by this user
	//

	ok = tempQuery.exec(QString("SELECT COUNT(*) FROM signal WHERE userId = %1").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));

	// Check results between result of function and data from table
	//

	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value(0).toInt() == tempQuery.value(0).toInt(), qPrintable(QString("Error: result not match (function: %1, table: %2)").arg(query.value(0).toInt()).arg(tempQuery.value(0).toInt())));

	// Check no records for invalid user
	//

	ok = query.exec(QString("SELECT * FROM get_signal_count(%1)").arg(maxValueId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toInt() == 0, qPrintable("No records expected"));
}
