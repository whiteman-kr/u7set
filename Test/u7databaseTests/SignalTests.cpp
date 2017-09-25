#include "SignalTests.h"
#include <assert.h>

QString SignalTests::SF_APP_SIGNAL_ID("AppSignalID");
QString SignalTests::SF_CUSTOM_APP_SIGNAL_ID("CustomAppSignalID");
QString SignalTests::SF_CAPTION("Caption");
QString SignalTests::SF_EQUIPMENT_ID("EquipmentID");
QString SignalTests::SF_BUS_TYPE_ID("BusTypeID");
QString SignalTests::SF_CHANNEL("Channel");

QString SignalTests::SF_SIGNAL_TYPE("SignalType");
QString SignalTests::SF_IN_OUT_TYPE("InOutType");

QString SignalTests::SF_DATA_SIZE("DataSize");
QString SignalTests::SF_BYTE_ORDER("ByteOrder");

QString SignalTests::SF_ANALOG_SIGNAL_FORMAT("AnalogSignalFormat");
QString SignalTests::SF_UNIT("Unit");

QString SignalTests::SF_LOW_ADC("LowADC");
QString SignalTests::SF_HIGH_ADC("HighADC");
QString SignalTests::SF_LOW_ENGENEERING_UNITS("LowEngeneeringUnits");
QString SignalTests::SF_HIGH_ENGENEERING_UNITS("HighEngeneeringUnits");
QString SignalTests::SF_LOW_VALID_RANGE("LowValidRange");
QString SignalTests::SF_HIGH_VALID_RANGE("HighValidRange");
QString SignalTests::SF_FILTERING_TIME("FilteringTime");
QString SignalTests::SF_SPREAD_TOLERANCE("SpreadTolerance");

QString SignalTests::SF_ELECTRIC_LOW_LIMIT("ElectricLowLimit");
QString SignalTests::SF_ELECTRIC_HIGH_LIMIT("ElectricHighLimit");
QString SignalTests::SF_ELECTRIC_UNIT("ElectricUnit");
QString SignalTests::SF_SENSOR_TYPE("SensorType");
QString SignalTests::SF_OUTPUT_MODE("OutputMode");

QString SignalTests::SF_ENABLE_TUNING("EnableTuning");
QString SignalTests::SF_TUNING_DEFAULT_VALUE("TuningDefaultValue");
QString SignalTests::SF_TUNING_LOW_BOUND("TuningLowBound");
QString SignalTests::SF_TUNING_HIGH_BOUND("TuningHighBound");

QString SignalTests::SF_ACQUIRE("Acquire");
QString SignalTests::SF_DECIMAL_PLACES("DecimalPlaces");
QString SignalTests::SF_COARSE_APERTURE("CoarseAperture");
QString SignalTests::SF_FINE_APERTURE("FineAperture");
QString SignalTests::SF_ADAPTIVE_APERTURE("AdaptiveAperture");

QString SignalTests::SF_SIGNAL_ID("SignalID");
QString SignalTests::SF_ID("ID");
QString SignalTests::SF_SIGNAL_GROUP_ID("SignalGroupID");
QString SignalTests::SF_SIGNAL_INSTANCE_ID("SignalInstanceID");
QString SignalTests::SF_CHANGESET_ID("ChangesetID");
QString SignalTests::SF_CHECKED_OUT("CheckedOut");
QString SignalTests::SF_USER_ID("UserID");
QString SignalTests::SF_CREATED("Created");
QString SignalTests::SF_DELETED("Deleted");
QString SignalTests::SF_INSTANCE_CREATED("InstanceCreated");
QString SignalTests::SF_INSTANCE_ACTION("InstanceAction");
QString SignalTests::SF_ACTION("Action");

QString SignalTests::SF_CHECKED_IN_INSTANCE_ID("CheckedInInstanceID");
QString SignalTests::SF_CHECKED_OUT_INSTANCE_ID("CheckedOutInstanceID");


SignalTests::SignalTests()
{
}

void SignalTests::initTestCase()
{
	QSqlQuery query;

	// Alter Administrator user. Set administrator password "123412341234"
	//

	bool ok = query.exec("SELECT salt FROM users WHERE username = 'Administrator'");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QString passwordHashQuery = QString("user_api.password_hash('%1', '%2')").arg(query.value(0).toString()).arg(m_adminPassword);

	ok = query.exec(QString("UPDATE users SET passwordhash = %1 WHERE username = 'Administrator'").arg(passwordHashQuery));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_adminPassword));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QString session_key = query.value(0).toString();

	// Testers
	//

	ok = query.exec(QString("SELECT user_api.create_user('%1', 'signalTestUser1', 'FIRSTTEST', 'FIRSTTEST', '12341234', false, false);").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	SignalTests::m_firstUserForTest = query.value("create_user").toInt();

	ok = query.exec(QString("SELECT user_api.create_user('%1', 'signalTestUser2', 'SECONDTEST', 'SECONDTEST', '12341234', false, false);").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	SignalTests::m_secondUserForTest = query.value("create_user").toInt();

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
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

		QVERIFY2(tempQuery.value("AppSignalID").toString() == QString("#SIGNAL" + query.value("id").toString() + "_" + char(int('A') + channelNumber)), qPrintable("Error: value AppSignalID in table signalInstance not match. " + QString("\n%1:::%2").arg(tempQuery.value("AppSignalID").toString()).arg(QString("#SIGNAL" + query.value("id").toString() + "_" + char(int('A') + channelNumber)))));
		QVERIFY2(tempQuery.value("CustomAppSignalID").toString() == QString("SIGNAL" + query.value("id").toString() + "_" + char(int('A') + channelNumber)), qPrintable("Error: value CustomAppSignalID in table signalInstance not match"));
		QVERIFY2(tempQuery.value("caption").toString() == QString("SIGNAL" + query.value("id").toString() + "_" + char(int('A') + channelNumber)), qPrintable("Error: value name in table signalInstance not match"));

		// Check data size to be correct
		//

		if (signalType == 0)
			QVERIFY2(tempQuery.value("dataSize").toInt() == 16, qPrintable("Error: wrong dataSize in table signalInstance"));
		else
			QVERIFY2(tempQuery.value("dataSize").toInt() == 0, qPrintable("Error: wrong dataSize in table signalInstance"));
		QVERIFY2(tempQuery.value("action").toInt() == 1, qPrintable("Error: value action in table signalInstance not match"));
		QVERIFY2(tempQuery.value("signalInstanceId").toInt() == checkOutInstanceId, qPrintable("Error: wrong checkedOutInstanceId"));

		// Set right channel number for every signal (actually - number of the signal)
		//

		channelNumber += 1;
	}

	// Check count of channels must be equal channel count
	//

	QVERIFY2(channelNumber == channelCount, qPrintable(QString("Error: number of channels is not similar to channelCount value. Actual: %1\tExpected: %2").arg(channelCount).arg(channelNumber)));

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

	channelNumber = 0;
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

	QVERIFY2(tempQuery.value("AppSignalID").toString() == QString("#SIGNAL" + query.value("id").toString()), qPrintable("Error: value AppSignalID in table signalInstance not match"));
	QVERIFY2(tempQuery.value("CustomAppSignalID").toString() == QString("SIGNAL" + query.value("id").toString()), qPrintable("Error: value CustomAppSignalID in table signalInstance not match"));
	QVERIFY2(tempQuery.value("caption").toString() == QString("SIGNAL" + query.value("id").toString()), qPrintable("Error: value name in table signalInstance not match"));
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

			QVERIFY2(tempQuery.value("checkedOutInstanceId").toInt() == 0, qPrintable("Error: no checkedOutInstanceId expected"));
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

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE signalId = %1").arg(signalIds[3]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	Signal sd;

	readSignalFromQuery(query, sd, EXM_SIGNAL_TABLE_FIELDS);

	sd.setID(signalIds[3]);
	sd.setInstanceAction(VcsItemAction::VcsItemActionType::Modified);		// == 2

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

			QVERIFY2(query.value("errCode").toInt() == 4, qPrintable("Signal not found error expected"));
			break;

		case 1:	// Deleted signal id

			QVERIFY2(query.value("errCode").toInt() == 3, qPrintable("Signal has been deleted error expected"));
			break;

		case 2:	// Already checked out signal id

			QVERIFY2(query.value("errCode").toInt() == 2, qPrintable("Signal already checked out error expected"));
			break;

		case 3:	// Ordinary signal id
			{
				QVERIFY2(query.value("errCode").toInt() == 0, qPrintable("Error code must be 0"));

				tempQuery.exec(QString("SELECT * FROM checkOut WHERE signalId = %1").arg(signalIds[currentSignalId]));
				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value(SF_USER_ID).toInt() == userId, qPrintable(QString("Error: userId is not match in table checkOut (signalId %1)").arg(signalIds[currentSignalId])));

				tempQuery.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(signalIds[currentSignalId]));
				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.value(SF_CHECKED_IN_INSTANCE_ID).toInt() != 0, qPrintable("Error: no record in column checkedInInstanceId expected"));

				int checkOutId = tempQuery.value(SF_CHECKED_OUT_INSTANCE_ID).toInt();

				QVERIFY2(tempQuery.value(SF_USER_ID).toInt() == userId, qPrintable(QString("Error: userId is not match in table signal (signalId %1)").arg(signalIds[currentSignalId])));

				tempQuery.exec(QString("SELECT * FROM signalInstance WHERE signalInstanceId = %1 AND changeSetId is NULL").arg(checkOutId));
				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				sd.setSignalInstanceID(checkOutId);

				verifyQueryAndSignal(tempQuery, sd, EXM_SIGNAL_TABLE_FIELDS);

			}
			break;
		}

		currentSignalId++;
	}

	// Try to checkOut chekcouted signal by another user; If error occured - check returned userId
	//

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 0)").arg(userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value(SF_ID).toInt();

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
	QSqlQuery query;
	QSqlQuery tempQuery;

	// Create signal to test
	//

	bool ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 0)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value(SF_ID).toInt();

	// Get info about this signal by user, who created it
	//

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE signalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	Signal sd;

	readSignalFromQuery(query, sd, EXM_SIGNAL_TABLE_FIELDS);

	sd.setID(signalId);
	sd.setCheckedOut(true);			// waiting TRUE
	sd.setUserID(m_firstUserForTest);

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	verifyQueryAndSignal(query, sd, EXM_CREATED | EXM_INSTANCE_CREATED);

	ok = tempQuery.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	// Check info
	//

	QVERIFY2(query.value(SF_SIGNAL_INSTANCE_ID).toInt() == tempQuery.value(SF_CHECKED_OUT_INSTANCE_ID).toInt(), qPrintable("Error: signalInstanceId is wrong"));
	QVERIFY2(signalId == query.value(SF_SIGNAL_ID).toInt(), qPrintable("Error: wrong sigalId"));

	// Try get info about new, do not checked in signal from another user
	//

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(m_secondUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("Error: no record expected"));

	// Check this signal in and test info about it with both users
	//

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'TEST');").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT checkedInInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value(SF_SIGNAL_INSTANCE_ID).toInt() == tempQuery.value(SF_CHECKED_IN_INSTANCE_ID).toInt(), qPrintable("Error: signalInstanceId is wrong"));
	QVERIFY2(signalId == query.value(SF_SIGNAL_ID).toInt(), qPrintable("Error: wrong sigalId"));

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(m_secondUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT checkedInInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value(SF_SIGNAL_INSTANCE_ID).toInt() == tempQuery.value(SF_CHECKED_IN_INSTANCE_ID).toInt(), qPrintable("Error: signalInstanceId is wrong"));
	QVERIFY2(signalId == query.value(SF_SIGNAL_ID).toInt(), qPrintable("Error: wrong sigalId"));

	// Delete signal, and test info about deleted signal, which was not checked in
	//

	ok = query.exec(QString("SELECT * FROM delete_signal(%1, %2);").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	// First user, who delete this signal, must see checked out version of the signal
	//

	QVERIFY2(query.value(SF_SIGNAL_INSTANCE_ID).toInt() == tempQuery.value(SF_CHECKED_OUT_INSTANCE_ID).toInt(), qPrintable("Error: signalInstanceId is wrong"));
	QVERIFY2(signalId == query.value(SF_SIGNAL_ID).toInt(), qPrintable("Error: wrong sigalId"));
	QVERIFY2(static_cast<VcsItemAction::VcsItemActionType>(query.value(SF_INSTANCE_ACTION).toInt()) ==
								VcsItemAction::VcsItemActionType::Deleted, qPrintable("Error: Wrong record!"));

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(m_secondUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT checkedInInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	// All another must see checked in version of the signal
	//

	QVERIFY2(query.value(SF_SIGNAL_INSTANCE_ID).toInt() == tempQuery.value(SF_CHECKED_IN_INSTANCE_ID).toInt(), qPrintable("Error: signalInstanceId is wrong"));
	QVERIFY2(signalId == query.value(SF_SIGNAL_ID).toInt(), qPrintable("Error: wrong sigalId"));
	QVERIFY2(static_cast<VcsItemAction::VcsItemActionType>(query.value(SF_INSTANCE_ACTION).toInt()) ==
			 VcsItemAction::VcsItemActionType::Added, qPrintable("Error: Wrong record!"));

	// Nobody must see deleted signal
	//

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'TEST')").arg(1).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2)").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("Error: no information must be for deleted signalId"));

	// Try to get info about invalid signal
	//

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2)").arg(1).arg(maxValueId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("Error: no information must be for invalid signalId"));

	// Try to get info about signal with invalid user
	//

	ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2)").arg(maxValueId).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("Error: no information must be for invalid User"));
}

void SignalTests::get_latest_signalsTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	int signalIds[4] = {-1, -1, -1, -1};
	int numberOfSignal = 0;

	// Value, that will be changed by another user to test checkOuted signals
	//

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

	/*ok = query.exec(QString("UPDATE signalInstance SET name = '%1' WHERE changeSetId IS NULL AND signalId = %2").arg(nameToChange).arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));*/

	ok = query.exec(QString("SELECT * FROM delete_signal(%1, %2);").arg(m_secondUserForTest).arg(signalIds[0]));
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

	/*ok = query.exec(QString("UPDATE signalInstance SET name = '%1' WHERE changeSetId IS NULL AND signalId = %2").arg(nameToChange).arg(signalIds[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));*/

	ok = query.exec(QString("SELECT * FROM delete_signal(%1, %2);").arg(m_firstUserForTest).arg(signalIds[2]));
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
				QVERIFY2(query.value(SF_SIGNAL_ID).toInt() == signalIds[numberOfSignal], qPrintable("Error: wrong signalId"));

				ok = tempQuery.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(signalIds[numberOfSignal]));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value(SF_CHECKED_IN_INSTANCE_ID).toInt() == query.value(SF_SIGNAL_INSTANCE_ID), qPrintable ("Error: wrong signalinstance in wrong userId with checkOuted signal"));

				ok = tempQuery.exec(QString("SELECT * FROM SignalInstance WHERE signalInstanceId = %1").arg(tempQuery.value("checkedInInstanceId").toInt()));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value(SF_SIGNAL_ID).toInt() == signalIds[numberOfSignal], qPrintable("Error: wrong signalId in signalInstance"));
				QVERIFY2(static_cast<VcsItemAction::VcsItemActionType>(tempQuery.value(SF_ACTION).toInt()) !=
						VcsItemAction::VcsItemActionType::Deleted, qPrintable("Error: wrong action in wrong userId with checkOuted signal"));

				numberOfSignal++;
			}
			break;

		case 1:
			{
				QVERIFY2(query.value(SF_SIGNAL_ID).toInt() == signalIds[numberOfSignal], qPrintable("Error: wrong signalId"));

				ok = tempQuery.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(signalIds[numberOfSignal]));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value(SF_CHECKED_IN_INSTANCE_ID).toInt() == query.value(SF_SIGNAL_INSTANCE_ID), qPrintable ("Error: wrong signalinstance in wrong userId with checkOuted signal"));

				numberOfSignal++;
			}
			break;

		case 2:
			{
				QVERIFY2(query.value(SF_SIGNAL_ID).toInt() == signalIds[numberOfSignal], qPrintable("Error: wrong signalId"));

				ok = tempQuery.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(signalIds[numberOfSignal]));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value(SF_CHECKED_OUT_INSTANCE_ID).toInt() == query.value(SF_SIGNAL_INSTANCE_ID), qPrintable ("Error: wrong signalinstance in wrong userId with checkOuted signal"));

				ok = tempQuery.exec(QString("SELECT * FROM SignalInstance WHERE signalInstanceId = %1").arg(tempQuery.value("checkedOutInstanceId").toInt()));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2(tempQuery.value(SF_SIGNAL_ID).toInt() == signalIds[numberOfSignal], qPrintable("Error: wrong signalId in signalInstance"));
				QVERIFY2(static_cast<VcsItemAction::VcsItemActionType>(tempQuery.value(SF_ACTION).toInt()) ==
										 VcsItemAction::VcsItemActionType::Deleted, qPrintable("Error: wrong action in wrong userId with checkOuted signal"));

				numberOfSignal++;
			}
			break;

		case 3:
			QFAIL("Error: There must be only 3 records");
			break;
		}
	}

	// Check in edited signal by second user, and check it by function with first user
	//

	ok = query.exec("SELECT * FROM add_signal (1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value(SF_ID).toInt();

	ok = query.exec(QString("SELECT * FROM checkin_signals (%1, '{%2}', '%3')").arg(1).arg(signalId).arg("TEST"));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals (%1, '{%2}')").arg(m_secondUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	const QString nameToChange = "TEST";

	ok = query.exec(QString("UPDATE signalInstance SET caption = '%1' WHERE changeSetId IS NULL AND signalId = %2").arg(nameToChange).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals (%1, '{%2}', '%3')").arg(m_secondUserForTest).arg(signalId).arg("TEST"));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_latest_signals(%1, '{%2}')").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value(SF_CAPTION).toString() == nameToChange, qPrintable("Error: function returns wrong name after function checkin_signals()"));

	// Check deleted signal. Nobody must see deleted signal
	//

	ok = query.exec(QString("SElECT * FROM checkin_signals(%1, '{%2}', 'TEST')").arg(1).arg(signalIds[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_latest_signals(%1, '{%2}');").arg(m_firstUserForTest).arg(signalIds[2]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Error: no record expected (deleted signal)"));

	// Try start function with invalid user
	//

	ok = query.exec(QString("SELECT * FROM get_latest_signals(%1, '{1}')").arg(maxValueId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("No record expected, invalid userId"));

	// Try start function with invalid signal
	//

	ok = query.exec(QString("SELECT * FROM get_latest_signals(1, '{%1}')").arg(maxValueId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("No record expected, invalid userId"));

	// Check All info from function
	//

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 0)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalID = query.value(SF_ID).toInt();

	Signal sd;

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE signalId=%1").arg(signalID));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	readSignalFromQuery(query, sd, EXM_SIGNAL_TABLE_FIELDS);

	sd.setID(signalID);
	sd.setCheckedOut(true);
	sd.setUserID(m_firstUserForTest);

	ok = query.exec(QString("SELECT * FROM get_latest_signals(%1, '{%2}')").arg(m_firstUserForTest).arg(sd.ID()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	verifyQueryAndSignal(query, sd, EXM_CREATED | EXM_INSTANCE_CREATED);
}

void SignalTests::get_latest_signals_allTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	int deletedSignalId = query.value(SF_ID).toInt();

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(deletedSignalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_signal (1, %1)").arg(deletedSignalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(deletedSignalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec("SELECT * FROM get_latest_signals_all(1)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec("SELECT * FROM Signal WHERE Deleted = false ORDER BY SignalId");
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));

	while (query.next() == true && tempQuery.next() == true)
	{
		QVERIFY2(query.value(SF_SIGNAL_ID).toInt() != deletedSignalId, qPrintable("Error: deleted signal was not expected"));
		QVERIFY2(tempQuery.value(SF_SIGNAL_ID).toInt() == query.value(SF_SIGNAL_ID).toInt(), qPrintable(QString("%1:%2").arg(query.value(SF_SIGNAL_ID).toInt()).arg(tempQuery.value(SF_SIGNAL_ID).toInt())));

		if (tempQuery.value(SF_USER_ID).toInt() == 0)
		{
			QVERIFY2(tempQuery.value(SF_CHECKED_IN_INSTANCE_ID).toInt() == query.value(SF_SIGNAL_INSTANCE_ID).toInt(), qPrintable("Error: wrong signalInstance Id"));
		}
		else
		{
			QVERIFY2(tempQuery.value(SF_CHECKED_OUT_INSTANCE_ID).toInt() == query.value(SF_SIGNAL_INSTANCE_ID).toInt(), qPrintable("Error: wrong signalInstance Id"));
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
	QVERIFY2(query.next() == false, qPrintable("No records with invalid user expected"));

	// Check all info

	ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE signalId=%1").arg(query.value("id").toInt()));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	Signal sd;

	readSignalFromQuery(query, sd, 0);

	ok = query.exec("SELECT * FROM get_latest_signals_all(1) ORDER BY signalId DESC");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	verifyQueryAndSignal(query, sd, 0);

/*
	QVERIFY2(query.value("signalId").toInt() == sd.signalId, qPrintable(QString("Error: signalId is wrong in table signalInstance (signalId %1, got %2)").arg(sd.signalId).arg(query.value("signalId").toInt())));
	QVERIFY2(query.value("changeSetId").toInt() == sd.changeSetId, qPrintable(QString("Error: changeSetId is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("action").toInt() == sd.action, qPrintable(QString("Error: action is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("appSignalID").toString() == sd.appSignalID, qPrintable(QString("Error: strId is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("customAppSignalID").toString() == sd.customAppSignalID, qPrintable(QString("Error: extStrId is wrong in table signalInstance (signalId %1) %2, %3").arg(sd.signalId)));
	QVERIFY2(query.value("caption").toString() == sd.caption, qPrintable(QString("Error: caption is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("analogSignalFormat").toInt() == sd.analogSignalFormat, qPrintable(QString("Error: analogSignalFormat is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("dataSize").toInt() == sd.dataSize, qPrintable(QString("Error: dataSize is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("lowAdc").toInt() == sd.lowAdc, qPrintable(QString("Error: lowAdc is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("highAdc").toInt() == sd.highAdc, qPrintable(QString("Error: highAdc is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("lowengeneeringunits").toDouble() == sd.lowengeneeringunits, qPrintable(QString("Error: lowengeneeringunits is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("highengeneeringunits").toDouble() == sd.highengeneeringunits, qPrintable(QString("Error: highengeneeringunits is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("unitId").toInt() == sd.unit, qPrintable(QString("Error: unitId is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("adjustment").toDouble() == sd.adjustment, qPrintable(QString("Error: adjustment is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("lowvalidrange").toDouble() == sd.lowvalidrange, qPrintable(QString("Error: lowvalidrange is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("highvalidrange").toDouble() == sd.highvalidrange, qPrintable(QString("Error: highvalidrange is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("unbalanceLimit").toDouble() == sd.unbalanceLimit, qPrintable(QString("Error: unbalanceLimit is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("inputLowLimit").toDouble() == sd.electricLowLimit, qPrintable(QString("Error: inputLowLimit is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("inputHighLimit").toDouble() == sd.electricHighLimit, qPrintable(QString("Error: inputHighLimit is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("inputUnitId").toInt() == sd.electricUnit, qPrintable(QString("Error: inputUnitId is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("inputSensorId").toInt() == sd.sensorType, qPrintable(QString("Error: inputSensorId is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("outputLowLimit").toDouble() == sd.outputLowLimit, qPrintable(QString("Error: outputLowLimit is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("outputHighLimit").toDouble() == sd.outputHighLimit, qPrintable(QString("Error: outputHighLimit is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("outputUnitId").toInt() == sd.outputUnitId, qPrintable(QString("Error: outputUnitId is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("outputSensorId").toInt() == sd.outputSensorId, qPrintable(QString("Error: outputSensorId is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("acquire").toString() == sd.acquire, qPrintable(QString("Error: acquire is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("calculated").toString() == sd.calculated, qPrintable(QString("Error: calculated is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("normalState").toInt() == sd.normalState, qPrintable(QString("Error: normalState is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("decimalPlaces").toInt() == sd.decimalPlaces, qPrintable(QString("Error: decimalPlaces is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("aperture").toDouble() == sd.coarseAperture, qPrintable(QString("Error: aperture is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("inOutType").toInt() == sd.inOutType, qPrintable(QString("Error: inOutType is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("equipmentID").toString() == sd.equipmentID, qPrintable(QString("Error: deviceStrId is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("outputRangeMode").toInt() == sd.outputMode, qPrintable(QString("Error: outputRangeMode is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("filteringTime").toDouble() == sd.filteringTime, qPrintable(QString("Error: filteringTime is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("spreadtolerance").toDouble() == sd.spreadtolerance, qPrintable(QString("Error: spreadtolerance is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("byteOrder").toInt() == sd.byteOrder, qPrintable(QString("Error: byteOrder is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("enableTuning").toString() == sd.enableTuning, qPrintable(QString("Error: enableTuning is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));
	QVERIFY2(query.value("tuningDefaultValue").toDouble() == sd.tuningDefaultValue, qPrintable(QString("Error: tuningDefaultValue is wrong in table signalInstance (signalId %1)").arg(sd.signalId)));*/
}

void SignalTests::undo_signal_changesTest()
{
	QSqlQuery query;

	int signalId = -1;
	int signalGroupId = -1;

	bool ok = query.exec ("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Call signal was not checked out error
	//

	ok = query.exec(QString("SELECT * FROM undo_signal_changes(1, %1);").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("errCode").toInt() == 1, qPrintable ("Expected error: ERR_SIGNAL_IS_NOT_CHECKED_OUT"));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_signal(1, %1)").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Call wrong user error
	//

	ok = query.exec(QString("SELECT * FROM undo_signal_changes(%1, %2)").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("errCode").toInt() == 2, qPrintable ("Expected error: ERR_SIGNAL_ALREADY_CHECKED_OUT"));

	// Remember checkedOutInstanceId of the signal, to check deleted record from signalInstance
	//

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE signalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int deletedCheckedOutInstanceId = query.value(0).toInt();

	ok = query.exec(QString("SELECT * FROM undo_signal_changes(1, %1)").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("errCode").toInt() == 0, qPrintable ("Expected error: ERR_SIGNAL_ALREADY_CHECKED_OUT"));

	// Check changes in Signal table
	//

	ok = query.exec(QString("SELECT * FROM signal WHERE signalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("checkedOutInstanceId").toInt() == 0, qPrintable("Error: function not changed Signal table (value checkedOutInstanceId is not NULL"));
	QVERIFY2(query.value("userId").toInt() == 0, qPrintable("Error: function not changed Signal table (value userId is not NULL)"));

	ok = query.exec(QString("SELECT * FROM SignalInstance WHERE signalInstanceId = %1").arg(deletedCheckedOutInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("Error: record from signalInstance was not deleted"));

	ok = query.exec(QString("SELECT * FROM checkOut WHERE signalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("No record expected"));

	// Check signalGroupId to be deleted in case no checked in signal, and no more signals
	// with same signalGroupId
	//

	int signalIds[2] = {-1, -1};
	int numberOfSignal=0;

	ok = query.exec ("SELECT * FROM add_signal(1, 0, 2)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	for (int i=0; i<2; i++)
	{
		QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
		signalIds[numberOfSignal] = query.value("id").toInt();
		numberOfSignal++;
	}

	ok = query.exec(QString("SELECT signalGroupId FROM signal WHERE signalId = %1").arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalGroupId = query.value(0).toInt();

	ok = query.exec(QString("SELECT * FROM undo_signal_changes(1, %1)").arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM undo_signal_changes(1, %1)").arg(signalIds[1]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT COUNT(*) FROM signalGroup WHERE signalGroupId = %1").arg(signalGroupId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value(0).toInt() == 0, qPrintable ("Error: signalGroupId of signal must be deleted"));

	ok = query.exec(QString("SELECT * FROM Signal WHERE SignalId = %1 OR SignalId = %2").arg(signalIds[0]).arg(signalIds[1]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("Error: must be no records"));

	// Test undo_signal_changes. Signal checkedOut by user, function started by admin
	//

	ok = query.exec("SELECT * FROM add_signal(1,0,0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("Id").toInt();

	ok = query.exec(QString("SELECT * FROM checkin_signals (1, '{%1}', 'TEST')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkOut_signals (%1, '{%2}')").arg(m_firstUserForTest).arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM undo_signal_changes(1, %1)").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("errCode").toInt() == 0, qPrintable("Error: function must work from admin on user's checked out signals"));
}

void SignalTests::set_signal_workcopyTest()
{
	QSqlQuery query;
	Signal sd;

	// Create signal with history
	//

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	// Remember signalId, to check function work
	//

	int signalID = query.value(SF_SIGNAL_ID).toInt();

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalID));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalID));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Get checkedOutInstanceId from signal to test function
	//

	ok = query.exec(QString("SELECT * FROM Signal WHERE signalId = %1").arg(signalID));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	// Values, that not used in function, but required
	//
/*
	sd.signalGroupId = 0;
	sd.changeSetId = 0;
	sd.checkedOut = "false";
	sd.userId = 0;
	sd.channel = 0;
	sd.type =0;
	sd.created = "2015-09-15 00:00:00.811+03";
	sd.deleted = "false";
	sd.instanceCreated = "2015-09-15 00:00:00.811+03";
	sd.action = 0;

	// Fill values that function use
	//

	sd.signalInstanceId = query.value("checkedOutInstanceId").toInt();
	sd.appSignalID = "appSignalID";
	sd.customAppSignalID = "customAppSignalID";
	sd.caption = "name";
	sd.analogSignalFormat = 1;
	sd.dataSize = 2;
	sd.lowAdc = 3;
	sd.highAdc = 4;
	sd.lowengeneeringunits = 5;
	sd.highengeneeringunits = 6;
	sd.unit = 7;
	sd.adjustment = 8;
	sd.lowvalidrange = 9;
	sd.highvalidrange = 10;
	sd.unbalanceLimit = 11;
	sd.electricLowLimit = 12;
	sd.electricHighLimit = 13;
	sd.electricUnit = 14;
	sd.sensorType = 15;
	sd.outputLowLimit = 16;
	sd.outputHighLimit = 17;
	sd.outputUnitId = 18;
	sd.outputSensorId = 19;
	sd.acquire = "true";
	sd.calculated = "true";
	sd.normalState = 20;
	sd.decimalPlaces = 21;
	sd.coarseAperture = 22;
	sd.inOutType = 23;
	sd.equipmentID = "equipmentID";
	sd.outputMode = 24;
	sd.filteringTime = 25;
	sd.spreadtolerance = 26;
	sd.byteOrder = 27;
	sd.enableTuning = "true";
	sd.tuningDefaultValue = 89.234;

	QString arguments = QString("%1, %2, %3, %4, %5, %6, %7, %8, '%9', %10, ")
	        .arg(sd.signalId)
	        .arg(sd.signalGroupId)
	        .arg(sd.changeSetId)
	        .arg(sd.signalInstanceId)
	        .arg(sd.checkedOut)
	        .arg(sd.userId)
	        .arg(sd.channel)
	        .arg(sd.type)
	        .arg(sd.created)
	        .arg(sd.deleted);

	arguments.append(QString("'%1', %2, '%3', '%4', '%5', %6, %7, %8, %9, %10, ")
	                 .arg(sd.instanceCreated)
	                 .arg(sd.action)
	                 .arg(sd.appSignalID)
	                 .arg(sd.customAppSignalID)
	                 .arg(sd.caption)
					 .arg(sd.analogSignalFormat)
	                 .arg(sd.dataSize)
	                 .arg(sd.lowAdc)
	                 .arg(sd.highAdc)
	                 .arg(sd.lowengeneeringunits));

	arguments.append(QString("%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, ")
	                 .arg(sd.highengeneeringunits)
					 .arg(sd.unit)
	                 .arg(sd.adjustment)
	                 .arg(sd.lowvalidrange)
	                 .arg(sd.highvalidrange)
	                 .arg(sd.unbalanceLimit)
					 .arg(sd.electricLowLimit)
					 .arg(sd.electricHighLimit)
					 .arg(sd.electricUnit)
					 .arg(sd.sensorType));

	arguments.append(QString("%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, ")
	                 .arg(sd.outputLowLimit)
	                 .arg(sd.outputHighLimit)
	                 .arg(sd.outputUnitId)
	                 .arg(sd.outputSensorId)
	                 .arg(sd.acquire)
	                 .arg(sd.calculated)
	                 .arg(sd.normalState)
	                 .arg(sd.decimalPlaces)
					 .arg(sd.coarseAperture)
	                 .arg(sd.inOutType));

	arguments.append(QString("'%1', %2, %3, %4, %5, '%6', %7")
	                 .arg(sd.equipmentID)
					 .arg(sd.outputMode)
	                 .arg(sd.filteringTime)
	                 .arg(sd.spreadtolerance)
	                 .arg(sd.byteOrder)
	                 .arg(sd.enableTuning)
	                 .arg(sd.tuningDefaultValue));

	// Start function
	//

	ok = query.exec(QString("SELECT * FROM set_signal_workcopy(1, ROW(%1))").arg(arguments));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	// Check errCode is 0
	//

	QVERIFY2(query.value("ErrCode").toInt() == 0, qPrintable ("Error: error code is not 0!"));

	// Check all data from table signalInstance
	//

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE signalInstanceId = %1").arg(sd.signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("signalId").toInt() == sd.signalId, qPrintable(QString("Error: value signalId is not match (Actual: %1, Expected: %2)").arg(query.value("signalId").toInt()).arg(sd.signalId)));
	QVERIFY2(query.value("appSignalID").toString() == sd.appSignalID, qPrintable(QString("Error: value appSignalID is not match (Actual: %1, Expected: %2)").arg(query.value("appSignalID").toString()).arg(sd.appSignalID)));
	QVERIFY2(query.value("customAppSignalID").toString() == sd.customAppSignalID, qPrintable(QString("Error: value customAppSignalID is not match (Actual: %1, Expected: %2)").arg(query.value("customAppSignalID").toString()).arg(sd.customAppSignalID)));
	QVERIFY2(query.value("caption").toString() == sd.caption, qPrintable(QString("Error: value name is not match (Actual: %1, Expected: %2)").arg(query.value("caption").toString()).arg(sd.caption)));
	QVERIFY2(query.value("analogSignalFormat").toInt() == sd.analogSignalFormat, qPrintable(QString("Error: value analogSignalFormat is not match (Actual: %1, Expected: %2)").arg(query.value("analogSignalFormat").toInt()).arg(sd.analogSignalFormat)));
	QVERIFY2(query.value("dataSize").toInt() == sd.dataSize, qPrintable(QString("Error: value dataSize is not match (Actual: %1, Expected: %2)").arg(query.value("dataSize").toInt()).arg(sd.dataSize)));
	QVERIFY2(query.value("lowAdc").toInt() == sd.lowAdc, qPrintable(QString("Error: value lowAdc is not match (Actual: %1, Expected: %2)").arg(query.value("lowAdc").toInt()).arg(sd.lowAdc)));
	QVERIFY2(query.value("highAdc").toInt() == sd.highAdc, qPrintable(QString("Error: value highAdc is not match (Actual: %1, Expected: %2)").arg(query.value("highAdc").toInt()).arg(sd.highAdc)));
	QVERIFY2(query.value("lowengeneeringunits").toInt() == sd.lowengeneeringunits, qPrintable(QString("Error: value lowengeneeringunits is not match (Actual: %1, Expected: %2)").arg(query.value("lowengeneeringunits").toInt()).arg(sd.lowengeneeringunits)));
	QVERIFY2(query.value("highengeneeringunits").toInt() == sd.highengeneeringunits, qPrintable(QString("Error: value highengeneeringunits is not match (Actual: %1, Expected: %2)").arg(query.value("highengeneeringunits").toInt()).arg(sd.highengeneeringunits)));
	QVERIFY2(query.value("unitId").toInt() == sd.unit, qPrintable(QString("Error: value unitId is not match (Actual: %1, Expected: %2)").arg(query.value("unitId").toInt()).arg(sd.unit)));
	QVERIFY2(query.value("adjustment").toInt() == sd.adjustment, qPrintable(QString("Error: value adjustment is not match (Actual: %1, Expected: %2)").arg(query.value("adjustment").toInt()).arg(sd.adjustment)));
	QVERIFY2(query.value("lowvalidrange").toInt() == sd.lowvalidrange, qPrintable(QString("Error: value lowvalidrange is not match (Actual: %1, Expected: %2)").arg(query.value("lowvalidrange").toInt()).arg(sd.lowvalidrange)));
	QVERIFY2(query.value("highvalidrange").toInt() == sd.highvalidrange, qPrintable(QString("Error: value highvalidrange is not match (Actual: %1, Expected: %2)").arg(query.value("highvalidrange").toInt()).arg(sd.highvalidrange)));
	QVERIFY2(query.value("unbalanceLimit").toInt() == sd.unbalanceLimit, qPrintable(QString("Error: value unbalanceLimit is not match (Actual: %1, Expected: %2)").arg(query.value("unbalanceLimit").toInt()).arg(sd.unbalanceLimit)));
	QVERIFY2(query.value("inputLowLimit").toInt() == sd.electricLowLimit, qPrintable(QString("Error: value inputLowLimit is not match (Actual: %1, Expected: %2)").arg(query.value("inputLowLimit").toInt()).arg(sd.electricLowLimit)));
	QVERIFY2(query.value("inputHighLimit").toInt() == sd.electricHighLimit, qPrintable(QString("Error: value inputHighLimit is not match (Actual: %1, Expected: %2)").arg(query.value("inputHighLimit").toInt()).arg(sd.electricHighLimit)));
	QVERIFY2(query.value("inputUnitId").toInt() == sd.electricUnit, qPrintable(QString("Error: value inputUnitId is not match (Actual: %1, Expected: %2)").arg(query.value("inputUnitId").toInt()).arg(sd.electricUnit)));
	QVERIFY2(query.value("inputSensorId").toInt() == sd.sensorType, qPrintable(QString("Error: value inputSensorId is not match (Actual: %1, Expected: %2)").arg(query.value("inputSensorId").toInt()).arg(sd.sensorType)));
	QVERIFY2(query.value("outputLowLimit").toInt() == sd.outputLowLimit, qPrintable(QString("Error: value outputLowLimit is not match (Actual: %1, Expected: %2)").arg(query.value("outputLowLimit").toInt()).arg(sd.outputLowLimit)));
	QVERIFY2(query.value("outputHighLimit").toInt() == sd.outputHighLimit, qPrintable(QString("Error: value outputHighLimit is not match (Actual: %1, Expected: %2)").arg(query.value("outputHighLimit").toInt()).arg(sd.outputHighLimit)));
	QVERIFY2(query.value("outputUnitId").toInt() == sd.outputUnitId, qPrintable(QString("Error: value outputUnitId is not match (Actual: %1, Expected: %2)").arg(query.value("outputUnitId").toInt()).arg(sd.outputUnitId)));
	QVERIFY2(query.value("outputSensorId").toInt() == sd.outputSensorId, qPrintable(QString("Error: value outputSensorId is not match (Actual: %1, Expected: %2)").arg(query.value("outputSensorId").toInt()).arg(sd.outputSensorId)));
	QVERIFY2(query.value("acquire").toString() == sd.acquire, qPrintable(QString("Error: value acquire is not match (Actual: %1, Expected: %2)").arg(query.value("acquire").toString()).arg(sd.acquire)));
	QVERIFY2(query.value("calculated").toString() == sd.calculated, qPrintable(QString("Error: value calculated is not match (Actual: %1, Expected: %2)").arg(query.value("calculated").toString()).arg(sd.calculated)));
	QVERIFY2(query.value("normalState").toInt() == sd.normalState, qPrintable(QString("Error: value normalState is not match (Actual: %1, Expected: %2)").arg(query.value("normalState").toInt()).arg(sd.normalState)));
	QVERIFY2(query.value("decimalPlaces").toInt() == sd.decimalPlaces, qPrintable(QString("Error: value decimalPlaces is not match (Actual: %1, Expected: %2)").arg(query.value("decimalPlaces").toInt()).arg(sd.decimalPlaces)));
	QVERIFY2(query.value("aperture").toInt() == sd.coarseAperture, qPrintable(QString("Error: value aperture is not match (Actual: %1, Expected: %2)").arg(query.value("aperture").toInt()).arg(sd.coarseAperture)));
	QVERIFY2(query.value("inOutType").toInt() == sd.inOutType, qPrintable(QString("Error: value inOutType is not match (Actual: %1, Expected: %2)").arg(query.value("inOutType").toInt()).arg(sd.inOutType)));
	QVERIFY2(query.value("equipmentID").toString() == sd.equipmentID, qPrintable(QString("Error: value equipmentID is not match (Actual: %1, Expected: %2)").arg(query.value("equipmentID").toString()).arg(sd.equipmentID)));
	QVERIFY2(query.value("outputRangeMode").toInt() == sd.outputMode, qPrintable(QString("Error: value outputRangeMode is not match (Actual: %1, Expected: %2)").arg(query.value("outputRangeMode").toInt()).arg(sd.outputMode)));
	QVERIFY2(query.value("filteringTime").toInt() == sd.filteringTime, qPrintable(QString("Error: value filteringTime is not match (Actual: %1, Expected: %2)").arg(query.value("filteringTime").toInt()).arg(sd.filteringTime)));
	QVERIFY2(query.value("spreadtolerance").toInt() == sd.spreadtolerance, qPrintable(QString("Error: value spreadtolerance is not match (Actual: %1, Expected: %2)").arg(query.value("spreadtolerance").toInt()).arg(sd.spreadtolerance)));
	QVERIFY2(query.value("byteOrder").toInt() == sd.byteOrder, qPrintable(QString("Error: value byteOrder is not match (Actual: %1, Expected: %2)").arg(query.value("byteOrder").toInt()).arg(sd.byteOrder)));
	QVERIFY2(query.value("enableTuning").toString() == sd.enableTuning, qPrintable(QString("Error: value enableTuning is not match (Actual: %1, Expected: %2)").arg(query.value("enableTuning").toString()).arg(sd.enableTuning)));
	QVERIFY2(query.value("tuningDefaultValue").toDouble() == sd.tuningDefaultValue, qPrintable(QString("Error: value tuningDefaultValue is not match (Actual: %1, Expected: %2)").arg(query.value("tuningDefaultValue").toDouble()).arg(sd.tuningDefaultValue)));

	// Call checked out by another user error
	//

	ok = query.exec(QString("SELECT * FROM set_signal_workcopy(%1, ROW(%2))").arg(m_firstUserForTest).arg(arguments));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("ErrCode").toInt() == 2, qPrintable ("Expected ERR_SIGNAL_ALREADY_CHECKED_OUT"));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(sd.signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Try invalid userId
	//

	ok = query.exec(QString("SELECT * FROM set_signal_workcopy(%1, ROW(%2))").arg(maxValueId).arg(arguments));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("ErrCode").toInt() == 1, qPrintable ("Expected ERR_SIGNAL_IS_NOT_CHECKED_OUT"));

	// Call signal is not checked out error
	//

	ok = query.exec(QString("SELECT * FROM set_signal_workcopy(1, ROW(%1))").arg(arguments));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("ErrCode").toInt() == 1, qPrintable ("Expected ERR_SIGNAL_IS_NOT_CHECKED_OUT"));*/

}

void SignalTests::get_signal_Ids_with_appsignalIdTest()
{
	QSqlQuery query;

	QVector<int> signalIds;

	QString appsignalId = "appSignalIdForGetSignalIdsWithAppSignalIds";

	// Create first signal (created by admin. Not checked In)
	//

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET appSIgnalId = '%1' WHERE signalInstanceId = %2").arg(appsignalId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Second signal (created by user, not checked In)
	//

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 0)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET appSIgnalId = '%1' WHERE signalInstanceId = %2").arg(appsignalId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Third signal (created by admin. Checked In - Checked Out - Deleted - Checked In)
	//

	ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET appSIgnalId = '%1' WHERE signalInstanceId = %2").arg(appsignalId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_signal(1, %1)").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// First time fucntion check. Started by admin. Two records expected (First and Second)
	//

	ok = query.exec(QString("SELECT * FROM get_signal_ids_with_appsignalid(1, '%1') ORDER BY get_signal_ids_with_appsignalid").arg(appsignalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int currentValue = 0;

	while (query.next())
	{
		QVERIFY2 (currentValue < 2, qPrintable("Error: wrong amount of returned signals"));

		QVERIFY2 (query.value(0).toInt() == signalIds[currentValue], qPrintable("Error: wrong signalId returned"));

		currentValue++;
	}

	QVERIFY2 (currentValue == 2, qPrintable("Error: wrong amount of returned signals"));

	// Second Time function start. Started by user. One record expected (Second)
	//

	ok = query.exec(QString("SELECT * FROM get_signal_ids_with_appsignalid(%1, '%2') ORDER BY get_signal_ids_with_appsignalid").arg(m_firstUserForTest).arg(appsignalId));

	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.value(0).toInt() == signalIds[1], qPrintable("Error: wrong signalId returned"));
	QVERIFY2 (query.next() == false, qPrintable("Error: one record expected"));

	// Check in First Signal by admin
	//

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalIds[0]));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	// Third time function start. Started by user. Two records expected (First, Second)

	ok = query.exec(QString("SELECT * FROM get_signal_ids_with_appsignalid(%1, '%2') ORDER BY get_signal_ids_with_appsignalid").arg(m_firstUserForTest).arg(appsignalId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	currentValue = 0;

	while (query.next())
	{
		QVERIFY2 (currentValue < 2, qPrintable("Error: wrong amount of returned signals"));

		QVERIFY2 (query.value(0).toInt() == signalIds[currentValue], qPrintable("Error: wrong signalId returned"));

		currentValue++;
	}

	QVERIFY2 (currentValue == 2, qPrintable("Error: wrong amount of returned signals"));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_signal_ids_with_appsignalid(%1, '%2') ORDER BY get_signal_ids_with_appsignalid").arg(m_firstUserForTest).arg(appsignalId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	currentValue = 0;

	while (query.next())
	{
		QVERIFY2 (currentValue < 2, qPrintable("Error: wrong amount of returned signals"));

		QVERIFY2 (query.value(0).toInt() == signalIds[currentValue], qPrintable("Error: wrong signalId returned"));

		currentValue++;
	}

	QVERIFY2 (currentValue == 2, qPrintable("Error: wrong amount of returned signals"));
}

void SignalTests::get_signal_Ids_with_customAppSignalIdTest()
{
	QSqlQuery query;

	QVector<int> signalIds;

	QString customAppsignalId = "CustomAppSignalIdForGetSignalIdsWithAppSignalIds";

	// Create first signal (created by admin. Not checked In)
	//

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET customAppSIgnalId = '%1' WHERE signalInstanceId = %2").arg(customAppsignalId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Second signal (created by user, not checked In)
	//

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 0)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET customAppSIgnalId = '%1' WHERE signalInstanceId = %2").arg(customAppsignalId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Third signal (created by admin. Checked In - Checked Out - Deleted - Checked In)
	//

	ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET customAppSIgnalId = '%1' WHERE signalInstanceId = %2").arg(customAppsignalId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_signal(1, %1)").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// First time fucntion check. Started by admin. Two records expected (First and Second)
	//

	ok = query.exec(QString("SELECT * FROM get_signal_ids_with_customappsignalid(1, '%1') ORDER BY get_signal_ids_with_customappsignalid").arg(customAppsignalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int currentValue = 0;

	while (query.next())
	{
		QVERIFY2 (currentValue < 2, qPrintable("Error: wrong amount of returned signals"));

		QVERIFY2 (query.value(0).toInt() == signalIds[currentValue], qPrintable("Error: wrong signalId returned"));

		currentValue++;
	}

	QVERIFY2 (currentValue == 2, qPrintable("Error: wrong amount of returned signals"));

	// Second Time function start. Started by user. One record expected (Second)
	//

	ok = query.exec(QString("SELECT * FROM get_signal_ids_with_customappsignalid(%1, '%2') ORDER BY get_signal_ids_with_customappsignalid").arg(m_firstUserForTest).arg(customAppsignalId));

	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.value(0).toInt() == signalIds[1], qPrintable("Error: wrong signalId returned"));
	QVERIFY2 (query.next() == false, qPrintable("Error: one record expected"));

	// Check in First Signal by admin
	//

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalIds[0]));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	// Third time function start. Started by user. Two records expected (First, Second)

	ok = query.exec(QString("SELECT * FROM get_signal_ids_with_customappsignalid(%1, '%2') ORDER BY get_signal_ids_with_customappsignalid").arg(m_firstUserForTest).arg(customAppsignalId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	currentValue = 0;

	while (query.next())
	{
		QVERIFY2 (currentValue < 2, qPrintable("Error: wrong amount of returned signals"));

		QVERIFY2 (query.value(0).toInt() == signalIds[currentValue], qPrintable("Error: wrong signalId returned"));

		currentValue++;
	}

	QVERIFY2 (currentValue == 2, qPrintable("Error: wrong amount of returned signals"));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_signal_ids_with_customappsignalid(%1, '%2') ORDER BY get_signal_ids_with_customappsignalid").arg(m_firstUserForTest).arg(customAppsignalId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	currentValue = 0;

	while (query.next())
	{
		QVERIFY2 (currentValue < 2, qPrintable("Error: wrong amount of returned signals"));

		QVERIFY2 (query.value(0).toInt() == signalIds[currentValue], qPrintable("Error: wrong signalId returned"));

		currentValue++;
	}

	QVERIFY2 (currentValue == 2, qPrintable("Error: wrong amount of returned signals"));
}

void SignalTests::get_signal_Ids_with_equipmentIdTest()
{
	QSqlQuery query;

	QVector<int> signalIds;

	QString equipmentId = "EquipmentIdForGetSignalIdsWithAppSignalIds";

	// Create first signal (created by admin. Not checked In)
	//

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET equipmentID = '%1' WHERE signalInstanceId = %2").arg(equipmentId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Second signal (created by user, not checked In)
	//

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 0)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET equipmentID = '%1' WHERE signalInstanceId = %2").arg(equipmentId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Third signal (created by admin. Checked In - Checked Out - Deleted - Checked In)
	//

	ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET equipmentID = '%1' WHERE signalInstanceId = %2").arg(equipmentId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_signal(1, %1)").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// First time fucntion check. Started by admin. Two records expected (First and Second)
	//

	ok = query.exec(QString("SELECT * FROM get_signal_ids_with_equipmentid(1, '%1') ORDER BY get_signal_ids_with_equipmentid").arg(equipmentId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int currentValue = 0;

	while (query.next())
	{
		QVERIFY2 (currentValue < 2, qPrintable("Error: wrong amount of returned signals"));

		QVERIFY2 (query.value(0).toInt() == signalIds[currentValue], qPrintable("Error: wrong signalId returned"));

		currentValue++;
	}

	QVERIFY2 (currentValue == 2, qPrintable("Error: wrong amount of returned signals"));

	// Second Time function start. Started by user. One record expected (Second)
	//

	ok = query.exec(QString("SELECT * FROM get_signal_ids_with_equipmentid(%1, '%2') ORDER BY get_signal_ids_with_equipmentid").arg(m_firstUserForTest).arg(equipmentId));

	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.value(0).toInt() == signalIds[1], qPrintable("Error: wrong signalId returned"));
	QVERIFY2 (query.next() == false, qPrintable("Error: one record expected"));

	// Check in First Signal by admin
	//

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalIds[0]));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	// Third time function start. Started by user. Two records expected (First, Second)

	ok = query.exec(QString("SELECT * FROM get_signal_ids_with_equipmentid(%1, '%2') ORDER BY get_signal_ids_with_equipmentid").arg(m_firstUserForTest).arg(equipmentId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	currentValue = 0;

	while (query.next())
	{
		QVERIFY2 (currentValue < 2, qPrintable("Error: wrong amount of returned signals"));

		QVERIFY2 (query.value(0).toInt() == signalIds[currentValue], qPrintable("Error: wrong signalId returned"));

		currentValue++;
	}

	QVERIFY2 (currentValue == 2, qPrintable("Error: wrong amount of returned signals"));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_signal_ids_with_equipmentid(%1, '%2') ORDER BY get_signal_ids_with_equipmentid").arg(m_firstUserForTest).arg(equipmentId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	currentValue = 0;

	while (query.next())
	{
		QVERIFY2 (currentValue < 2, qPrintable("Error: wrong amount of returned signals"));

		QVERIFY2 (query.value(0).toInt() == signalIds[currentValue], qPrintable("Error: wrong signalId returned"));

		currentValue++;
	}

	QVERIFY2 (currentValue == 2, qPrintable("Error: wrong amount of returned signals"));
}

void SignalTests::delete_signal_by_equipmentidTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	QVector<int> signalIds;

	QString equipmentId = "delete_signal_by_equipmentid";

	// TODO: ONLY ONE SIGNAL WILL BE DELETED!!!!
	//

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET equipmentID = '%1' WHERE signalInstanceId = %2").arg(equipmentId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET equipmentID = '%1' WHERE signalInstanceId = %2").arg(equipmentId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1, %2}', 'TEST')").arg(signalIds[0]).arg(signalIds[1]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1, %2}')").arg(signalIds[0]).arg(signalIds[1]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_signal_by_equipmentid(1, '%1') ORDER BY id").arg(equipmentId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	for (int currentSignalId : signalIds)
	{
		ok = tempQuery.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(currentSignalId));
		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));

		ok = tempQuery.exec(QString("SELECT * FROM Signal WHERE SignalId = %1").arg(currentSignalId));
		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("deleted").toBool() == true, qPrintable("Error: Signal was not deleted"));
	}
}

void SignalTests::is_signal_with_equipmentid_existsTest()
{
	QSqlQuery query;

	QVector<int> signalIds;

	QString equipmentId = "is_signal_with_equipmentid_existsTest";

	// TODO: ONLY ONE SIGNAL WILL BE DELETED!!!!
	//

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET equipmentID = '%1' WHERE signalInstanceId = %2").arg(equipmentId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET equipmentID = '%1' WHERE signalInstanceId = %2").arg(equipmentId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Test before checkIn
	//

	ok = query.exec(QString("SELECT * FROM is_signal_with_equipmentid_exists(1, '%1')").arg(equipmentId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error: function return false (true expected)"));

	ok = query.exec(QString("SELECT * FROM is_signal_with_equipmentid_exists(%1, '%2')").arg(m_firstUserForTest).arg(equipmentId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: function return true (false expected)"));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Test with one signal checkedIn
	//

	ok = query.exec(QString("SELECT * FROM is_signal_with_equipmentid_exists(1, '%1')").arg(equipmentId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error: function return false (true expected)"));

	ok = query.exec(QString("SELECT * FROM is_signal_with_equipmentid_exists(%1, '%2')").arg(m_firstUserForTest).arg(equipmentId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error: function return false (true expected)"));

	// Signal created by user, and was not checkedIn
	//

	signalIds.clear();

	equipmentId = "second_is_signal_with_equipmentid_existsTest";

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 0)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET equipmentID = '%1' WHERE signalInstanceId = %2").arg(equipmentId).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM is_signal_with_equipmentid_exists(1, '%1')").arg(equipmentId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error: function return false (true expected)"));

	ok = query.exec(QString("SELECT * FROM is_signal_with_equipmentid_exists(%1, '%2')").arg(m_firstUserForTest).arg(equipmentId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error: function return false (true expected)"));

	// Lets delete this signal
	//

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'TEST')").arg(m_firstUserForTest).arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(%1, '{%2}')").arg(m_firstUserForTest).arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_signal(%1, %2)").arg(m_firstUserForTest).arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'TEST')").arg(m_firstUserForTest).arg(signalIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM is_signal_with_equipmentid_exists(1, '%1')").arg(equipmentId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: function return true (false expected)"));

	ok = query.exec(QString("SELECT * FROM is_signal_with_equipmentid_exists(%1, '%2')").arg(m_firstUserForTest).arg(equipmentId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: function return true (false expected)"));
}

void SignalTests::get_latest_signals_by_appsignalIds()
{
	QSqlQuery query;

	QVector<int> signalIds;

	QString appsignalIdFirst = "getLatestSignalsByAppsignalIdsFirst";
	QString appsignalIdSecond = "getLatestSignalsByAppsignalIdsSecond";

	// Create first signal (created by admin. Checked In.)
	//

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET appSIgnalId = '%1' WHERE signalInstanceId = %2").arg(appsignalIdFirst).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkIn_signals(1, '{%1}', 'TEST')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Second signal (created by admin, not checked In)
	//

	ok = query.exec(QString("SELECT * FROM add_signal(%1, 0, 0)").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET appSIgnalId = '%1' WHERE signalInstanceId = %2").arg(appsignalIdSecond).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Third signal (created by admin. Checked In - Checked Out - Deleted - Checked In)
	//

	ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalId = query.value("Id").toInt();
	signalIds.push_back(signalId);

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM Signal WHERE SignalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	signalInstanceId = query.value(0).toInt();

	ok = query.exec(QString("UPDATE SignalInstance SET customAppSIgnalId = '%1' WHERE signalInstanceId = %2").arg(appsignalIdFirst).arg(signalInstanceId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_signal(1, %1)").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(1, '{%1}')").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
}

void SignalTests::get_signal_historyTest()
{
	QSqlQuery query, signalInstanceQuery, changesetQuery, usersQuery;

	int recordsAmount = 0;

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value("Id").toInt();

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'First checkIn')").arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(%1, '{%2}')").arg(m_firstUserForTest).arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'Second checkIn')").arg(m_firstUserForTest).arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(%1, '{%2}')").arg(m_secondUserForTest).arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'Third checkIn')").arg(m_secondUserForTest).arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_signal_history(1, %1)").arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = signalInstanceQuery.exec(QString("SELECT * FROM signalInstance WHERE SignalId = %1 ORDER BY changesetId DESC").arg(signalId));

	QVERIFY2(ok == true, qPrintable(signalInstanceQuery.lastError().databaseText()));

	while (query.next() == true)
	{
		QVERIFY2(signalInstanceQuery.next() == true, qPrintable(signalInstanceQuery.lastError().databaseText()));

		int changesetFromQuery = query.value("changesetId").toInt();
		int changesetFromCheck = signalInstanceQuery.value("changesetId").toInt();

		QVERIFY2(changesetFromQuery == changesetFromCheck, qPrintable("Error: get_signal_history returned wrong changesetId"));
		QVERIFY2(query.value("action").toInt() == signalInstanceQuery.value("action").toInt(), qPrintable("Error: wrong action has been returned"));

		ok = changesetQuery.exec(QString("SELECT * FROM changeset WHERE changesetId = %1").arg(changesetFromQuery));

		QVERIFY2(ok == true, qPrintable(changesetQuery.lastError().databaseText()));
		QVERIFY2(changesetQuery.next() == true, qPrintable(changesetQuery.lastError().databaseText()));

		QVERIFY2(query.value("comment").toString() == changesetQuery.value("comment").toString(), qPrintable("Error: wrong comment has been set"));
		QVERIFY2(query.value("userId").toInt() == changesetQuery.value("userId").toInt(), qPrintable("Error: wrong userId has been set"));
		QVERIFY2(query.value("checkInTime").toString() == changesetQuery.value("time").toString(), qPrintable("Error: wrong checkInTime has been set"));

		ok = usersQuery.exec(QString("SELECT username FROM users WHERE userId = %1").arg(query.value("userId").toInt()));

		QVERIFY2(ok == true, qPrintable(usersQuery.lastError().databaseText()));
		QVERIFY2(usersQuery.next() == true, qPrintable(usersQuery.lastError().databaseText()));

		QVERIFY2(usersQuery.value(0).toString() == query.value("username").toString(), qPrintable("Error: wrong username"));

		recordsAmount++;
	}

	QVERIFY2(recordsAmount == 3, qPrintable("Error: get_signal_history returned wrong amount of changesets"));
}

void SignalTests::get_specific_signalTest()
{
	QSqlQuery query, tempQuery;
	QString keyComment = "keyComment";

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int signalId = query.value("Id").toInt();

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'First checkIn')").arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(%1, '{%2}')").arg(m_firstUserForTest).arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', '%3')").arg(m_firstUserForTest).arg(signalId).arg(keyComment));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkout_signals(%1, '{%2}')").arg(m_secondUserForTest).arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'Third checkIn')").arg(m_secondUserForTest).arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT changesetId FROM changeset WHERE comment = '%1'").arg(keyComment));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int changesetId = query.value(0).toInt();

	ok = query.exec(QString("SELECT * FROM get_specific_signal(1, %1, %2)").arg(signalId).arg(changesetId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("changesetId").toInt() == changesetId, qPrintable("Error: wrongChangesetId returned"));

	ok = tempQuery.exec(QString("SELECT * FROM signalInstance WHERE changesetId = %1").arg(changesetId));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value("signalId").toInt() == tempQuery.value("signalId").toInt(), qPrintable(QString("Error: value signalId is not match (Actual: %1, Expected: %2)").arg(query.value("signalId").toInt()).arg(tempQuery.value("signalId").toInt())));
	QVERIFY2(query.value("appSignalID").toString() == tempQuery.value("appSignalID").toString(), qPrintable(QString("Error: value appSignalID is not match (Actual: %1, Expected: %2)").arg(query.value("appSignalID").toString()).arg(tempQuery.value("appSignalID").toString())));
	QVERIFY2(query.value("customAppSignalID").toString() == tempQuery.value("customAppSignalID").toString(), qPrintable(QString("Error: value customAppSignalID is not match (Actual: %1, Expected: %2)").arg(query.value("customAppSignalID").toString()).arg(tempQuery.value("customAppSignalID").toString())));
	QVERIFY2(query.value("caption").toString() == tempQuery.value("caption").toString(), qPrintable(QString("Error: value name is not match (Actual: %1, Expected: %2)").arg(query.value("caption").toString()).arg(tempQuery.value("caption").toString())));
	QVERIFY2(query.value("analogSignalFormat").toInt() == tempQuery.value("analogSignalFormat").toInt(), qPrintable(QString("Error: value analogSignalFormat is not match (Actual: %1, Expected: %2)").arg(query.value("analogSignalFormat").toInt()).arg(tempQuery.value("analogSignalFormat").toInt())));
	QVERIFY2(query.value("dataSize").toInt() == tempQuery.value("dataSize").toInt(), qPrintable(QString("Error: value dataSize is not match (Actual: %1, Expected: %2)").arg(query.value("dataSize").toInt()).arg(tempQuery.value("dataSize").toInt())));
	QVERIFY2(query.value("lowAdc").toInt() == tempQuery.value("lowAdc").toInt(), qPrintable(QString("Error: value lowAdc is not match (Actual: %1, Expected: %2)").arg(query.value("lowAdc").toInt()).arg(tempQuery.value("lowAdc").toInt())));
	QVERIFY2(query.value("highAdc").toInt() == tempQuery.value("highAdc").toInt(), qPrintable(QString("Error: value highAdc is not match (Actual: %1, Expected: %2)").arg(query.value("highAdc").toInt()).arg(tempQuery.value("highAdc").toInt())));
	QVERIFY2(query.value("lowengeneeringunits").toInt() == tempQuery.value("lowengeneeringunits").toInt(), qPrintable(QString("Error: value lowengeneeringunits is not match (Actual: %1, Expected: %2)").arg(query.value("lowengeneeringunits").toInt()).arg(tempQuery.value("lowengeneeringunits").toInt())));
	QVERIFY2(query.value("highengeneeringunits").toInt() == tempQuery.value("highengeneeringunits").toInt(), qPrintable(QString("Error: value highengeneeringunits is not match (Actual: %1, Expected: %2)").arg(query.value("highengeneeringunits").toInt()).arg(tempQuery.value("highengeneeringunits").toInt())));
	QVERIFY2(query.value("unitId").toInt() == tempQuery.value("unitId").toInt(), qPrintable(QString("Error: value unitId is not match (Actual: %1, Expected: %2)").arg(query.value("unitId").toInt()).arg(tempQuery.value("unitId").toInt())));
	QVERIFY2(query.value("adjustment").toInt() == tempQuery.value("adjustment").toInt(), qPrintable(QString("Error: value adjustment is not match (Actual: %1, Expected: %2)").arg(query.value("adjustment").toInt()).arg(tempQuery.value("adjustment").toInt())));
	QVERIFY2(query.value("lowvalidrange").toInt() == tempQuery.value("lowvalidrange").toInt(), qPrintable(QString("Error: value lowvalidrange is not match (Actual: %1, Expected: %2)").arg(query.value("lowvalidrange").toInt()).arg(tempQuery.value("lowvalidrange").toInt())));
	QVERIFY2(query.value("highvalidrange").toInt() == tempQuery.value("highvalidrange").toInt(), qPrintable(QString("Error: value highvalidrange is not match (Actual: %1, Expected: %2)").arg(query.value("highvalidrange").toInt()).arg(tempQuery.value("highvalidrange").toInt())));
	QVERIFY2(query.value("unbalanceLimit").toInt() == tempQuery.value("unbalanceLimit").toInt(), qPrintable(QString("Error: value unbalanceLimit is not match (Actual: %1, Expected: %2)").arg(query.value("unbalanceLimit").toInt()).arg(tempQuery.value("unbalanceLimit").toInt())));
	QVERIFY2(query.value("inputLowLimit").toInt() == tempQuery.value("inputLowLimit").toInt(), qPrintable(QString("Error: value inputLowLimit is not match (Actual: %1, Expected: %2)").arg(query.value("inputLowLimit").toInt()).arg(tempQuery.value("inputLowLimit").toInt())));
	QVERIFY2(query.value("inputHighLimit").toInt() == tempQuery.value("inputHighLimit").toInt(), qPrintable(QString("Error: value inputHighLimit is not match (Actual: %1, Expected: %2)").arg(query.value("inputHighLimit").toInt()).arg(tempQuery.value("inputHighLimit").toInt())));
	QVERIFY2(query.value("inputUnitId").toInt() == tempQuery.value("inputUnitId").toInt(), qPrintable(QString("Error: value inputUnitId is not match (Actual: %1, Expected: %2)").arg(query.value("inputUnitId").toInt()).arg(tempQuery.value("inputUnitId").toInt())));
	QVERIFY2(query.value("inputSensorId").toInt() == tempQuery.value("inputSensorId").toInt(), qPrintable(QString("Error: value inputSensorId is not match (Actual: %1, Expected: %2)").arg(query.value("inputSensorId").toInt()).arg(tempQuery.value("inputSensorId").toInt())));
	QVERIFY2(query.value("outputLowLimit").toInt() == tempQuery.value("outputLowLimit").toInt(), qPrintable(QString("Error: value outputLowLimit is not match (Actual: %1, Expected: %2)").arg(query.value("outputLowLimit").toInt()).arg(tempQuery.value("outputLowLimit").toInt())));
	QVERIFY2(query.value("outputHighLimit").toInt() == tempQuery.value("outputHighLimit").toInt(), qPrintable(QString("Error: value outputHighLimit is not match (Actual: %1, Expected: %2)").arg(query.value("outputHighLimit").toInt()).arg(tempQuery.value("outputHighLimit").toInt())));
	QVERIFY2(query.value("outputUnitId").toInt() == tempQuery.value("outputUnitId").toInt(), qPrintable(QString("Error: value outputUnitId is not match (Actual: %1, Expected: %2)").arg(query.value("outputUnitId").toInt()).arg(tempQuery.value("outputUnitId").toInt())));
	QVERIFY2(query.value("outputSensorId").toInt() == tempQuery.value("outputSensorId").toInt(), qPrintable(QString("Error: value outputSensorId is not match (Actual: %1, Expected: %2)").arg(query.value("outputSensorId").toInt()).arg(tempQuery.value("outputSensorId").toInt())));
	QVERIFY2(query.value("acquire").toString() == tempQuery.value("acquire").toString(), qPrintable(QString("Error: value acquire is not match (Actual: %1, Expected: %2)").arg(query.value("acquire").toString()).arg(tempQuery.value("acquire").toString())));
	QVERIFY2(query.value("calculated").toString() == tempQuery.value("calculated").toString(), qPrintable(QString("Error: value calculated is not match (Actual: %1, Expected: %2)").arg(query.value("calculated").toString()).arg(tempQuery.value("calculated").toString())));
	QVERIFY2(query.value("normalState").toInt() == tempQuery.value("normalState").toInt(), qPrintable(QString("Error: value normalState is not match (Actual: %1, Expected: %2)").arg(query.value("normalState").toInt()).arg(tempQuery.value("normalState").toInt())));
	QVERIFY2(query.value("decimalPlaces").toInt() == tempQuery.value("decimalPlaces").toInt(), qPrintable(QString("Error: value decimalPlaces is not match (Actual: %1, Expected: %2)").arg(query.value("decimalPlaces").toInt()).arg(tempQuery.value("decimalPlaces").toInt())));
	QVERIFY2(query.value("aperture").toInt() == tempQuery.value("aperture").toInt(), qPrintable(QString("Error: value aperture is not match (Actual: %1, Expected: %2)").arg(query.value("aperture").toInt()).arg(tempQuery.value("aperture").toInt())));
	QVERIFY2(query.value("inOutType").toInt() == tempQuery.value("inOutType").toInt(), qPrintable(QString("Error: value inOutType is not match (Actual: %1, Expected: %2)").arg(query.value("inOutType").toInt()).arg(tempQuery.value("inOutType").toInt())));
	QVERIFY2(query.value("equipmentID").toString() == tempQuery.value("equipmentID").toString(), qPrintable(QString("Error: value equipmentID is not match (Actual: %1, Expected: %2)").arg(query.value("equipmentID").toString()).arg(tempQuery.value("equipmentID").toString())));
	QVERIFY2(query.value("outputRangeMode").toInt() == tempQuery.value("outputRangeMode").toInt(), qPrintable(QString("Error: value outputRangeMode is not match (Actual: %1, Expected: %2)").arg(query.value("outputRangeMode").toInt()).arg(tempQuery.value("outputRangeMode").toInt())));
	QVERIFY2(query.value("filteringTime").toInt() == tempQuery.value("filteringTime").toInt(), qPrintable(QString("Error: value filteringTime is not match (Actual: %1, Expected: %2)").arg(query.value("filteringTime").toInt()).arg(tempQuery.value("filteringTime").toInt())));
	QVERIFY2(query.value("spreadtolerance").toInt() == tempQuery.value("spreadtolerance").toInt(), qPrintable(QString("Error: value spreadtolerance is not match (Actual: %1, Expected: %2)").arg(query.value("spreadtolerance").toInt()).arg(tempQuery.value("spreadtolerance").toInt())));
	QVERIFY2(query.value("byteOrder").toInt() == tempQuery.value("byteOrder").toInt(), qPrintable(QString("Error: value byteOrder is not match (Actual: %1, Expected: %2)").arg(query.value("byteOrder").toInt()).arg(tempQuery.value("byteOrder").toInt())));
	QVERIFY2(query.value("enableTuning").toString() == tempQuery.value("enableTuning").toString(), qPrintable(QString("Error: value enableTuning is not match (Actual: %1, Expected: %2)").arg(query.value("enableTuning").toString()).arg(tempQuery.value("enableTuning").toString())));
	QVERIFY2(query.value("tuningDefaultValue").toDouble() == tempQuery.value("tuningDefaultValue").toDouble(), qPrintable(QString("Error: value tuningDefaultValue is not match (Actual: %1, Expected: %2)").arg(query.value("tuningDefaultValue").toDouble()).arg(tempQuery.value("tuningDefaultValue").toDouble())));

	ok = tempQuery.exec(QString("SELECT * FROM signal WHERE signalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value("signalGroupId").toInt() == tempQuery.value("signalGroupId").toInt(), qPrintable("Error: wrong signalGroupId returned"));
	QVERIFY2(query.value("channel").toInt() == tempQuery.value("channel").toInt(), qPrintable("Error: wrong channel returned"));
	QVERIFY2(query.value("type").toInt() == tempQuery.value("type").toInt(), qPrintable("Error: wrong type returned"));
	QVERIFY2(query.value("created").toString() == tempQuery.value("created").toString(), qPrintable("Error: wrong creation date returned"));
	QVERIFY2(query.value("deleted").toBool() == tempQuery.value("deleted").toBool(), qPrintable("Error: wrong deleted value returned"));

	ok = tempQuery.exec(QString("SELECT * FROM changeset WHERE changesetId = %1").arg(changesetId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value("userId").toInt() == tempQuery.value("userId").toInt(), qPrintable(QString("Error: wrong userId value returned (Actual: %1, Expected: %2").arg(query.value("userId").toInt()).arg(tempQuery.value("userId").toInt())));

	// Check empty user
	//

	ok = query.exec(QString("SELECT * FROM get_specific_signal(0, %1, %2)").arg(signalId).arg(changesetId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("Expected error: empty userId"));

	// Check empty signalId
	//

	ok = query.exec(QString("SELECT * FROM get_specific_signal(1, 0, %1)").arg(changesetId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("Expected error: empty signalId"));

	// Check wrong changeset
	//

	ok = query.exec(QString("SELECT * FROM get_specific_signal(1, %1, 0)").arg(signalId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == false, qPrintable("Expected error: empty changeset"));
}


bool SignalTests::readSignalFromQuery(const QSqlQuery& q, Signal& s, quint64 excludeMask)
{
	s.setAppSignalID(q.value(SF_APP_SIGNAL_ID).toString());
	s.setCustomAppSignalID(q.value(SF_CUSTOM_APP_SIGNAL_ID).toString());
	s.setCaption(q.value(SF_CAPTION).toString());
	s.setEquipmentID(q.value(SF_EQUIPMENT_ID).toString());
	s.setBusTypeID(q.value(SF_BUS_TYPE_ID).toString());

	if ((excludeMask & EXM_CHANNEL) == 0)
	{
		s.setChannel(static_cast<E::Channel>(q.value(SF_CHANNEL).toInt()));
	}

	if ((excludeMask & EXM_SIGNAL_TYPE) == 0)
	{
		s.setSignalType(static_cast<E::SignalType>(q.value(SF_SIGNAL_TYPE).toInt()));
	}

	s.setInOutType(static_cast<E::SignalInOutType>(q.value(SF_IN_OUT_TYPE).toInt()));

	s.setDataSize(q.value(SF_DATA_SIZE).toInt());
	s.setByteOrder(static_cast<E::ByteOrder>(q.value(SF_BYTE_ORDER).toInt()));

	s.setAnalogSignalFormat(static_cast<E::AnalogAppSignalFormat>(q.value(SF_ANALOG_SIGNAL_FORMAT).toInt()));
	s.setUnit(q.value(SF_UNIT).toString());

	s.setLowADC(q.value(SF_LOW_ADC).toInt());
	s.setHighADC(q.value(SF_HIGH_ADC).toInt());
	s.setLowEngeneeringUnits(q.value(SF_LOW_ENGENEERING_UNITS).toDouble());
	s.setHighEngeneeringUnits(q.value(SF_HIGH_ENGENEERING_UNITS).toDouble());
	s.setLowValidRange(q.value(SF_LOW_VALID_RANGE).toDouble());
	s.setHighValidRange(q.value(SF_HIGH_VALID_RANGE).toDouble());
	s.setFilteringTime(q.value(SF_FILTERING_TIME).toDouble());
	s.setSpreadTolerance(q.value(SF_SPREAD_TOLERANCE).toDouble());

	s.setElectricLowLimit(q.value(SF_ELECTRIC_LOW_LIMIT).toDouble());
	s.setElectricHighLimit(q.value(SF_ELECTRIC_HIGH_LIMIT).toDouble());
	s.setElectricUnit(static_cast<E::ElectricUnit>(q.value(SF_ELECTRIC_UNIT).toInt()));
	s.setSensorType(static_cast<E::SensorType>(q.value(SF_SENSOR_TYPE).toInt()));
	s.setOutputMode(static_cast<E::OutputMode>(q.value(SF_OUTPUT_MODE).toInt()));

	s.setEnableTuning(q.value(SF_ENABLE_TUNING).toBool());
	s.setTuningDefaultValue(q.value(SF_TUNING_DEFAULT_VALUE).toFloat());
	s.setTuningLowBound(q.value(SF_TUNING_LOW_BOUND).toFloat());
	s.setTuningHighBound(q.value(SF_TUNING_HIGH_BOUND).toFloat());

	s.setAcquire(q.value(SF_ACQUIRE).toBool());
	s.setDecimalPlaces(q.value(SF_DECIMAL_PLACES).toInt());
	s.setCoarseAperture(q.value(SF_COARSE_APERTURE).toDouble());
	s.setFineAperture(q.value(SF_FINE_APERTURE).toDouble());
	s.setAdaptiveAperture(q.value(SF_ADAPTIVE_APERTURE).toBool());

	s.setID(q.value(SF_SIGNAL_ID).toInt());

	if ((excludeMask & EXM_SIGNAL_GROUP_ID) == 0)
	{
		s.setSignalGroupID(q.value(SF_SIGNAL_GROUP_ID).toInt());
	}

	s.setSignalInstanceID(q.value(SF_SIGNAL_INSTANCE_ID).toInt());
	s.setChangesetID(q.value(SF_CHANGESET_ID).toInt());

	if ((excludeMask & EXM_CHECKED_OUT) == 0)
	{
		s.setCheckedOut(q.value(SF_CHECKED_OUT).toBool());
	}

	if ((excludeMask & EXM_USER_ID) == 0)
	{
		s.setUserID(q.value(SF_USER_ID).toInt());
	}

	if ((excludeMask & EXM_CREATED) == 0)
	{
		s.setCreated(q.value(SF_CREATED).toDateTime());
	}

	if ((excludeMask & EXM_DELETED) == 0)
	{
		s.setDeleted(q.value(SF_DELETED).toBool());
	}

	if ((excludeMask & EXM_INSTANCE_CREATED) == 0)
	{
		s.setInstanceCreated(q.value(SF_INSTANCE_CREATED).toDateTime());
	}

	if ((excludeMask & EXM_INSTANCE_ACTION) == 0)
	{
		s.setInstanceAction(static_cast<VcsItemAction::VcsItemActionType>(q.value(SF_INSTANCE_ACTION).toInt()));
	}

	return true;
}

void SignalTests::verifyQueryAndSignal(const QSqlQuery& q, Signal& s, quint64 excludeMask)
{
	QVERIFY2(q.value(SF_APP_SIGNAL_ID).toString() == s.appSignalID(), "Error: appSignalID is wrong");
	QVERIFY2(q.value(SF_CUSTOM_APP_SIGNAL_ID).toString() == s.customAppSignalID(), "Error: customAppSignalID is wrong");
	QVERIFY2(q.value(SF_CAPTION).toString() == s.caption(), "Error: caption is wrong");
	QVERIFY2(q.value(SF_EQUIPMENT_ID).toString() == s.equipmentID(), "Error: equipmentID is wrong");
	QVERIFY2(q.value(SF_BUS_TYPE_ID).toString() == s.busTypeID(), "Error: busTypeID is wrong");

	if ((excludeMask & EXM_CHANNEL) == 0)
	{
		QVERIFY2(q.value(SF_CHANNEL).toInt() == s.channelInt(), "Error: channel is wrong");
	}

	if ((excludeMask & EXM_SIGNAL_TYPE) == 0)
	{
		QVERIFY2(q.value(SF_SIGNAL_TYPE).toInt() == s.signalTypeInt(), "Error: signalType is wrong");
	}

	QVERIFY2(q.value(SF_IN_OUT_TYPE).toInt() == s.inOutTypeInt(), "Error: inOutType is wrong");

	QVERIFY2(q.value(SF_DATA_SIZE).toInt() == s.dataSize(), "Error: dataSize is wrong");
	QVERIFY2(q.value(SF_BYTE_ORDER).toInt() == s.byteOrderInt(), "Error: byteOrder is wrong");

	QVERIFY2(q.value(SF_ANALOG_SIGNAL_FORMAT).toInt() == s.analogSignalFormatInt(), "Error: analogSignalFormat is wrong");
	QVERIFY2(q.value(SF_UNIT).toString() == s.unit(), "Error: unit is wrong");

	QVERIFY2(q.value(SF_LOW_ADC).toInt() == s.lowADC(), "Error: lowADC is wrong");
	QVERIFY2(q.value(SF_HIGH_ADC).toInt() == s.highADC(), "Error: highADC is wrong");
	QVERIFY2(q.value(SF_LOW_ENGENEERING_UNITS).toDouble() == s.lowEngeneeringUnits(), "Error: lowEngeneeringUnits is wrong");
	QVERIFY2(q.value(SF_HIGH_ENGENEERING_UNITS).toDouble() == s.highEngeneeringUnits(), "Error: highEngeneeringUnits is wrong");
	QVERIFY2(q.value(SF_LOW_VALID_RANGE).toDouble() == s.lowValidRange(), "Error: lowValidRange is wrong");
	QVERIFY2(q.value(SF_HIGH_VALID_RANGE).toDouble() == s.highValidRange(), "Error: highValidRange is wrong");
	QVERIFY2(q.value(SF_FILTERING_TIME).toDouble() == s.filteringTime(), "Error: filteringTime is wrong");
	QVERIFY2(q.value(SF_SPREAD_TOLERANCE).toDouble() == s.spreadTolerance(), "Error: spreadTolerance is wrong");

	QVERIFY2(q.value(SF_ELECTRIC_LOW_LIMIT).toDouble() == s.electricLowLimit(), "Error: electricLowLimit is wrong");
	QVERIFY2(q.value(SF_ELECTRIC_HIGH_LIMIT).toDouble() == s.electricHighLimit(), "Error: electricHighLimit is wrong");
	QVERIFY2(q.value(SF_ELECTRIC_UNIT).toInt() == s.electricUnitInt(), "Error: electricUnit is wrong");
	QVERIFY2(q.value(SF_SENSOR_TYPE).toInt() == s.sensorTypeInt(), "Error: sensorType is wrong");
	QVERIFY2(q.value(SF_OUTPUT_MODE).toInt() == s.outputModeInt(), "Error: outputMode is wrong");

	QVERIFY2(q.value(SF_ENABLE_TUNING).toBool() == s.enableTuning(), "Error: enableTuning is wrong");
	QVERIFY2(q.value(SF_TUNING_DEFAULT_VALUE).toFloat() == s.tuningDefaultValue(), "Error: tuningDefaultValue is wrong");
	QVERIFY2(q.value(SF_TUNING_LOW_BOUND).toFloat() == s.tuningLowBound(), "Error: tuningLowBound is wrong");
	QVERIFY2(q.value(SF_TUNING_HIGH_BOUND).toFloat() == s.tuningHighBound(), "Error: tuningHighBound is wrong");

	QVERIFY2(q.value(SF_ACQUIRE).toBool() == s.acquire(), "Error: acquire is wrong");
	QVERIFY2(q.value(SF_DECIMAL_PLACES).toInt() == s.decimalPlaces(), "Error: decimalPlaces is wrong");
	QVERIFY2(q.value(SF_COARSE_APERTURE).toDouble() == s.coarseAperture(), "Error: coarseAperture is wrong");
	QVERIFY2(q.value(SF_FINE_APERTURE).toDouble() == s.fineAperture(), "Error: fineAperture is wrong");
	QVERIFY2(q.value(SF_ADAPTIVE_APERTURE).toBool() == s.adaptiveAperture(), "Error: adaptiveAperture is wrong");

	if ((excludeMask & EXM_SIGNAL_ID) == 0)
	{
		QVERIFY2(q.value(SF_SIGNAL_ID).toInt() == s.ID(), "Error: signalID is wrong");
	}

	if ((excludeMask & EXM_SIGNAL_GROUP_ID) == 0)
	{
		QVERIFY2(q.value(SF_SIGNAL_GROUP_ID).toInt() == s.signalGroupID(), "Error: signalGroupID is wrong");
	}

	QVERIFY2(q.value(SF_SIGNAL_INSTANCE_ID).toInt() == s.signalInstanceID(), "Error: signalInstanceID is wrong");
	QVERIFY2(q.value(SF_CHANGESET_ID).toInt() == s.changesetID(), "Error: changesetID is wrong");

	if ((excludeMask & EXM_CHECKED_OUT) == 0)
	{
		QVERIFY2(q.value(SF_CHECKED_OUT).toBool() == s.checkedOut(), "Error: checkedOut is wrong");
	}

	if ((excludeMask & EXM_USER_ID) == 0)
	{
		QVERIFY2(q.value(SF_USER_ID).toInt() == s.userID(), "Error: userID is wrong");
	}

	if ((excludeMask & EXM_CREATED) == 0)
	{
		QVERIFY2(q.value(SF_CREATED).toDateTime() == s.created(), "Error: created is wrong");
	}

	if ((excludeMask & EXM_DELETED) == 0)
	{
		QVERIFY2(q.value(SF_DELETED).toBool() == s.deleted(), "Error: deleted is wrong");
	}

	if ((excludeMask & EXM_INSTANCE_CREATED) == 0)
	{
		QVERIFY2(q.value(SF_INSTANCE_CREATED).toDateTime() == s.instanceCreated(), "Error: instanceCreated is wrong");
	}

	if ((excludeMask & EXM_INSTANCE_ACTION) == 0)
	{
		QVERIFY2(q.value(SF_INSTANCE_ACTION).toInt() == s.instanceAction().toInt(), "Error: instanceAction is wrong");
	}
}

