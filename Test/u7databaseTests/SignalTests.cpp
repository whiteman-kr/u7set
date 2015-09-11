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

void SignalTests::checkin_signalsTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	int userId = 1;

	int signalId[3] = {-1,-1,-1};
	int deletedChekedOutInstanceId[2] = {-1,-1};

	int signalIdNumber=0;
	int numberOfDeletedSignalId = 0;

	QString comment="test";

	// Create few signals for test
	//

	bool ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 0)").arg(userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	signalId[0] = query.value("ID").toInt();

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 2)").arg(userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	signalId[1] = query.value("ID").toInt();
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	signalId[2] = query.value("ID").toInt();

	// Match action in second signal as 3 (matched to be deleted)
	//

	ok = query.exec(QString("SELECT * FROM signal WHERE signalId = %1 OR signalId = %2").arg(signalId[1]).arg(signalId[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	// Save signalGroup and checkedOutInstanceId of deleted files to check siignal to be deleted
	//

	int deletedSignalGroupId = query.value("signalGroupId").toInt();
	deletedChekedOutInstanceId[0] = query.value("checkedOutInstanceId").toInt();

	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	deletedChekedOutInstanceId[1] = query.value("checkedOutInstanceId").toInt();

	ok = query.exec(QString("UPDATE signalInstance SET action = 3 WHERE signalId = %1 OR signalId = %2").arg(signalId[1]).arg(signalId[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Start checkin_signals() function
	//

	ok = query.exec(QString("SElECT * FROM checkin_signals(1, '{%1, %2, %3}', '%4')").arg(signalId[0]).arg(signalId[1]).arg(signalId[2]).arg(comment));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next())
	{
		// Check errCode is 0 and record in checkOut table, which must be deleted

		QVERIFY2(query.value("errCode").toInt() == 0, qPrintable(QString("Error: error code must be 0, current: %1").arg(query.value("errCode").toInt())));

		ok = tempQuery.exec(QString("SELECT * FROM checkOut WHERE signalId = %1").arg(signalId[signalIdNumber]));
		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.next() == false, qPrintable("Error: no checkout record expected"));

		if (signalIdNumber == 0) // Check first record which must not be deleted
		{
			// Check record in signalInstance table
			//

			ok = tempQuery.exec(QString("SELECT * FROM signalInstance WHERE signalId = %1").arg(signalId[signalIdNumber]));
			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value("changeSetId").toInt() != 0, qPrintable("Error: no changeset created"));

			int changeSetId = tempQuery.value("changesetId").toInt();
			int checkedInInstanceId = tempQuery.value("signalInstanceId").toInt();

			// Check record in table changeSet
			//

			ok = tempQuery.exec(QString("SELECT * FROM changeSet WHERE changesetId = %1").arg(changeSetId));
			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value("file").toBool() == false, qPrintable("Error: wrong record in file column table changeSet: FALSE expected"));
			QVERIFY2(tempQuery.value("userId").toInt() == userId, qPrintable("Error: wrong user in table ChangeSet"));
			QVERIFY2(tempQuery.value("comment").toString() == comment, qPrintable("Error: wrong comment"));

			// Check record in table signal
			//

			ok = tempQuery.exec(QString("SELECT * FROM signal WHERE signalId = %1").arg(signalId[signalIdNumber]));
			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value("checkedOutInstanceId").toInt() == NULL, qPrintable("Error: no checkedOutInstanceId expected"));
			QVERIFY2(tempQuery.value("checkedInInstanceId").toInt() == checkedInInstanceId, qPrintable("Error: wrong checkedInInstanceId"));
		}
		else	// Checking deleted record
		{
			// Check that record was deleted in two tables
			//

			ok = tempQuery.exec(QString("SELECT * FROM signal WHERE signalId = %1").arg(signalId[signalIdNumber]));
			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == false, qPrintable("Error: no record expected (Must be deleted)"));

			ok = tempQuery.exec(QString("SELECT * FROM signalInstance WHERE signalInstanceId = %1").arg(deletedChekedOutInstanceId[numberOfDeletedSignalId]));
			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == false, qPrintable("Error: no record expected (Must be deleted)"));

			// Check signalGroup was, or was not deleted in case of count signalIds in group
			//

			ok = tempQuery.exec(QString("SELECT * FROM signalGroup WHERE signalGroupId = %1").arg(deletedSignalGroupId));
			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == false, qPrintable("Error: no signalGroup expected"));

			numberOfDeletedSignalId++;
		}

		signalIdNumber++;

	}

	// Call checkedIn signal error
	//

	ok = query.exec(QString("SElECT * FROM checkin_signals(1, '{%1}', '%2')").arg(signalId[0]).arg(comment));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("errCode").toInt() == 1, qPrintable("Already cheked In error expected!"));

	// Call wrong user error
	//

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 0)").arg(userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	signalId[0] = query.value("ID").toInt();

	ok = query.exec(QString("SElECT * FROM checkin_signals(%1, '{%2}', '%3')").arg(m_firstUserForTest).arg(signalId[0]).arg(comment));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("errCode").toInt() == 2, qPrintable("Wrong user error expected!"));
}  // try to delete multi-channel signal

void SignalTests::checkout_signalsTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	QString comment = "checkout_signal Test";

	// Create array of signalIds. First element must bee invalid to test error #4: ERR_SIGNAL_NOT_FOUND
	//

	int signalIds[4] = {maxValueId,0,0,0};
	int currentSignalId = 1;
	int userId = 1;


	bool ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 3)").arg(userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	for (int signalNumber = 0; signalNumber<3; signalNumber++)
	{
		query.next();
		signalIds[currentSignalId] = query.value("ID").toInt();
		currentSignalId++;
	}

	QVERIFY2(query.next() == false, qPrintable("Error: error in add_signal() function: too much output records!"));

	// Match second element as deleted to test error #3: ERR_SIGNAL_DELETED
	//

	ok = query.exec(QString("UPDATE Signal SET deleted = true WHERE signalId = %1").arg(signalIds[1]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Do not check in third element to test error #2: ERR_SIGNAL_ALREADY_CHECKED_OUT

	ok = query.exec(QString("SElECT * FROM checkIn_signals(%1, '{%2}', '%3')").arg(userId).arg(signalIds[3]).arg(comment));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SElECT * FROM checkOut_signals(%1, '{%2, %3, %4, %5}')").arg(userId).arg(signalIds[0]).arg(signalIds[1]).arg(signalIds[2]).arg(signalIds[3]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	currentSignalId = 0;

	while (query.next())
	{
		switch(currentSignalId)
		{
			case 0:	// Invalid signal id
			{
				QVERIFY2(query.value("errCode").toInt() == 4, qPrintable("Signal not found error expected"));
			} break;

			case 1:	// Deleted signal id
			{
				QVERIFY2(query.value("errCode").toInt() == 3, qPrintable("Signal has been deleted error expected"));
			} break;

			case 2:	// Already checked out signal id
			{
				QVERIFY2(query.value("errCode").toInt() == 2, qPrintable("Signal already checked out error expected"));
			} break;

			case 3:	// Ordinary signal id
			{
				QVERIFY2(query.value("errCode").toInt() == 0, qPrintable("Error code must be 0"));

				tempQuery.exec(QString("SELECT * FROM checkOut WHERE signalId = %1").arg(signalIds[currentSignalId]));
				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value("userId").toInt() == userId, qPrintable(QString("Error: userId is not match in table checkOut (signalId %1)").arg(signalIds[currentSignalId])));

				tempQuery.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(signalIds[currentSignalId]));
				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.value("checkedOutInstanceId").toInt() != NULL, qPrintable("Error: no record in column checkedInInstanceId expected"));

				int checkOutId = tempQuery.value("checkedOutInstanceId").toInt();


				QVERIFY2(tempQuery.value("userId").toInt() == userId, qPrintable(QString("Error: userId is not match in table signal (signalId %1)").arg(signalIds[currentSignalId])));

				tempQuery.exec(QString("SELECT * FROM signalInstance WHERE signalId = %1 AND changeSetId is NULL").arg(signalIds[currentSignalId]));
				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value("signalInstanceId").toInt() == checkOutId, qPrintable(QString("Error: signalInstanceId is wrong in table signalInstance (signalId %1)").arg(signalIds[currentSignalId])));
			} break;
		}

		currentSignalId++;

	}

	// Try to checkOut chekcouted signal by another user; If error occured - check returned userId
	//

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 0)").arg(userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	int signalId = query.value("ID").toInt();

	ok = query.exec(QString("SElECT * FROM checkIn_signals(%1, '{%2}', '%3')").arg(userId).arg(signalId).arg(comment));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SElECT * FROM checkOut_signals(%1, '{%2}')").arg(userId).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SElECT * FROM checkOut_signals(%1, '{%2}')").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT userId FROM Signal WHERE signalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(tempQuery.value(0).toInt() == userId, qPrintable("Error: userId is wrong in error testing"));
	QVERIFY2(query.value("errCode").toInt() == 2, qPrintable(QString("Error: Error code is %1, expected - 2").arg(query.value("errCode").toInt())));
}

void SignalTests::delete_signalTest()
{
	QSqlQuery query;

	int signalIds[3] = {-1, -1, -1};
	int numberOfSignal = 0;
	bool ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 3)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	while (query.next() == true)
	{
		signalIds[numberOfSignal] = query.value("id").toInt();
		numberOfSignal++;
	}

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM signal WHERE signalId = %1").arg(signalIds[1]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int deletedCheckedOutId = query.value(0).toInt();

	// Check in first signal to make sure, that function will checkOut it, and change action to 3
	//

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'TEST');").arg(m_firstUserForTest).arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Do not change second signal. Function must delete it from database, because it was never checked in
	//

	// Check in and checkOut last signal to get ALREADY_CHECKED_OUT error
	//

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'TEST');").arg(m_firstUserForTest).arg(signalIds[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(%1, '{%2}');").arg(m_secondUserForTest).arg(signalIds[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Start delete_signal function with first signal
	//

	ok = query.exec(QString("SELECT * FROM delete_signal(%1, %2)").arg(m_firstUserForTest).arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("errCode").toInt() == 0, qPrintable("Error: error code is not 0!"));

	ok = query.exec(QString("SELECT * FROM signal WHERE signalId = %1").arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("checkedOutInstanceId").toInt() != 0, qPrintable("Signal must be checked out"));

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE signalInstanceId = %1").arg(query.value("checkedOutInstanceId").toInt()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("action").toInt() == 3, qPrintable ("Error: action must be 3 - matched for deletion"));

	// Start delete_signal function with second signal
	//

	ok = query.exec(QString("SELECT * FROM delete_signal(%1, %2)").arg(m_firstUserForTest).arg(signalIds[1]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("errCode").toInt() == 0, qPrintable("Error: error code is not 0!"));

	ok = query.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(signalIds[1]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("Error: no record expected"));

	// Check signalInstance table for deleted record
	//

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE signalInstanceId = %1").arg(deletedCheckedOutId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("Error: no record expected"));

	// Start delete_signal function with already checkedOut signal
	//

	ok = query.exec(QString("SELECT * FROM delete_signal(%1, %2);").arg(m_firstUserForTest).arg(signalIds[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("errCode").toInt() == 2, qPrintable("SIGNAL_ALREADY_CHECKEDOUT error expected!"));

	// Check deletion of signalGroups
	//

	ok = query.exec("SELECT * FROM add_signal(1, 0, 2);");
	QVERIFY2(ok == true, qPrintable (query.lastError().databaseText()));

	// delete data from signalIds array, and fill it with new signaIds
	//

	int signalIdsForSignalGroupTest[2] = {-1, -1};

	QVERIFY2(query.next() == true, qPrintable (query.lastError().databaseText()));
	signalIdsForSignalGroupTest[0] = query.value("id").toInt();
	QVERIFY2(query.next() == true, qPrintable (query.lastError().databaseText()));
	signalIdsForSignalGroupTest[1] = query.value("id").toInt();

	// Find and save signals groupId
	//

	ok = query.exec(QString("SELECT signalGroupId FROM Signal WHERE signalId = %1").arg(signalIdsForSignalGroupTest[0]));
	QVERIFY2(ok == true, qPrintable (query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable (query.lastError().databaseText()));

	int signalGroupId = query.value(0).toInt();

	ok = query.exec(QString("SELECT * FROM delete_signal(1, %1)").arg(signalIdsForSignalGroupTest[0]));
	QVERIFY2(ok == true, qPrintable (query.lastError().databaseText()));

	// Check that afted removing first signal, signalGroupId is not removed
	//

	ok = query.exec(QString("SELECT * FROM signalGroup WHERE signalGroupId = %1").arg(signalGroupId));
	QVERIFY2(ok == true, qPrintable (query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable ("Error: signalGroup has been deleted."));

	// Delete second signal
	//

	ok = query.exec(QString("SELECT * FROM delete_signal(1, %1)").arg(signalIdsForSignalGroupTest[1]));
	QVERIFY2(ok == true, qPrintable (query.lastError().databaseText()));

	// Check that signalGroup has been deleted
	//

	ok = query.exec(QString("SELECT * FROM signalGroup WHERE signalGroupId = %1").arg(signalGroupId));
	QVERIFY2(ok == true, qPrintable (query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable ("Error: signalGroup must be deleted"));
}

void SignalTests::get_latest_signalTest()
{
	/*QSqlQuery query;
	QSqlQuery tempQuery;

	bool ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 0)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	int signalId = query.value("ID").toInt();

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value("signalInstanceId").toInt() == tempQuery.value(0).toInt(), qPrintable("Error: signalInstanceId is wrong"));
	QVERIFY2(signalId == query.value("signalId").toInt(), qPrintable("Error: wrong sigalId"));

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(m_secondUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("Error: no record expected"));  // Why empty string appears? There must be no string.

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'TEST');").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT checkedInInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value("signalInstanceId").toInt() == tempQuery.value(0).toInt(), qPrintable("Error: signalInstanceId is wrong"));
	QVERIFY2(signalId == query.value("signalId").toInt(), qPrintable("Error: wrong sigalId"));

	ok = query.exec(QString("SELECT * FROM checkout_signals(%1, '{%2}');").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("UPDATE signalInstance SET action = 2 WHERE changeSetId IS NULL AND signalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value("signalInstanceId").toInt() == tempQuery.value(0).toInt(), qPrintable("Error: signalInstanceId is wrong"));
	QVERIFY2(signalId == query.value("signalId").toInt(), qPrintable("Error: wrong sigalId"));
	QVERIFY2(query.value("action").toInt() == 2, qPrintable("Error: Wrong record!"));

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(m_secondUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT checkedInInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value("signalInstanceId").toInt() == tempQuery.value(0).toInt(), qPrintable("Error: signalInstanceId is wrong"));
	QVERIFY2(signalId == query.value("signalId").toInt(), qPrintable("Error: wrong sigalId"));
	QVERIFY2(query.value("action").toInt() == 1, qPrintable("Error: Wrong record!"));*/
}

void SignalTests::get_latest_signalsTest()
{
	/*QSqlQuery query;
	QSqlQuery tempQuery;

	int signalIds[4] = {-1, -1, -1, -1};
	int numberOfSignal = 0;

	// Value, that will be changed by another user to test checkOuted signals
	//

	QString nameToChange = "TEST";

	bool ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 3)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	for (numberOfSignal=0; numberOfSignal<3; numberOfSignal++)
	{
		QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
		signalIds[numberOfSignal] = query.value("id").toInt();
	}

	// Create one signal by secondUser to check, that firstUser will get empty string
	//

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 3)").arg(m_secondUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalIds[numberOfSignal] = query.value("id").toInt();

	numberOfSignal = 0;

	// Create history for first signalId. After get_latest_signal() function first user must get all checkedIn data of the signal, and second user
	// must get all checkedOut data of the signal
	//

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'TEST');").arg(m_firstUserForTest).arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkOut_signals(%1, '{%2}');").arg(m_secondUserForTest).arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("UPDATE signalInstance SET name = '%1' WHERE changeSetId IS NULL AND signalId = %2").arg(nameToChange).arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Check in one signal to make sure, that first user get data of checkedIn file
	//

	ok = query.exec(QString("SElECT * FROM checkin_signals(%1, '{%2}', 'TEST');").arg(m_firstUserForTest).arg(signalIds[1]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Create history for third signal id. First user must checkOut signal, and get all data of checkOuted signal
	//

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'TEST');").arg(m_firstUserForTest).arg(signalIds[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkOut_signals(%1, '{%2}');").arg(m_firstUserForTest).arg(signalIds[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("UPDATE signalInstance SET name = '%1' WHERE changeSetId IS NULL AND signalId = %2").arg(nameToChange).arg(signalIds[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Start get_latest_signals() test function
	//

	ok = query.exec(QString("SELECT * FROM get_latest_signals(%1, '{%2, %3, %4, %5}') ORDER BY SignalId").arg(m_firstUserForTest).arg(signalIds[0]).arg(signalIds[1]).arg(signalIds[2]).arg(signalIds[3]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next() == true)
	{
		switch (numberOfSignal)
		{
			case 0:
			{
				QVERIFY2(query.value("signalId").toInt() == signalIds[numberOfSignal], qPrintable("Error: wrong signalId"));

				ok = tempQuery.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(signalIds[numberOfSignal]));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value("checkedInInstanceId").toInt() == query.value("signalInstanceId"), qPrintable ("Error: wrong signalinstance in wrong userId with checkOuted signal"));

				ok = tempQuery.exec(QString("SELECT * FROM SignalInstance WHERE signalInstanceId = %1").arg(tempQuery.value("checkedInInstanceId").toInt()));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value("signalId").toInt() == signalIds[numberOfSignal], qPrintable("Error: wrong signalId in signalInstance"));
				QVERIFY2(tempQuery.value("name").toString() != nameToChange, qPrintable("Error: wrong name in wrong userId with checkOuted signal"));

				numberOfSignal++;
			} break;

			case 1:
			{
				QVERIFY2(query.value("signalId").toInt() == signalIds[numberOfSignal], qPrintable("Error: wrong signalId"));

				ok = tempQuery.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(signalIds[numberOfSignal]));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value("checkedInInstanceId").toInt() == query.value("signalInstanceId"), qPrintable ("Error: wrong signalinstance in wrong userId with checkOuted signal"));

				numberOfSignal++;
			} break;
			case 2:
			{
				QVERIFY2(query.value("signalId").toInt() == signalIds[numberOfSignal], qPrintable("Error: wrong signalId"));

				ok = tempQuery.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(signalIds[numberOfSignal]));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value("checkedOutInstanceId").toInt() == query.value("signalInstanceId"), qPrintable ("Error: wrong signalinstance in wrong userId with checkOuted signal"));

				ok = tempQuery.exec(QString("SELECT * FROM SignalInstance WHERE signalInstanceId = %1").arg(tempQuery.value("checkedOutInstanceId").toInt()));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value("signalId").toInt() == signalIds[numberOfSignal], qPrintable("Error: wrong signalId in signalInstance"));
				QVERIFY2(tempQuery.value("name").toString() == nameToChange, qPrintable("Error: wrong name in wrong userId with checkOuted signal"));

				numberOfSignal++;
			} break;
			case 3: QFAIL("Error: There must be only 3 records"); break;
		}
	}

	// Check in edited signal by second user, and check it by function with first user
	//

	ok = query.exec(QString("SELECT * FROM checkin_signals (%1, '{%2}', '%3')").arg(m_secondUserForTest).arg(signalIds[0]).arg("TEST"));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_latest_signals(%1, '{%2}')").arg(m_firstUserForTest).arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value("name").toString() == nameToChange, qPrintable("Error: function returns wrong name after function checkin_signals()"));

	// Check deleted signal. user, who deleted must see it, and others musn't.

	ok = query.exec(QString("SELECT * FROM delete_signal(%1, %2)").arg(m_firstUserForTest).arg(signalIds[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SElECT * FROM checkin_signals(%1, '{%2}', 'TEST')").arg(m_firstUserForTest).arg(signalIds[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_latest_signals(%1, '{%2}');").arg(m_firstUserForTest).arg(signalIds[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));



	// Try start function with invalid user
	//

	ok = query.exec(QString("SELECT * FROM get_latest_signals(%1, '{1}')").arg(maxValueId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("No record expected, invalid userId"));

	// Try start function with invalid signal
	//

	ok = query.exec(QString("SELECT * FROM get_latest_signals(1, '{%1}')").arg(maxValueId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("No record expected, invalid userId"));*/
}

void SignalTests::get_latest_signals_allTest()
{
	/*QSqlQuery query;
	QSqlQuery tempQuery;

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	int deletedSignalId = query.value("id").toInt();

	ok = query.exec(QString("UPDATE Signal SET Deleted = true WHERE signalId = %1").arg(query.value("Id").toInt())); // Do it with functions delete_signal and checkin_signal
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));											// change one row which were ediedby another user

	ok = query.exec("SELECT * FROM get_latest_signals_all(1)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec("SELECT * FROM Signal WHERE Deleted = false ORDER BY SignalId");
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));

	while (query.next() == true && tempQuery.next() == true)
	{
		QVERIFY2(query.value("signalId").toInt() != deletedSignalId, qPrintable("Error: deleted signal was not expected"));
		QVERIFY2(tempQuery.value("signalId").toInt() == query.value("signalId").toInt(), qPrintable(QString("%1:%2").arg(query.value("signalId").toInt()).arg(tempQuery.value("signalId").toInt())));

		if (tempQuery.value("userId").toInt() == 0)
		{
			QVERIFY2(tempQuery.value("checkedInInstanceId").toInt() == query.value("signalInstanceId").toInt(), qPrintable("Error: wrong signalInstance Id"));
		}
		else
		{
			QVERIFY2(tempQuery.value("checkedOutInstanceId").toInt() == query.value("signalInstanceId").toInt(), qPrintable("Error: wrong signalInstance Id"));
		}
	}

	ok = query.exec(QString("SELECT * FROM get_latest_signals_all(%1)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec("SELECT * FROM Signal WHERE Deleted = false ORDER BY SignalId");
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));

	while (query.next() == true && tempQuery.next() == true)
	{
		while (tempQuery.value("checkedInInstanceId").toInt() == 0 && tempQuery.value("userId").toInt() != m_firstUserForTest)
		{
			tempQuery.next();
		}

		QVERIFY2(tempQuery.value("signalId").toInt() == query.value("signalId").toInt(), qPrintable(QString("Error in signal Id's: (function)%1 : (table)%2").arg(query.value("signalId").toInt()).arg(tempQuery.value("signalId").toInt())));

		if (tempQuery.value("userId").toInt() == 0 || tempQuery.value("userId").toInt() != m_firstUserForTest)
		{
			QVERIFY2(tempQuery.value("checkedInInstanceId").toInt() == query.value("signalInstanceId").toInt(), qPrintable("Error: wrong signalInstance Id"));
		}
		else
		{
			QVERIFY2(tempQuery.value("checkedOutInstanceId").toInt() == query.value("signalInstanceId").toInt(), qPrintable("Error: wrong signalInstance Id"));
		}
	}

	QVERIFY2(query.next() == false, qPrintable ("Error: some records were skipped"));

	// Try execute function with invalid user
	//

	ok = query.exec(QString("SELECT * FROM get_latest_signals_all (%1)").arg(maxValueId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("No records with invalid user expected")); // Better not give information for invalid user */
}

