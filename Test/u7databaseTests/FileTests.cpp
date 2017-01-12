#include <QtSql>
#include <QString>
#include <QTest>
#include "FileTests.h"
#include "../../lib/DbController.h"

void FileTests::getObjectState(QSqlQuery& q, ObjectState& os)
{
	os.id = q.value(0).toInt();
	os.deleted = q.value(1).toBool();
	os.checkedOut = q.value(2).toBool();
	os.action = q.value(3).toInt();
	os.userId = q.value(4).toInt();
	os.errCode = q.value(5).toInt();
}

FileTests::FileTests()
{
}

void FileTests::initTestCase()
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

	// Files_exist & file_exist
	//

	ok = query.exec("UPDATE file SET Deleted = true WHERE fileid = 3");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Testers
	//

	ok = query.exec(QString("SElECT user_api.create_user('%1', 'FIRSTTEST', 'FIRSTTEST', 'FIRSTTEST', '12341234', false, false);").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	FileTests::m_firstUserForTest = query.value("create_user").toInt();

	ok = query.exec(QString("SElECT user_api.create_user('%1', 'SECONDTEST', 'SECONDTEST', 'SECONDTEST', '12341234', false, false);").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	FileTests::m_secondUserForTest = query.value("create_user").toInt();

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
}

void FileTests::cleanupTestCase()
{
}

void FileTests::fileExistsTest()
{
	QSqlQuery query;

	bool ok = query.exec("SELECT file_exists(1);");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("file_exists").toBool() == true, qPrintable("Error: true expected"));

	ok = query.exec(QString("SELECT file_exists(%1);").arg(maxValueId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("file_exists").toBool() == false, qPrintable("Error: false expected"));

	ok = query.exec("SELECT file_exists(3);");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("file_exists").toBool() == false, qPrintable("Error: false expected"));
}

void FileTests::filesExistTest_data()
{
	QTest::addColumn<QString>("fileID");
	QTest::addColumn<QString>("result");

	QTest::newRow("existedFile") << "1" << "true";
	QTest::newRow("arrayExistingFiles") << "4,5,1" << "true,true,true";
	QTest::newRow("invalidFileTest") << "8590" << "false";
	QTest::newRow("arrayRandomFiles") << "67,99,9999" << "true,true,false";
	QTest::newRow("deletedFile") << "3" << "false";
}

void FileTests::filesExistTest()
{
	QFETCH(QString, fileID);
	QFETCH(QString, result);

	QCOMPARE(FileTests::filesExist(fileID), result);
}

void FileTests::is_any_checked_outTest()
{
	QSqlQuery query;

	bool ok = query.exec("Select is_any_checked_out();");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("is_any_checked_out").toBool() == false, qPrintable("Error: no checkedOut files expected"));


	ok = query.exec("SELECT * FROM add_file (1, 'isAnyCheckedOutTest', 2, 'TEST', '{}')");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec("Select is_any_checked_out();");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("is_any_checked_out").toBool() == true, qPrintable("Error: some checkedOuted files expected"));
}

void FileTests::filesAddTest_data()
{
	QTest::addColumn<int>("user_id");
	QTest::addColumn<QString>("file_name");
	QTest::addColumn<int>("parent_id");
	QTest::addColumn<QString>("file_data");
	QTest::addColumn<QString>("details");
	QTest::addColumn<bool>("result");

	QTest::newRow("checkingRandomFileRandomData") << 1 << "Test" << 2 << "Test" << "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}" << true;
	QTest::newRow("existingFile") << 1 << "Test" << 2 << "Test" << "{}" << false;
	QTest::newRow("checkInvalidUserId") << 999 << "TestInvalidUserId" << 2 << "OneTwoThree" << "{}" << false;
	QTest::newRow("checkInvalidParentId") << 1 << "TestInvalidParentId" << 999 << "OneTwoThreeFour" << "{}" << false;
	QTest::newRow("createdByUser") << m_firstUserForTest << "TestCreatedByUser" << 2 << "OneTwoThreeFourFive" << "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000001}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}" << true;
}

void FileTests::filesAddTest()
{
	QFETCH(int, user_id);
	QFETCH(QString, file_name);
	QFETCH(int, parent_id);
	QFETCH(QString, file_data);
	QFETCH(QString, details);
	QFETCH(bool, result);

	QCOMPARE(FileTests::add_file(user_id, file_name, parent_id, file_data, details), result);
}

void FileTests::is_file_checkedout()
{
	// Create new checkedOut fileId to Test
	//

	QSqlQuery query;

	bool ok = query.exec("SELECT * FROM add_file (1, 'isFileCheckedOutTest', 2, 'TESTING', '{}')");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fileId = query.value(0).toInt();

	// Try checkedOut file
	//

	ok = query.exec(QString("SELECT is_file_checkedout(%1);").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("is_file_checkedout").toBool() == true, qPrintable("Error: checkedOut fileId. True expected"));

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("checkedOutInstanceId").toUuid() != 0, qPrintable("Error: no record in column \"checkedOutInstanceId\" from \"file\""));
	QVERIFY2(query.value("checkedInInstanceId").toUuid() == 0, qPrintable("Error: there must not be record in column \"checkedInInstanceId\" from \"file\""));

	// Try checkedIn file
	//

	ok = query.exec(QString("SELECT is_file_checkedout(%1);").arg(1));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("is_file_checkedout").toBool() == false, qPrintable("CheckedIn fileid. False expected."));
}

void FileTests::file_has_children()
{
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT * FROM add_file (%1, 'fileHasChildrenTestParent', 1, 'TESTING', '{}')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec("SELECT count(*) FROM file WHERE parentid = 1");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int countOfFileIds = query.value(0).toInt();

	// Call function by file owner
	//

	ok = query.exec(QString("SELECT file_has_children (%1,%2);")
					.arg(m_firstUserForTest)
					.arg(1));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("file_has_children").toInt() == countOfFileIds, qPrintable(QString("Error! Wrong result!\nExpected: %1\nActual: %2\nFirst value: %3\nSecond value: %4").arg(countOfFileIds).arg(query.value("file_has_children").toInt()).arg(m_firstUserForTest).arg(1)));

	// Call function by other user
	//

	ok = query.exec(QString("SELECT file_has_children (%1,%2);")
					.arg(m_secondUserForTest)
					.arg(1));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("file_has_children").toInt() == countOfFileIds-1, qPrintable(QString("Error! Wrong result!\nExpected: %1\nActual: %2\nFirst value: %3\nSecond value: %4").arg(countOfFileIds-1).arg(query.value("file_has_children").toInt()).arg(m_secondUserForTest).arg(1)));

	// Call function by Admin
	//

	ok = query.exec(QString("SELECT file_has_children (%1,%2);")
					.arg(1)
					.arg(1));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("file_has_children").toInt() == countOfFileIds, qPrintable(QString("Error! Wrong result!\nExpected: %1\nActual: %2\nFirst value: %3\nSecond value: %4").arg(countOfFileIds).arg(query.value("file_has_children").toInt()).arg(1).arg(1)));

	// Call function with invalid user
	//

	ok = query.exec(QString("SELECT file_has_children (%1,%2);")
					.arg(maxValueId)
					.arg(1));
	QVERIFY2(ok == false, qPrintable("Wrong userid error expected"));

	// Call function with invalid file
	//

	ok = query.exec(QString("SELECT file_has_children (%1,%2);")
					.arg(1)
					.arg(maxValueId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("file_has_children").toInt() == 0, qPrintable(QString("Error! Wrong result!\nExpected: %1\nActual: %2\nFirst value: %3\nSecond value: %4").arg(0).arg(query.value("file_has_children").toInt()).arg(1).arg(maxValueId)));
}

void FileTests::delete_fileTest_data()
{
	QTest::addColumn<int>("userId");
	QTest::addColumn<int>("fileId");
	QTest::addColumn<bool>("result");

	QSqlQuery query;

	bool ok = query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestFirstFile', 1, 'TESTING', '{}')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int firstFileOwnerFirstUser = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestSecondFile', 1, 'TESTING', '{}')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int secondFileOwnerFirstUser = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestThirdFile', 1, 'TESTING', '{}')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int thirdFileOwnerFirstUser = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestFourthFile', 1, 'TESTING', '{}')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int checkedInFileOwnerFirstUser = query.value("id").toInt();

	ok = query.exec(QString("SELECT check_in(%1, '{%2}', 'test')")
					.arg(1)
					.arg(checkedInFileOwnerFirstUser));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	if (ok == false)
		qDebug() << "ERROR EXECUTTING check_in QUERY";

	QTest::newRow("deleteFileOwnerFirstUserBySecondUser") << FileTests::m_secondUserForTest << firstFileOwnerFirstUser << false;
	QTest::newRow("deleteFileOwnerFirstUserByFirstUser") << FileTests::m_firstUserForTest << firstFileOwnerFirstUser << true;
	QTest::newRow("deleteFileOwnerFirstUserByAdmin") << 1 << secondFileOwnerFirstUser << true;
	QTest::newRow("deleteFileInvalidUser") << 9999 << thirdFileOwnerFirstUser << false;
	QTest::newRow("deleteFileInvalidFile") << 1 << 9999 << false;
	QTest::newRow("checkedInFileOwnerFirstUserByAdmin") << 1 << checkedInFileOwnerFirstUser << true;
}

void FileTests::delete_fileTest()
{
	QFETCH (int, userId);
	QFETCH (int, fileId);
	QFETCH (bool, result);

	QCOMPARE(FileTests::delete_file(userId, fileId), result);
}

void FileTests::check_inTest()
{
	QSqlQuery query;

	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	bool ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestFirstFile', 1, 'TESTING', '%2')").arg(m_firstUserForTest).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString firstFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestSecondFile', 1, 'TESTING', '%2')").arg(m_firstUserForTest).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString secondFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestThirdFile', 1, 'TESTING', '%2')").arg(m_firstUserForTest).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString thirdFile = query.value("id").toString();
	ok = query.exec(QString("UPDATE fileInstance SET action = 3 WHERE fileId = %1").arg(thirdFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestFourthFile', 1, 'TESTING', '%2')").arg(m_secondUserForTest).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString fourthFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestFifthFile', 1, 'TESTING', '%2')").arg(m_firstUserForTest).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString fifthFile = query.value("id").toString();
	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'TEST');").arg(m_firstUserForTest).arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	QString comment = "TEST";


	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', '%3');").arg(m_firstUserForTest).arg(QString(firstFile + ", " + secondFile + ", " + thirdFile)).arg(comment));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next())
	{
		int id = query.value("id").toInt();

		QVERIFY2(query.value("errCode").toInt() == 0, qPrintable(QString("Error code %1 at fileId %2").arg(query.value("errCode").toInt()).arg(id)));
		QSqlQuery tempQuery;

		ok = tempQuery.exec(QString("SELECT COUNT(*) FROM checkout WHERE fileId = %1").arg(id));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.value(0) == 0, qPrintable(QString("FILEID %1 hasn't been deleted from checkout!").arg(id)));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(id));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.value("checkedOutInstanceId") == "", qPrintable(QString("File %1 has not been checked in").arg(id)));

		QString Uuid = tempQuery.value("checkedInInstanceId").toString();

		ok = tempQuery.exec(QString("SELECT * FROM fileinstance WHERE fileId = %1").arg(id));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.value("fileInstanceId").toString() == Uuid, qPrintable(QString("Error: wrong Uuid at file %1!").arg(id)));
		QVERIFY2(tempQuery.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(detail)));

		int changeSetId = tempQuery.value("changeSetId").toInt();
		int action = query.value("action").toInt();

		ok = tempQuery.exec(QString("SELECT COUNT(*) FROM changeSet WHERE changeSetId = %1 AND userId = %2 AND comment = '%3'").arg(changeSetId).arg(m_firstUserForTest).arg(comment));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.value(0) != 0, qPrintable(QString("Fileid %1 has not been recorded to changeset table").arg(id)));

		if (action == 3)
		{
			ok = tempQuery.exec(QString("SELECT deleted FROM file WHERE fileId = %1").arg(id));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.value(0).toBool() == true, qPrintable(QString("Error: flag deleted is not set on file %1").arg(id)));
		}
	}

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', '%3');").arg(m_firstUserForTest).arg(fourthFile).arg("TEST"));
	QVERIFY2(ok == false, qPrintable("User has no rights to checkin file checkouted by another user error expected"));

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', '%3');").arg(m_firstUserForTest).arg(fourthFile).arg("TEST"));
	QVERIFY2(ok == false, qPrintable("Can not checkin checkinned file error expected"));
}

void FileTests::check_outTest()
{
	QSqlQuery query;

	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	bool ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkOutFileTestFirstFile', 1, 'TESTING', '%2')").arg(m_firstUserForTest).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString firstFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'TEST');").arg(m_firstUserForTest).arg(firstFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkOutFileTestSecondFile', 1, 'TESTING', '%2')").arg(m_firstUserForTest).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString secondFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'TEST');").arg(m_firstUserForTest).arg(secondFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkOutFileTestThirdFile', 1, 'TESTING', '%2')").arg(m_firstUserForTest).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString thirdFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM check_out(%1, '{%2}');").arg(m_firstUserForTest).arg(QString(firstFile + ", " + secondFile)));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next())
	{
		int id = query.value("id").toInt();
		QVERIFY2(query.value("errCode").toInt() == 0, qPrintable(QString("Error code %1 AT FILEID %2").arg(query.value("errCode").toInt()).arg(id)));

		QSqlQuery tempQuery;

		ok = tempQuery.exec(QString("SELECT COUNT(*) FROM checkOut WHERE fileId = %1 AND userId = %2").arg(id).arg(m_firstUserForTest));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.value(0).toInt() != 0, qPrintable(QString("File %1 has not been checked out").arg(id)));

		ok = tempQuery.exec(QString("SELECT checkedOutInstanceId FROM file WHERE fileId = %1").arg(id));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QString Uuid = tempQuery.value(0).toString();

		ok = tempQuery.exec(QString("SElECT * FROM fileInstance WHERE fileId = %1 AND action  = 2").arg(id));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.value("fileInstanceId").toString() == Uuid, qPrintable(QString("No record in fileinstance at file %1").arg(id)));
		QVERIFY2(tempQuery.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(detail)));
	}
}

void FileTests::set_workcopyTest()
{
	QSqlQuery query;
	QString data = "TEST";
	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'setWorkcopyTestFirstFile', 1, 'TESTING', '{}')").arg(m_firstUserForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QString firstFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'setWorkcopyTestSecondFile', 1, 'TESTING', '{}')").arg(m_secondUserForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QString secondFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM set_workcopy(%1, %2, '%3', '%4');").arg(m_firstUserForTest).arg(firstFile).arg(data).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM file WHERE FileID = %1").arg(firstFile));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QString Uuid = query.value(0).toString();

	ok = query.exec(QString("SELECT * FROM FileInstance WHERE fileInstanceId = '%1'").arg(Uuid));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("data").toString() == data, qPrintable(QString("Column data has not been changed in table fileInstance with fileID %1").arg(firstFile)));
	QVERIFY2(query.value("size").toInt() == data.length(), qPrintable(QString("Column size has not been changed in table fileInstance with fileID %1").arg(firstFile)));
	QVERIFY2(query.value("FileID").toString() == firstFile, qPrintable(QString("Column fileId has not been changed in table fileInstance with checkOutInstanceId %1").arg(Uuid)));
	QVERIFY2(query.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(query.value("details").toString()).arg(detail)));

	//Call user error

	ok = query.exec(QString("SELECT * FROM set_workcopy(%1, %2, '%3', '{}');").arg(m_firstUserForTest).arg(secondFile).arg(data));

	QVERIFY2(ok == false, qPrintable("User is not allowed set workcopy error expected"));

	//Call checkedIn file error

	ok = query.exec(QString("SELECT * FROM set_workcopy(%1, %2, '%3', '{}');").arg(m_firstUserForTest).arg(1).arg(data));

	QVERIFY2(ok == false, qPrintable("Checked In file error expected"));
}

void FileTests::get_workcopyTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;
	QString data = "TEST";
	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'getWorkcopyTestFirstFile', 1, '%2', '%3')").arg(m_firstUserForTest).arg(data).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QString firstFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM get_workcopy(%1, %2);").arg(m_firstUserForTest).arg(firstFile));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT * FROM checkOut WHERE fileId = %1").arg(query.value("fileId").toInt()));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value("userId").toInt() == query.value("userId").toInt(), qPrintable(QString("Error: in column \"userId\" - data mistmatch")));
	QVERIFY2(query.value("checkedOut").toBool() == true, qPrintable(QString("Error: in column \"checkOut\" - data mistmatch")));

	ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(query.value("fileId").toInt()));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value("name").toString() == query.value("name").toString(), qPrintable("Error: in column \"name\" - data mistmatch"));
	QVERIFY2(tempQuery.value("parentId").toInt() == query.value("parentId").toInt(), qPrintable("Error: in column \"parentId\" - data mistmatch"));

	ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileId = %1").arg(query.value("fileId").toInt()));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("Error: in column \"size\" - data mistmatch")));
	QVERIFY2(data == query.value("data").toString(), qPrintable(QString("Error: in column \"data\" - data mistmatch")));
	QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("Error: in column \"action\" - data mistmatch")));
	QVERIFY2(query.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(query.value("details").toString()).arg(detail)));
	QVERIFY2(query.value("changeSetId").toInt() == tempQuery.value("changeSetId").toInt(), qPrintable("Error: in column \"changeSetId\" - data mistmatch"));

	ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(query.value("fileId").toInt()));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(tempQuery.value("deleted").toBool() == query.value("deleted").toBool(), qPrintable("Error in column \"deleted\""));
}

void FileTests::get_file_historyTest()
{
	QSqlQuery query;
	QSqlQuery fileInstanceQuery;
	QSqlQuery changeSetQuery;
	QSqlQuery usersQuery;

	// Create new file, and create history for it
	//

	bool ok = query.exec("SELECT * FROM add_file(1, 'getFileHistoryTest', 1, 'TEST', '{}');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(1, '{%1}', 'blah-blah');").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_out (%1, '{%2}');").arg(m_firstUserForTest).arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT MAX(changesetId) FROM FileInstance WHERE fileID = %1").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int changesetIdForDataUpdate = query.value(0).toInt();

	ok = query.exec(QString("UPDATE FileInstance SET Data = 'testtesttest' WHERE changesetId = %1").arg(changesetIdForDataUpdate));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'blah-blah');").arg(m_firstUserForTest).arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	// Check history of the file with fileId

	ok = query.exec(QString("SELECT * FROM get_file_history(1, %1)").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = fileInstanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileId = %1 ORDER BY changeSetId desc").arg(fileId));

	QVERIFY2(ok == true, qPrintable(fileInstanceQuery.lastError().databaseText()));

	while (query.next())
	{
		QVERIFY2(fileInstanceQuery.next() == true, qPrintable(fileInstanceQuery.lastError().databaseText()));

		QVERIFY2(fileInstanceQuery.value("changeSetId").toInt() == query.value("changesetId").toInt(), qPrintable("Error: changesetId is not match!"));
		QVERIFY2(fileInstanceQuery.value("action").toInt() == query.value("action").toInt(), qPrintable("Error: action is not match"));

		int changeSetId = query.value("changesetId").toInt();

		ok = changeSetQuery.exec(QString("SELECT * FROM changeSet WHERE changeSetId = %1").arg(changeSetId));

		QVERIFY2(ok == true, qPrintable(changeSetQuery.lastError().databaseText()));
		QVERIFY2(changeSetQuery.first() == true, qPrintable(changeSetQuery.lastError().databaseText()));

		QVERIFY2(changeSetQuery.value("userId").toInt() == query.value("userId").toInt(), qPrintable("Error: userId not match!"));
		QVERIFY2(changeSetQuery.value("comment").toString() == query.value("comment").toString(), qPrintable("Error: comment is not match!"));

		ok = usersQuery.exec(QString("SELECT userName FROM users WHERE userId = %1").arg(query.value("userId").toInt()));

		QVERIFY2(ok == true, qPrintable(usersQuery.lastError().databaseText()));
		QVERIFY2(usersQuery.first() == true, qPrintable(usersQuery.lastError().databaseText()));

		QVERIFY2(usersQuery.value(0).toString() == query.value("userName").toString(), qPrintable("Error: wrong username returned"));
	}

	// Check file history with another user
	//

	ok = query.exec(QString("SELECT * FROM get_file_history(%1, %2)").arg(m_firstUserForTest).arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = fileInstanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileId = %1 ORDER BY changeSetId desc").arg(fileId));

	QVERIFY2(ok == true, qPrintable(fileInstanceQuery.lastError().databaseText()));

	while (query.next())
	{
		QVERIFY2(fileInstanceQuery.next() == true, qPrintable(fileInstanceQuery.lastError().databaseText()));
		QVERIFY2(fileInstanceQuery.value("changeSetId").toInt() == query.value("changesetId").toInt(), qPrintable("Error: changesetId is not match!"));
		QVERIFY2(fileInstanceQuery.value("action").toInt() == query.value("action").toInt(), qPrintable("Error: action is not match"));
		int changeSetId = query.value("changesetId").toInt();

		ok = changeSetQuery.exec(QString("SELECT * FROM changeSet WHERE changeSetId = %1").arg(changeSetId));

		QVERIFY2(ok == true, qPrintable(changeSetQuery.lastError().databaseText()));
		QVERIFY2(changeSetQuery.first() == true, qPrintable(changeSetQuery.lastError().databaseText()));
		QVERIFY2(changeSetQuery.value("userId").toInt() == query.value("userId").toInt(), qPrintable("Error: userId not match!"));
		QVERIFY2(changeSetQuery.value("comment").toString() == query.value("comment").toString(), qPrintable("Error: comment is not match!"));

		ok = usersQuery.exec(QString("SELECT userName FROM users WHERE userId = %1").arg(query.value("userId").toInt()));

		QVERIFY2(ok == true, qPrintable(usersQuery.lastError().databaseText()));
		QVERIFY2(usersQuery.first() == true, qPrintable(usersQuery.lastError().databaseText()));

		QVERIFY2(usersQuery.value(0).toString() == query.value("userName").toString(), qPrintable("Error: wrong username returned"));
	}

	// Check if function returns nothing
	//

	ok = query.exec(QString("SELECT * FROM get_file_history(1, %1)").arg(FileTests::maxValueId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Invalid fileId error expected"));

	// Just added file must have no history
	//

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'getFileHistoryTestFailFile', 1, 'TEST', '{}')").arg(m_firstUserForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_file_history(1, %1)").arg(query.value("id").toInt()));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Error: new file must not has history"));
}

void FileTests::get_file_stateTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;
	int changeSetId = -1;

	// Add file for test
	//

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'getFileStateTestFirst', 1, 'TEST', '{}')").arg(m_firstUserForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fileId = query.value("id").toInt();

	// Add file and delete it to set "delete" flag
	//

	ok = query.exec("SELECT * FROM add_file(1, 'getFileStateTestSecond', 1, 'TEST', '{}')");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int deleted_file = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(1, '{%1}', 'TEST');").arg(deleted_file));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_file(1, %1)").arg(deleted_file));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_in(1, '{%1}', 'TEST');").arg(deleted_file));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	std::vector<int> dataForTest = {fileId, 1, deleted_file};

	for (size_t i = 0; i < dataForTest.size(); i++)
	{
		ok = query.exec (QString("SELECT * FROM get_file_state(%1)").arg(dataForTest[i]));

		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(dataForTest[i]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.value("deleted").toBool() == query.value("deleted").toBool(), qPrintable("Error: value \"deleted\" not match!"));

		if (query.value("deleted").toBool() == false)
		{
			if (query.value("checkedOut").toBool() == true)
			{
				QVERIFY2(query.value("errCode").toInt() == 0, qPrintable(QString("Error with code %1").arg(query.value("errCode").toInt())));

				QVERIFY2(tempQuery.value("checkedOutInstanceId").toString() != "", qPrintable("Error: function returned checkedOut true, Expected false"));

				ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedOutInstanceId").toString()));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable("Wrong action!"));

				ok = tempQuery.exec(QString("SELECT userId FROM checkOut WHERE fileId = %1").arg(dataForTest[i]));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.value(0).toInt() == query.value("userId").toInt(), qPrintable("Error in column \"userId\"!"));
			}
			else
			{
				QVERIFY2(query.value("errCode").toInt() == 0, qPrintable(QString("Error with code %1").arg(query.value("errCode").toInt())));

				QVERIFY2(tempQuery.value("checkedOutInstanceId").toString() == "", qPrintable("Error: function returned checkedOut false	, Expected true"));

				ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedInInstanceId").toString()));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable("Wrong action!"));

				changeSetId = tempQuery.value("changeSetId").toInt();

				ok = tempQuery.exec(QString("SELECT userId FROM changeSet WHERE changeSetId = %1").arg(changeSetId));

				QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2(tempQuery.value(0).toInt() == query.value("userId").toInt(), qPrintable("Error in column \"userId\"!"));
			}
		}
		else
		{
			QVERIFY2(query.value("checkedOut").toBool() == false, qPrintable("Error: value \"checkedOut\" with \"deleted\" flag not match!"));
		}
	}

	ok = query.exec(QString("SELECT * FROM get_file_state(%1)").arg(maxValueId));

	QVERIFY2(ok == false, qPrintable("Invalid fileId error expected"));
}

void FileTests::get_last_changesetTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;
	bool ok = query.exec("SELECT * FROM get_last_changeset()");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec("SElECT MAX(changesetId) FROM changeSet");

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value(0).toInt() == query.value(0).toInt(), qPrintable("Error: wrong changeset!"));
}

void FileTests::get_file_IdIntegerTextTest()
{
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'getFileIdTest', 7, 'foxes everywhere', '{}');").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM get_file_id(%1, '///$root$/MC/getFileIdTest///');").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(fileId == query.value(0).toInt(), qPrintable("Error: fileId not match!"));

	ok = query.exec("SELECT * FROM get_file_id(1, '///$root$/MC/getFileIdTest///');");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(fileId == query.value(0).toInt(), qPrintable("Error: fileId not match!"));

	ok = query.exec(QString("SELECT * FROM get_file_id(%1, '///$root$/MC/getFileIdTest///');").arg(m_secondUserForTest));
	QVERIFY2(ok == false, qPrintable("Wrong user error expected"));

	ok = query.exec("SELECT * FROM get_file_id(1, '///$root$/MC/abcdef///');");
	QVERIFY2(ok == false, qPrintable("NULL fileId error expected"));

	ok = query.exec(QString("SELECT * FROM add_or_update_file(%1, '$root$/MC/', 'getFileIdTest', 'simple comment', 'foxes everywhere', '{}');").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_file_id(%1, '///$root$/MC/getFileIdTest///');").arg(m_secondUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(fileId == query.value(0).toInt(), qPrintable("Error: other user must has acsess to file after update"));
}

void FileTests::get_file_IdIntegerIntegerTextTest()
{
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'getFileIdTest1', 7, 'foxes everywhere', '{}');").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM get_file_id(%1, 7, 'getFileIdTest1');").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(fileId == query.value(0).toInt(), qPrintable("Error: fileId not match!"));

	ok = query.exec("SELECT * FROM get_file_id(1, 7, 'getFileIdTest1');");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(fileId == query.value(0).toInt(), qPrintable("Error: fileId not match!"));

	ok = query.exec(QString("SELECT * FROM get_file_id(%1, 'getFileIdTest1');").arg(m_secondUserForTest));
	QVERIFY2(ok == false, qPrintable("Wrong user error expected"));

	ok = query.exec("SELECT * FROM get_file_id(1, 7, 'abcdef');");
	QVERIFY2(ok == false, qPrintable("NULL fileId error expected"));

	ok = query.exec("SELECT * FROM get_file_id(1, 0, 'getFileIdTest1');");
	QVERIFY2(ok == false, qPrintable("NULL fileId error expected"));

	ok = query.exec(QString("SELECT * FROM add_or_update_file(%1, '$root$/MC/', 'getFileIdTest1', 'simple comment', 'foxes everywhere', '{}');").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_file_id(%1, 7, 'getFileIdTest1');").arg(m_secondUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(fileId == query.value(0).toInt(), qPrintable("Error: other user must has acsess to file after update"));
}

void FileTests::get_file_infoTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;
	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	// Create checked out file to test
	//

	bool ok = query.exec(QString("SELECT * FROM add_file(1, 'getFileInfoCheckedOutFile', 1, 'TESTTESTTESTTEST', '%1');").arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int checkedOutFileId = query.value("id").toInt();

	// Create checked in file to test
	//

	ok = query.exec(QString("SELECT * FROM add_file(1, 'getFileInfoCheckedInFile', 1, 'TESTTESTTESTTEST', '%1');").arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int checkedInFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(1, '{%1}', 'TEST');").arg(checkedInFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int fileIds[2] = {checkedOutFileId, checkedInFileId};
	int fileIdNumber = 0;
	QString fileInstanceId;

	ok = query.exec(QString("SELECT * FROM get_file_info(1, '{%1,%2}') ORDER BY fileId;").arg(checkedOutFileId).arg(checkedInFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next())
	{
		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("fileId").toInt() == query.value("fileId").toInt(), qPrintable(QString("Error: fileId not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("deleted").toBool() == query.value("deleted").toBool(), qPrintable(QString("Error: deleted not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("name").toString() == query.value("name").toString(), qPrintable(QString("Error: string not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("parentId").toInt() == query.value("parentId").toInt(), qPrintable(QString("Error: parentId not match in fileId %1").arg(fileIds[fileIdNumber])));

		if (fileIdNumber == 0)
		{
			fileInstanceId = tempQuery.value("checkedOutInstanceId").toString();
		}
		else
		{
			fileInstanceId = tempQuery.value("checkedInInstanceId").toString();
		}

		ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(fileInstanceId));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("changesetId").toInt() == query.value("changesetId").toInt(), qPrintable(QString("Error: changesetId not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("Error: size not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("Error: action not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("details").toString() == query.value("details").toString(), qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(query.value("details").toString()).arg(tempQuery.value("details").toString())));


		ok = tempQuery.exec(QString("SELECT * FROM is_file_checkedOut(%1);").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.value(0).toBool() == query.value("checkedOut").toBool(), qPrintable(QString("Error: checkedOut not match in fileId %1").arg(fileIds[fileIdNumber])));

		if (tempQuery.value(0).toBool() == true)
		{
			ok = tempQuery.exec(QString("SELECT * FROM checkOut WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(query.value("userId").toInt() == tempQuery.value("userId").toInt(), qPrintable(QString("Error: userId not match in fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(query.value("checkedOut").toBool() == true, qPrintable("Error: not checkedOut"));
		}
		else
		{
			ok = tempQuery.exec(QString("SELECT * FROM changeset WHERE changesetId = %1").arg(query.value("changesetId").toInt()));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(query.value("userId").toInt() == tempQuery.value("userId").toInt(), qPrintable(QString("Error: userId not match in fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(query.value("checkedOut").toBool() == false, qPrintable("Error: must not be checkedOut"));
		}
		fileIdNumber++;
	}

	ok = query.exec(QString("SELECT * FROM get_file_info(1, '{%1}')").arg(maxValueId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Empty string expected (invalid fileId)"));
}

void FileTests::get_latest_file_versionTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;
	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	// Create file and check in it to test
	//

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'get_latest_file_versionTestCheckedOutFile', 1, 'Hello TUX!', '%2');").arg(m_firstUserForTest).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int checkedInFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'TUX is busy :(');").arg(m_firstUserForTest).arg(checkedInFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'get_latest_file_versionTestCheckedInFile', 1, 'Hello TUX!', '%2');").arg(m_firstUserForTest).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int checkedOutFileId = query.value("id").toInt();

	int fileIds[2] = {checkedOutFileId, checkedInFileId};
	// Testing checked in file
	//

	for (int NumberOfFileId = 0; NumberOfFileId < 2; NumberOfFileId++)
	{

		ok = query.exec(QString("SElECT * FROM get_latest_file_version(%1, %2);").arg(m_firstUserForTest).arg(fileIds[NumberOfFileId]));

		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

		QVERIFY2(query.value("fileId").toInt() == fileIds[NumberOfFileId], qPrintable(QString("fileId is not match at fileId %1").arg(fileIds[NumberOfFileId])));

		ok = tempQuery.exec(QString("SELECT * FROM is_file_checkedOut(%1)").arg(fileIds[NumberOfFileId]));
		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		bool checkedOut = tempQuery.value(0).toBool();

		QVERIFY2(checkedOut == query.value("checkedOut").toBool(), qPrintable(QString("checkedOut is not match at fileId %1").arg(fileIds[NumberOfFileId])));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[NumberOfFileId]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(query.value("deleted").toBool() == tempQuery.value("deleted").toBool(), qPrintable(QString("deleted is not match at fileId %1").arg(fileIds[NumberOfFileId])));
		QVERIFY2(query.value("name").toString() == tempQuery.value("name").toString(), qPrintable(QString("name is not match at fileId %1").arg(fileIds[NumberOfFileId])));
		QVERIFY2(query.value("parentId").toInt() == tempQuery.value("parentId").toInt(), qPrintable(QString("parentId is not match at fileId %1").arg(fileIds[NumberOfFileId])));

		if (checkedOut == false)
		{
			ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedInInstanceId").toString()));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value("changesetId").toInt() == query.value("changesetId").toInt(), qPrintable(QString("changesetId is not match at fileId %1").arg(fileIds[NumberOfFileId])));
			QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("size is not match at fileId %1").arg(fileIds[NumberOfFileId])));
			QVERIFY2(tempQuery.value("data").toString() == query.value("data").toString(), qPrintable(QString("data is not match at fileId %1").arg(fileIds[NumberOfFileId])));
			QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("action is not match at fileId %1").arg(fileIds[NumberOfFileId])));
			QVERIFY2(tempQuery.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(detail)));

			ok = tempQuery.exec(QString("SELECT * FROM changeset WHERE changesetId = %1").arg(query.value("changesetId").toInt()));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value("userId").toInt() == query.value("userId").toInt(), qPrintable(QString("userId is not match at fileId %1").arg(fileIds[NumberOfFileId])));
		}
		else
		{
			ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedOutInstanceId").toString()));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value("changesetId").toInt() == query.value("changesetId").toInt(), qPrintable(QString("changesetId is not match at fileId %1").arg(fileIds[NumberOfFileId])));
			QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("size is not match at fileId %1").arg(fileIds[NumberOfFileId])));
			QVERIFY2(tempQuery.value("data").toString() == query.value("data").toString(), qPrintable(QString("data is not match at fileId %1").arg(fileIds[NumberOfFileId])));
			QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("action is not match at fileId %1").arg(fileIds[NumberOfFileId])));
			QVERIFY2(tempQuery.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(detail)));

			ok = tempQuery.exec(QString("SELECT * FROM checkOut WHERE fileId = %1").arg(fileIds[NumberOfFileId]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value("userId").toInt() == query.value("userId").toInt(), qPrintable(QString("userId is not match at fileId %1").arg(fileIds[NumberOfFileId])));
		}
	}
	// Call invalid file error
	//

	ok = query.exec(QString("SElECT * FROM get_latest_file_version(1, %1);").arg(maxValueId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Empty row expected"));
}

void FileTests::get_file_listIntegerIntegerTextTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;
	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	// Create some files to test: 1st file must not be checked in
	// 2nd file must be checked in, 3rd file must be checked in - deleted - checked in
	// 4th file must has wrong parent, and 5th file must has wrong name
	//

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'get_file_listFirst', 1, 'Mandriva', '%2');").arg(m_firstUserForTest).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int firstFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'get_file_listSecond', 1, 'Slackware', '%2');").arg(m_firstUserForTest).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int secondFileId = query.value("id").toInt();

	// Check in one of files
	//

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_firstUserForTest).arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'get_file_listThird', 1, 'FreeBSD', '%2');").arg(m_firstUserForTest).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int thirdFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_firstUserForTest).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_file(%1, %2);").arg(m_firstUserForTest).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_firstUserForTest).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'get_file_listFourth', 0, 'FreeBSD', '%2');").arg(m_firstUserForTest).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fourthFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_firstUserForTest).arg(fourthFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'letsBreakFunctionByCreatingWrongNames', 1, 'Gentoo', '%2');").arg(m_firstUserForTest).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fifthFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_firstUserForTest).arg(fifthFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_file_list(%1, 1, 'get_file_list%') ORDER BY fileId").arg(m_firstUserForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int fileIds[5] = {firstFileId, secondFileId, thirdFileId, fourthFileId, fifthFileId};
	int fileIdNumber = 0;

	while (query.next())
	{
		QVERIFY2(fileIdNumber != 3, qPrintable(QString("Error: wrong fileId detectd %1").arg(fileIds[fileIdNumber])));

		QVERIFY2(query.value("fileId").toInt() == fileIds[fileIdNumber], qPrintable(QString("Error: unexpected fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("deleted").toBool() == query.value("deleted").toBool(), qPrintable(QString("Error in column deleted at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("name").toString() == query.value("name").toString(), qPrintable(QString("Error in column name at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("parentId").toInt() == query.value("parentId").toInt(), qPrintable(QString("Error in column name at fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM is_file_checkedOut(%1)").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value(0).toBool() == query.value("checkedOut").toBool(), qPrintable(QString("checkedOut is not match at fileId %1").arg(fileIds[fileIdNumber])));

		if (tempQuery.value(0).toBool() == true)
		{
			ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedOutInstanceId").toString()));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value("changeSetId").toInt() == query.value("changeSetId").toInt(), qPrintable(QString("changeSetId is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("size is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("action is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("details").toString() == query.value("details"), qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(query.value("details").toString())));

			ok = tempQuery.exec(QString("SELECT userId FROM checkOut WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value(0).toInt() == query.value("userId").toInt(), qPrintable(QString("userId is not match at fileId %1").arg(fileIds[fileIdNumber])));
		}
		else
		{
			ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedInInstanceId").toString()));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			int changeSetId = tempQuery.value("changeSetId").toInt();

			QVERIFY2(tempQuery.value("changeSetId").toInt() == query.value("changeSetId").toInt(), qPrintable(QString("changeSetId is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("size is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("action is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(detail)));

			ok = tempQuery.exec(QString("SELECT userId FROM changeSet WHERE changeSetId = %1").arg(changeSetId));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value(0).toInt() == query.value("userId").toInt(), qPrintable(QString("userId is not match at fileId %1").arg(fileIds[fileIdNumber])));
		}

		fileIdNumber++;
	}

	// Start test as second user
	//

	// CheckOut and change one of the files first, to make sure
	// that second user has no rights to work with checkedOut file
	// version, and function will return checkedIn version of the
	// file
	//

	ok = query.exec(QString("SELECT * FROM check_out(%1, '{%2}');").arg(m_firstUserForTest).arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SElECT * FROM delete_file(%1, %2);").arg(m_firstUserForTest).arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Try execute function by another user
	//

	ok = query.exec(QString("SELECT * FROM get_file_list(%1, 1, 'get_file_list%') ORDER BY fileId").arg(m_secondUserForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// First file was not checked in. Then executted by another user
	// function must not recognize it.
	//

	fileIdNumber = 1;

	while (query.next() == true)
	{
		QVERIFY2(fileIdNumber != 3, qPrintable(QString("Error: wrong fileId detectd %1").arg(fileIds[fileIdNumber])));

		QVERIFY2(query.value("fileId").toInt() == fileIds[fileIdNumber], qPrintable(QString("Error: unexpected fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("deleted").toBool() == query.value("deleted").toBool(), qPrintable(QString("Error in column deleted at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("name").toString() == query.value("name").toString(), qPrintable(QString("Error in column name at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("parentId").toInt() == query.value("parentId").toInt(), qPrintable(QString("Error in column name at fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedInInstanceId").toString()));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		int changeSetId = tempQuery.value("changeSetId").toInt();

		QVERIFY2(tempQuery.value("changeSetId").toInt() == query.value("changeSetId").toInt(), qPrintable(QString("changeSetId is not match at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("size is not match at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("action is not match at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(detail)));

		if (fileIds[fileIdNumber] == secondFileId)
		{
			QVERIFY2(tempQuery.value("action").toInt() != 3, qPrintable ("Error: Function returned checkedOut file version to wrong user!"));
		}

		ok = tempQuery.exec(QString("SELECT userId FROM changeSet WHERE changeSetId = %1").arg(changeSetId));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value(0).toInt() == query.value("userId").toInt(), qPrintable(QString("userId is not match at fileId %1").arg(fileIds[fileIdNumber])));

		fileIdNumber++;
	}

	// Execute function as administrator
	//

	ok = query.exec("SELECT * FROM get_file_list(1, 1, 'get_file_list%') ORDER BY fileId");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	fileIdNumber = 0;

	while (query.next())
	{
		QVERIFY2(fileIdNumber != 3, qPrintable(QString("Error: wrong fileId detectd %1").arg(fileIds[fileIdNumber])));

		QVERIFY2(query.value("fileId").toInt() == fileIds[fileIdNumber], qPrintable(QString("Error: unexpected fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("deleted").toBool() == query.value("deleted").toBool(), qPrintable(QString("Error in column deleted at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("name").toString() == query.value("name").toString(), qPrintable(QString("Error in column name at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("parentId").toInt() == query.value("parentId").toInt(), qPrintable(QString("Error in column name at fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM is_file_checkedOut(%1)").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value(0).toBool() == query.value("checkedOut").toBool(), qPrintable(QString("checkedOut is not match at fileId %1").arg(fileIds[fileIdNumber])));

		if (tempQuery.value(0).toBool() == true)
		{
			ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedOutInstanceId").toString()));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value("changeSetId").toInt() == query.value("changeSetId").toInt(), qPrintable(QString("changeSetId is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("size is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("action is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("details").toString() == query.value("details"), qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(query.value("details").toString())));

			ok = tempQuery.exec(QString("SELECT userId FROM checkOut WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value(0).toInt() == query.value("userId").toInt(), qPrintable(QString("userId is not match at fileId %1").arg(fileIds[fileIdNumber])));
		}
		else
		{
			ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedInInstanceId").toString()));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			int changeSetId = tempQuery.value("changeSetId").toInt();

			QVERIFY2(tempQuery.value("changeSetId").toInt() == query.value("changeSetId").toInt(), qPrintable(QString("changeSetId is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("size is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("action is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(detail)));

			ok = tempQuery.exec(QString("SELECT userId FROM changeSet WHERE changeSetId = %1").arg(changeSetId));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value(0).toInt() == query.value("userId").toInt(), qPrintable(QString("userId is not match at fileId %1").arg(fileIds[fileIdNumber])));
		}

		fileIdNumber++;
	}


	ok = query.exec(QString("SELECT * FROM get_file_list(%1, 2, 'get_file_list%') ORDER BY fileId").arg(m_firstUserForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Empty row expected"));
}

void FileTests::get_file_listIntegerIntegerTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;
	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	// Create some files to test: 1st file must not be checked in
	// 2nd file must be checked in, 3rd file must be checked in - deleted - checked in
	// 4th file must has wrong parent, and 5th file must has wrong name
	//

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'randomParentNameForGetFileListTest', 1, 'FatherGentoo', '%2');").arg(m_firstUserForTest).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int parentFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'randomNameForGetFileListTest', %2, 'Mandriva', '%3');").arg(m_firstUserForTest).arg(parentFileId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int firstFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'randomNameForGetFileListTest1', %2, 'Slackware', '%3');").arg(m_firstUserForTest).arg(parentFileId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int secondFileId = query.value("id").toInt();

	// Check in one of files
	//

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_firstUserForTest).arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'randomNameForGetFileListTest2', %2, 'FreeBSD', '%3');").arg(m_firstUserForTest).arg(parentFileId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int thirdFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_firstUserForTest).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_file(%1, %2);").arg(m_firstUserForTest).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_firstUserForTest).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'randomNameForGetFileListTest3', 0, 'FreeBSD', '%2');").arg(m_firstUserForTest).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fourthFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_firstUserForTest).arg(fourthFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_file_list(%1, %2) ORDER BY fileId").arg(m_firstUserForTest).arg(parentFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int fileIds[4] = {firstFileId, secondFileId, thirdFileId, fourthFileId};
	int fileIdNumber = 0;

	while (query.next())
	{
		QVERIFY2(fileIdNumber != 3, qPrintable(QString("Error: wrong fileId detectd %1").arg(fileIds[fileIdNumber])));

		QVERIFY2(query.value("fileId").toInt() == fileIds[fileIdNumber], qPrintable(QString("Error: unexpected fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("deleted").toBool() == query.value("deleted").toBool(), qPrintable(QString("Error in column deleted at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("name").toString() == query.value("name").toString(), qPrintable(QString("Error in column name at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("parentId").toInt() == query.value("parentId").toInt(), qPrintable(QString("Error in column name at fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM is_file_checkedOut(%1)").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value(0).toBool() == query.value("checkedOut").toBool(), qPrintable(QString("checkedOut is not match at fileId %1").arg(fileIds[fileIdNumber])));

		if (tempQuery.value(0).toBool() == true)
		{
			ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedOutInstanceId").toString()));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value("changeSetId").toInt() == query.value("changeSetId").toInt(), qPrintable(QString("changeSetId is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("size is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("action is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("details").toString() == query.value("details"), qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(query.value("details").toString())));

			ok = tempQuery.exec(QString("SELECT userId FROM checkOut WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value(0).toInt() == query.value("userId").toInt(), qPrintable(QString("userId is not match at fileId %1").arg(fileIds[fileIdNumber])));
		}
		else
		{
			ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedInInstanceId").toString()));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			int changeSetId = tempQuery.value("changeSetId").toInt();

			QVERIFY2(tempQuery.value("changeSetId").toInt() == query.value("changeSetId").toInt(), qPrintable(QString("changeSetId is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("size is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("action is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(detail)));

			ok = tempQuery.exec(QString("SELECT userId FROM changeSet WHERE changeSetId = %1").arg(changeSetId));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value(0).toInt() == query.value("userId").toInt(), qPrintable(QString("userId is not match at fileId %1").arg(fileIds[fileIdNumber])));
		}

		fileIdNumber++;
	}

	// Start test as second user
	//

	// CheckOut and change one of the files first, to make sure
	// that second user has no rights to work with checkedOut file
	// version
	//

	ok = query.exec(QString("SELECT * FROM check_out(%1, '{%2}');").arg(m_firstUserForTest).arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SElECT * FROM delete_file(%1, %2);").arg(m_firstUserForTest).arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Try execute function by another user
	//

	ok = query.exec(QString("SELECT * FROM get_file_list(%1, %2) ORDER BY fileId").arg(m_secondUserForTest).arg(parentFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// First file was not checked in. Then executted by another user
	// function must not recognize it
	//

	fileIdNumber = 1;

	while (query.next() == true)
	{
		QVERIFY2(fileIdNumber != 3, qPrintable(QString("Error: wrong fileId detectd %1").arg(fileIds[fileIdNumber])));

		QVERIFY2(query.value("fileId").toInt() == fileIds[fileIdNumber], qPrintable(QString("Error: unexpected fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("deleted").toBool() == query.value("deleted").toBool(), qPrintable(QString("Error in column deleted at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("name").toString() == query.value("name").toString(), qPrintable(QString("Error in column name at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("parentId").toInt() == query.value("parentId").toInt(), qPrintable(QString("Error in column name at fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedInInstanceId").toString()));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		int changeSetId = tempQuery.value("changeSetId").toInt();

		QVERIFY2(tempQuery.value("changeSetId").toInt() == query.value("changeSetId").toInt(), qPrintable(QString("changeSetId is not match at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("size is not match at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("action is not match at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(detail)));

		if (fileIds[fileIdNumber] == secondFileId)
		{
			QVERIFY2(tempQuery.value("action").toInt() != 3, qPrintable ("Error: Function returned checkedOut file version to wrong user!"));
		}

		ok = tempQuery.exec(QString("SELECT userId FROM changeSet WHERE changeSetId = %1").arg(changeSetId));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value(0).toInt() == query.value("userId").toInt(), qPrintable(QString("userId is not match at fileId %1").arg(fileIds[fileIdNumber])));

		fileIdNumber++;
	}

	// Execute function as administrator
	//

	ok = query.exec(QString("SELECT * FROM get_file_list(1, %1) ORDER BY fileId").arg(parentFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	fileIdNumber = 0;

	while (query.next())
	{
		QVERIFY2(fileIdNumber != 3, qPrintable(QString("Error: wrong fileId detectd %1").arg(fileIds[fileIdNumber])));

		QVERIFY2(query.value("fileId").toInt() == fileIds[fileIdNumber], qPrintable(QString("Error: unexpected fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("deleted").toBool() == query.value("deleted").toBool(), qPrintable(QString("Error in column deleted at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("name").toString() == query.value("name").toString(), qPrintable(QString("Error in column name at fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("parentId").toInt() == query.value("parentId").toInt(), qPrintable(QString("Error in column name at fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM is_file_checkedOut(%1)").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value(0).toBool() == query.value("checkedOut").toBool(), qPrintable(QString("checkedOut is not match at fileId %1").arg(fileIds[fileIdNumber])));

		if (tempQuery.value(0).toBool() == true)
		{
			ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedOutInstanceId").toString()));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value("changeSetId").toInt() == query.value("changeSetId").toInt(), qPrintable(QString("changeSetId is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("size is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("action is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("details").toString() == query.value("details"), qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(query.value("details").toString())));

			ok = tempQuery.exec(QString("SELECT userId FROM checkOut WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value(0).toInt() == query.value("userId").toInt(), qPrintable(QString("userId is not match at fileId %1").arg(fileIds[fileIdNumber])));
		}
		else
		{
			ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedInInstanceId").toString()));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			int changeSetId = tempQuery.value("changeSetId").toInt();

			QVERIFY2(tempQuery.value("changeSetId").toInt() == query.value("changeSetId").toInt(), qPrintable(QString("changeSetId is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("size is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("action is not match at fileId %1").arg(fileIds[fileIdNumber])));
			QVERIFY2(tempQuery.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(detail)));

			ok = tempQuery.exec(QString("SELECT userId FROM changeSet WHERE changeSetId = %1").arg(changeSetId));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2(tempQuery.value(0).toInt() == query.value("userId").toInt(), qPrintable(QString("userId is not match at fileId %1").arg(fileIds[fileIdNumber])));
		}

		fileIdNumber++;
	}

	ok = query.exec(QString("SELECT * FROM get_file_list(%1, %2) ORDER BY fileId").arg(m_firstUserForTest).arg(parentFileId + 1));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Empty row expected"));
}

void FileTests::get_latest_file_tree_versionTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;
	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	// Create fileid for test
	//

	bool ok = query.exec(QString("SElECT * FROM add_file(1, 'getLatestFileTreeVersionFirstFile', 0, 'FreeBSD', '%1');").arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int firstFileId = query.value("id").toInt();

	// Create children file of first file for test
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'getLatestFileTreeVersionSecondFile', %1, 'OpenBSD', '%2');").arg(firstFileId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int secondFileId = query.value("id").toInt();

	// Create children file of second file for test
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'getLatestFileTreeVersionThirdFile', %1, 'NetBSD', '%2');").arg(secondFileId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int thirdFileId = query.value("id").toInt();

	// Create children file from first file
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'getLatestFileTreeVersionFourthFile', %1, 'PC-BSD', '%2');").arg(firstFileId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fourthFileId = query.value("id").toInt();

	// Create file with wrong parentId
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'getLatestFileTreeVersionFifthFile', 2, 'Mandrake', '%1');").arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fifthFileId = query.value("id").toInt();

	ok = query.exec(QString("SElECT * FROM get_latest_file_tree_version (1, %1);").arg(firstFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int fileIds[5] = {firstFileId, secondFileId, thirdFileId, fourthFileId, fifthFileId};
	int fileIdNumber = 0;

	while (query.next())
	{
		QVERIFY2(fileIdNumber != 4, qPrintable(QString("Error: wrong fileId detectd %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("deleted").toBool() == query.value("deleted").toBool(), qPrintable(QString("Error: deleted not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("name").toString() == query.value("name").toString(), qPrintable(QString("Error: name not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("parentId").toInt() == query.value("parentId").toInt(), qPrintable(QString("Error: parentId not match in fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedOutInstanceId").toString()));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("changeSetId").toInt() == query.value("changeSetId").toInt(), qPrintable(QString("Error: changSetId not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("Error: size not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("data").toString() == query.value("data").toString(), qPrintable(QString("Error: data not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("Error: action not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(tempQuery.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(detail)));

		ok = tempQuery.exec(QString("SELECT * FROM is_file_checkedOut(%1)").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value(0).toBool() == query.value("checkedOut").toBool(), qPrintable(QString("Error: checkedOut not match in fileId %1").arg(fileIds[fileIdNumber])));

		ok = tempQuery.exec(QString("SELECT * FROM checkout WHERE fileId = %1").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		QVERIFY2(tempQuery.value("userId").toInt() == query.value("userId").toInt(), qPrintable(QString("Error: userId not match in fileId %1").arg(fileIds[fileIdNumber])));

		fileIdNumber++;
	}

	ok = query.exec(QString("SElECT * FROM get_latest_file_tree_version (1, %1);").arg(maxValueId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Invalid fileId error expected"));

	ok = query.exec(QString("SElECT * FROM get_latest_file_tree_version (%1, %2);").arg(m_firstUserForTest).arg(firstFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Invalid user error expected"));
}

void FileTests::undo_changesTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	// Create ordinary file for test
	//

	bool ok = query.exec("SElECT * FROM add_file(1, 'undoChangesFirstTest', 1, 'KDE4', '{}');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int firstFileId = query.value("id").toInt();

	// Create file with history (checkedIn - checkedOut) for test
	//

	ok = query.exec("SElECT * FROM add_file(1, 'undoChangesSecondTest', 1, 'GNOME2', '{}');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int secondFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(1, '{%1}', 'I do not like KDE');").arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_out(1, '{%1}');").arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Create another ordinary file for test
	//

	ok = query.exec("SElECT * FROM add_file(1, 'undoChangesThirdTest', 1, 'xfce', '{}');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int thirdFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM undo_changes(1, '{%1, %2, %3}') ORDER BY id;").arg(firstFileId).arg(secondFileId).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int fileIds[3] = {firstFileId, secondFileId, thirdFileId};
	int fileIdNumber = 0;

	while (query.next())
	{
		if (fileIds[fileIdNumber] == firstFileId)
		{
			QVERIFY2(query.value("deleted").toBool() == true, qPrintable(query.lastError().databaseText()));
		}

		if (fileIds[fileIdNumber] == secondFileId)
		{
			QVERIFY2(query.value("deleted").toBool() == false, qPrintable(query.lastError().databaseText()));
		}

		if (fileIds[fileIdNumber] == thirdFileId)
		{
			QVERIFY2(query.value("deleted").toBool() == true, qPrintable(query.lastError().databaseText()));
		}

		if (query.value("deleted").toBool() == true)
		{
			ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == false, qPrintable("Error: file matched to be deleted"));
		}
		else
		{
			ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1 AND checkedOutInstanceId IS NULL").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

			ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileId = %1 AND changeSetId IS NULL;").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == false, qPrintable("Error: record in table fileInstance with NULL changesetId must be deleted"));

			ok = tempQuery.exec(QString("SELECT * FROM checkOut WHERE fileId = %1").arg(fileIds[fileIdNumber]));

			QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2(tempQuery.first() == false, qPrintable("Error: record in table checkout must be deleted"));
		}
		fileIdNumber++;
	}

	// Create file for call errors
	//

	ok = query.exec("SElECT * FROM add_file(1, 'undoChangesErrTest', 1, 'Windows', '{}');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int errorCallFileId = query.value("id").toInt();

	// Call wrong user error
	//

	ok = query.exec(QString("SELECT * FROM undo_changes(%1, '{%2}');").arg(m_firstUserForTest).arg(errorCallFileId));

	QVERIFY2(ok == false, qPrintable("Wrong user error expected"));

	ok = query.exec(QString("SELECT * FROM check_in(1, '{%1}', 'I do not like KDE');").arg(errorCallFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Call error of not checkedOut file
	//

	ok = query.exec(QString("SELECT * FROM undo_changes(1, '{%1}');").arg(errorCallFileId));

	QVERIFY2(ok == false, qPrintable("File is not checkedOut error expected"));
}

void FileTests::add_or_update_fileTest()
{
	QSqlQuery query;

	// Create file for update test
	//

	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	bool ok = query.exec("SElECT * FROM add_file(1, 'addOrUpdateFileTest', 7, 'Linux Kernel', '{}');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fileId = query.value("id").toInt();

	QString comment = "check_in comment";
	QString fileData = "data";

	// File update test
	//

	ok = query.exec(QString("SELECT * FROM add_or_update_file(1, '$root$/MC/', 'addOrUpdateFileTest', '%1', '%2', '%3');").arg(comment).arg(fileData).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("add_or_update_file").toInt() == fileId, qPrintable("Error: wrong fileId!"));

	ok = query.exec(QString("SElECT * FROM file WHERE fileId = %1").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("checkedOutInstanceId").toString() == "", qPrintable("Error: file stil checked out"));

	ok = query.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(query.value("checkedInInstanceId").toString()));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("data").toString() == fileData, qPrintable("Error: data not match!"));
	QVERIFY2(query.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(query.value("details").toString()).arg(detail)));

	ok = query.exec(QString("SELECT * FROM changeSet WHERE changeSetId = %1").arg(query.value("changeSetId").toString()));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("comment").toString() == comment, qPrintable("Error: wrong comment"));

	// File create test
	//

	ok = query.exec(QString("SELECT * FROM add_or_update_file(1, '$root$/MC/', 'addOrUpdateCreateFileTest', '%1', '%2', '%3');").arg(comment).arg(fileData).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileId = query.value("add_or_update_file").toInt();

	ok = query.exec(QString("SElECT * FROM file WHERE fileId = %1").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("parentId").toInt() == 7, qPrintable("Wrong parentId"));
	QVERIFY2(query.value("checkedOutInstanceId").toString() == "", qPrintable("Error: file stil checked out"));

	ok = query.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(query.value("checkedInInstanceId").toString()));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("data").toString() == fileData, qPrintable("Error: data not match!"));
	QVERIFY2(query.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(query.value("details").toString()).arg(detail)));

	ok = query.exec(QString("SELECT * FROM changeSet WHERE changeSetId = %1").arg(query.value("changeSetId").toString()));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("comment").toString() == comment, qPrintable("Error: wrong comment"));

	// Check files marked as deleted
	//

	ok = query.exec("SElECT * FROM add_file(1, 'addOrUpdateErrorUserTest', 7, 'Linux Kernel', '{}');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileId = query.value("id").toInt();

	ok = query.exec(QString("UPDATE file SET deleted = true WHERE fileId = %1").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_or_update_file(%1, '$root$/MC/', 'addOrUpdateErrorUserTest', 'deleted', 'deleted', '{}');").arg(m_firstUserForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM file WHERE fileId = %1 AND deleted = false").arg(fileId+1));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
}

void FileTests::get_specific_copyTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	bool ok = query.exec(QString("SElECT * FROM add_file(1, 'getSpecificFileCopyTest', 1, 'GNOME2', '%1');").arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(1, '{%1}', 'I do not like KDE');").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SElECT checkedInInstanceId FROM file WHERE fileId = %1").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT changeSetId FROM fileInstance WHERE fileInstanceId = '%1'").arg(query.value(0).toString()));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int changeSetId = query.value(0).toInt();

	ok = query.exec(QString("SELECT * FROM check_out(1, '{%1}');").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_specific_copy (1, %1, %2);").arg(fileId).arg(changeSetId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileId));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value("name").toString() == tempQuery.value("name").toString(), qPrintable("Error: name not match!)"));
	QVERIFY2(query.value("parentId").toInt() == tempQuery.value("parentId").toInt(), qPrintable("Error: parentId not match!)"));

	ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedInInstanceId").toString()));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value("size").toInt() == tempQuery.value("size").toInt(), qPrintable("Error: size not match!)"));
	QVERIFY2(query.value("data").toString() == tempQuery.value("data").toString(), qPrintable("Error: data not match!)"));
	QVERIFY2(query.value("action").toInt() == tempQuery.value("action").toInt(), qPrintable("Error: action not match!)"));
	QVERIFY2(query.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(query.value("details").toString()).arg(detail)));

	ok = tempQuery.exec(QString("SELECT * FROM changeSet WHERE changeSetId = %1").arg(changeSetId));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value("userId").toInt() == tempQuery.value("userId").toInt(), qPrintable("Error: userId not match!)"));

	// Call function with wrong changeSetId
	//

	ok = query.exec(QString("SELECT * FROM get_specific_copy (1, %1, %2);").arg(fileId).arg(maxValueId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT MAX(changesetId) FROM fileinstance WHERE fileId = %1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	int maxChangesetId = tempQuery.value(0).toInt();

	QVERIFY2(query.value("changeSetId").toInt() == maxChangesetId, qPrintable(QString("Error: wrong changesetId returned while entered %1 changesetId").arg(maxValueId)));

	ok = query.exec(QString("SELECT * FROM get_specific_copy (1, %1, -1);").arg(fileId));
	QVERIFY2(ok == false, qPrintable(query.lastError().databaseText()));

	// Call second function, which works with timestamps
	//

	QString dateTimeToCheck = "2016-12-05 15:59:21.530509+02";

	ok = query.exec(QString("UPDATE Fileinstance SET created = '%1' WHERE changesetId = %2 AND FileId = %3")
	                .arg(dateTimeToCheck)
	                .arg(maxChangesetId)
	                .arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_specific_copy (1, %1, TIMESTAMP WITH TIME ZONE '%2')")
	                .arg(fileId)
	                .arg(dateTimeToCheck));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("changesetId").toInt() == maxChangesetId, qPrintable("Wrong changeset returned (timestamp)"));
	QVERIFY2(query.value("fileId").toInt() == fileId, qPrintable("Wrong fileId returned (timestamp)"));
}

void FileTests::add_deviceTest()
{
	QSqlQuery query;

	QString data = "Source of linux kernel";
	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	bool ok = query.exec(QString("SELECT * FROM add_device(1, '%1', 1, '.src', '%2');").arg(data).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1 AND parentId=1").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1' AND data = '%2' AND details = '%3'")
					.arg(query.value("checkedOutInstanceId").toString())
					.arg(data)
					.arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM checkOut WHERE fileId=%1 AND userId=1").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
}

QString FileTests::filesExist(QString fileID)
{
	QSqlQuery query;
	if (query.exec(QString("SELECT files_exist('{%1}');").arg(fileID)) == false)
	{
		qDebug() << query.lastError().databaseText();
		return "Query executing error";
	}
	QString result = "";
	while (query.next())
	{
		if (query.value("files_exist").toString().indexOf("t")!=-1)
			result.append("true,");
		else
			result.append("false,");
	}
	result.remove(result.size()-1,1);
	return result;
}

bool FileTests::add_file(int userId, QString fileName, int parentId, QString fileData, QString details)
{
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, '%2', %3, '%4', '%5');")
						 .arg(userId)
						 .arg(fileName)
						 .arg(parentId)
						 .arg(fileData)
						 .arg(details));

	if (ok == false)
	{
		//qDebug() << query.lastError().databaseText();
		return ok;
	}

	if (query.next() == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	ObjectState resultQueryObject;
	FileTests::getObjectState(query, resultQueryObject);

	int fileId = resultQueryObject.id;
	int errCode = resultQueryObject.errCode;

	if (errCode != 0)
	{
		qDebug() << QString("Error: error code in result function is %1").arg(errCode);
		return false;
	}

	// Checking Table "file"
	//

	ok = query.exec("SELECT * FROM file WHERE fileId = " + QString::number(fileId));
	if (ok == false)
	{
		return false;
	}

	ok = query.first();
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}


	if (query.value("fileId").toInt() != fileId)
	{
		qDebug() << "Error in column \"fileId\" from \"files\"\nActual: " << query.value("fileId").toInt() << "\nExpected" << fileId;
		return false;
	}

	if (query.value("name").toString() != fileName)
	{
		qDebug() << "Error in column \"name\" from \"files\"\nActual: " << query.value("name").toString() << "\nExpected" << fileName;
		return false;
	}

	if (query.value("parentId").toInt() != parentId)
	{
		qDebug() << "Error in column \"parentId\" from \"files\"\nActual: " << query.value("parentId").toInt() << "\nExpected" << parentId;
		return false;
	}

	if (query.value("deleted").toBool() != false)
	{
		qDebug() << "Error in column \"deleted\" from \"files\"\nActual: " << query.value("deleted").toBool() << "\nExpected: " << false;
		return false;
	}

	if (query.value("checkedInInstanceId").toUuid().isNull() == false)
	{
		qDebug() << "Error in column \"checkedoutInstanceId\" from \"files\"\nActual: " << query.value("checkedIninstanceId").toString() << "\nExpected: " << "";
		return false;
	}

	if (query.value("checkedOutInstanceId").toUuid().isNull() == true)
	{
		qDebug() << "Error in column \"checkedoutOutInstanceId\" from \"files\"\nActual: " << query.value("checkedOutInstanceId").toString() << "\nExpected: (uuid)" << "";
		return false;
	}

	QUuid checkedOutInstanceId = query.value("checkedOutInstanceId").toUuid();

	// Checking Table "fileInstance"
	//

	ok = query.exec ("SELECT * FROM fileInstance WHERE fileId=" + QString::number(fileId));
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	ok = query.first();
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.value("data").toString() != fileData)
	{
		qDebug() << "Error in column \"data\" from \"fileInstance\"\nActual: " << query.value("data").toString() << "\nExpected: " << fileData;
		return false;
	}

	if (query.value("changesetId").toInt() != 0)
	{
		qDebug() << "Error in column \"changeSetId\" from \"fileInstance\"\nActual: " << query.value("changesetid").toString() << "\nExpected: " << "0";
		return false;
	}

	if (query.value("action").toInt() != 1)
	{
		qDebug() << "Error in column \"action\" from \"fileInstance\"\nActual: " << query.value("action").toString() << "\nExpected: " << "1";
		return false;
	}

	if (query.value("fileInstanceId").toUuid() != checkedOutInstanceId)
	{
		qDebug() << "Error in column \"fileInstanceId\" from \"fileInstance\"\nActual: " << query.value("fileInstanceId").toString() << "\nExpected: " << checkedOutInstanceId;
		return false;
	}

	if (query.value("details").toString() != details)
	{
		qDebug() << "Error in column \"details\" from \"fileInstance\"\nActual: " << query.value("details").toString() << "\nExpected: " << details;
		return false;
	}

	ok = query.exec(QString("SELECT COUNT(*) FROM fileInstance WHERE fileId = %1").arg(fileId));
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.first() == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.value(0).toInt() != 1)
	{
		qDebug() << "Error: There are one more record with fileId" << fileId;
		return false;
	}

	// Checking Table "CheckOut"
	//

	ok = query.exec ("SELECT * FROM checkOut WHERE fileId = " + QString::number(fileId));
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	ok = query.first();
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.value("userId").toInt() != userId)
	{
		qDebug() << "Error in column \"userId\" from \"checkOut\"\nActual: " << query.value("userId").toInt() << "\nExpected: " << userId;
		return false;
	}

	if (query.value("signalId").toInt() != 0)
	{
		qDebug() << "Error in column \"signalId\" from \"checkOut\"\nActual: " << query.value("signalId").toInt() << "\nExpected: " << 0;
		return false;
	}

	return true;
}

bool FileTests::delete_file(int userId, int fileId)
{
	QSqlQuery query;
	bool ok = query.exec(QString("SELECT * FROM delete_file(%1, %2)")
						 .arg(userId)
						 .arg(fileId));
	if (ok == false)
	{
		//qDebug() << query.lastError().databaseText();
		return false;
	}

	ok = query.first();
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.value("errCode").toInt() == 1)
	{
		//qDebug() << "Cannot checkout file";
		return false;
	}

	if (query.value("errCode").toInt() == 2)
	{
		//qDebug() << "Error, user " << userId << " has no rights";
		return false;
	}

	ok = query.exec(QString("SELECT COUNT(*) FROM file WHERE fileId = %1").arg(fileId));
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.first() == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.value(0).toInt() == 0)
	{
		return true;
	}

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM file WHERE fileid = %1").arg(fileId));
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.first() == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	ok = query.exec(QString("SELECT action FROM fileInstance WHERE fileInstanceId = '%1'").arg(query.value(0).toString()));
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.first() == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.value(0).toInt() != 3)
	{
		qDebug() << "Error: record is not matched to be deleted";
		return false;
	}

	ok = query.exec(QString ("SELECT check_in ('%1', '{%2}', 'Deleted %2')").arg(userId).arg(fileId));
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	ok = query.exec(QString("SELECT * FROM file WHERE fileid = %1")
					.arg(fileId));

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	ok = query.first();
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.value("deleted").toBool() == false)
	{
		qDebug() << "Result error: file has not been deleted from table \"file\"";
		return false;
	}

	return true;
}

void FileTests::get_checked_out_filesTest()
{
	QSqlQuery query;

	// Create fileid for test
	//

	int fileIds[7] = {0, 0, 0, 0, 0, 0, 0};

	bool ok = query.exec("SElECT * FROM add_file(1, 'getChecekdOutFilesFirstFile', 0, 'FreeBSD', '{}');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[0] = query.value("id").toInt();

	// Create children file of first file for test
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'getChecekdOutFilesSecondFile', %1, 'OpenBSD', '{}');").arg(fileIds[0]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[1] = query.value("id").toInt();

	// Create children file of second file for test
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'getChecekdOutFilesThirdFile', %1, 'NetBSD', '{}');").arg(fileIds[1]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[2] = query.value("id").toInt();

	// Create children file from first file
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'getChecekdOutFilesFourthFile', %1, 'PC-BSD', '{}');").arg(fileIds[0]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[3] = query.value("id").toInt();

	// Create file with random parent
	//

	ok = query.exec("SElECT * FROM add_file(1, 'getChecekdOutFilesRandomFile', 0, 'Yosemite', '{}');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[4] = query.value("id").toInt();

	// Create child of the file with random parent
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'getChecekdOutFilesRandomFileChild', %1, 'El Capitano', '{}');").arg(fileIds[4]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[5] = query.value("id").toInt();

	// Create file from first file, and check it in
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'getChecekdOutFilesFifthFile', %1, 'Solaris', '{}');").arg(fileIds[0]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[6] = query.value("id").toInt();

	ok = query.exec(QString("SElECT * FROM check_in(1, '{%1}', 'TEST');").arg(fileIds[6]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Start the function
	//

	ok = query.exec(QString("SELECT * FROM get_checked_out_files(1, '{%1, %2}') ORDER BY fileId").arg(fileIds[0]).arg(fileIds[4]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int numberOfFileId = 0;

	while (query.next())
	{
		QVERIFY2(query.value("fileId").toInt() != fileIds[6], qPrintable ("Error: function show checkedIn files"));

		QVERIFY2(query.value("fileId").toInt() == fileIds[numberOfFileId], qPrintable(QString("Error: fileId's mistmatch. Unexpected fileId's:\nActual: %1\nExpected: %2").arg(query.value("fileId").toInt()).arg(fileIds[numberOfFileId])));
		numberOfFileId++;
	}

	QVERIFY2(numberOfFileId == 6, qPrintable("Wrong count of records in result"));

	// Call invalid user error
	//

	ok = query.exec(QString("SELECT * FROM get_checked_out_files(%1, '{%2}');").arg(maxValueId).arg(fileIds[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Invalid user error expected"));
}

void FileTests::check_in_treeTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	int fileIds[7] = {0, 0, 0, 0, 0, 0, 0};

	bool ok = query.exec("SElECT * FROM add_file(1, 'checkInTreeTest', 0, 'FreeBSD', '{}');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[0] = query.value("id").toInt();

	// Create children file of first file for test
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'checkInTreeTestSecondFile', %1, 'OpenBSD', '{}');").arg(fileIds[0]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[1] = query.value("id").toInt();

	// Create children file of second file for test
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'checkInTreeTestThirdFile', %1, 'NetBSD', '{}');").arg(fileIds[1]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[2] = query.value("id").toInt();

	// Create children file from first file
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'checkInTreeTestFourthFile', %1, 'PC-BSD', '{}');").arg(fileIds[0]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[3] = query.value("id").toInt();

	// Create file with random parent
	//

	ok = query.exec("SElECT * FROM add_file(1, 'checkInTreeTestRandomFile', 0, 'Yosemite', '{}');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[4] = query.value("id").toInt();

	// Create child of the file with random parent, which will be deleted
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'checkInTreeTestRandomFileChild', %1, 'El Capitano', '{}');").arg(fileIds[4]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[5] = query.value("id").toInt();

	// Create file from first file, and check it in
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'checkInTreeTestFifthFile', %1, 'Solaris', '{}');").arg(fileIds[0]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[6] = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in (1, '{%1, %2}', 'TEST');").arg(fileIds[6]).arg(fileIds[5]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_out(1, '{%1}');").arg(fileIds[5]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_file (1, %1);").arg(fileIds[5]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_in_tree(1, '{%1, %2}', 'TEST') ORDER BY id").arg(fileIds[0]).arg(fileIds[4]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int fileNumber = 0;
	while (query.next())
	{
		QVERIFY2(query.value("id").toInt() != fileIds[6], qPrintable("Error: checked in file has been checked_in twice"));
		QVERIFY2(query.value("id").toInt() == fileIds[fileNumber], qPrintable(QString("Error: wrong fileId\nActual: %1\nExpected: %2").arg(query.value("id").toInt()).arg(fileIds[fileNumber])));

		if (query.value("id").toInt() == fileIds[5])
		{
			// Check deleted file
			//

			ok = tempQuery.exec(QString("SELECT deleted FROM file WHERE fileId = %1").arg(fileIds[5]));

			QVERIFY2 (ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2 (tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2 (tempQuery.value(0).toBool() == true, qPrintable ("File has not been deleted"));
		}
		else
		{
			// Check all data of the checkedIn file
			//

			ok = tempQuery.exec(QString("SELECT * FROM checkOut WHERE fileId = %1").arg(query.value("id").toInt()));
			QVERIFY2 (ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2 (tempQuery.next() == false, qPrintable("Error: file was not checked in"));

			ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(query.value("id").toInt()));
			QVERIFY2 (ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2 (tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2 (tempQuery.value("checkedInInstanceId").toString() != "", qPrintable("File table was not uppdated (checkedInInstanceId"));
			QVERIFY2 (tempQuery.value("checkedOutInstanceId").toString() == "", qPrintable ("File table was not updated (checkedOutInstanceId"));

			ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = '%1'").arg(tempQuery.value("checkedInInstanceId").toString()));

			QVERIFY2 (ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2 (tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2 (tempQuery.value("fileId").toInt() == fileIds[fileNumber], qPrintable ("Wrong fileId in table fileInstance"));
		}

		fileNumber++;
	}

	// Check amount of results
	//

	QVERIFY2 (fileNumber == 6, qPrintable ("Error: wrong file amount"));

	// Try call bug, when child file dublates in result
	//

	int dublicateTest[2] = {0, 0};

	ok = query.exec("SELECT * FROM add_file (1, 'checkInTreeFileDublicateTest', 1, 'comment', '{}')");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	dublicateTest[0] = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file (1, 'checkInTreeFileDublicateTest', %1, 'comment', '{}')").arg(dublicateTest[0]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	dublicateTest[1] = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in_tree(1, '{%1, %2}', 'TEST') ORDER BY id").arg(dublicateTest[0]).arg(dublicateTest[1]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	fileNumber = 0;
	while (query.next())
	{
		QVERIFY2 (fileNumber < 2, qPrintable ("Error: child file dublicates in result"));
		QVERIFY2 (query.value("id").toInt() == dublicateTest[fileNumber], qPrintable("Error: wrong id in dublicateTest"));

		fileNumber++;
	}
}
