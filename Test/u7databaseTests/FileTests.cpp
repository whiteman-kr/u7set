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

FileTests::FileTests() :
	m_user1{"User1", "UserP2ssw0rd", -1},
	m_user2{"User2", "UserP2ssw0rd", -1}
{
}

QString FileTests::logIn(User user)
{
	return logIn(user.username, user.password);
}

QString FileTests::logIn(QString username, QString password)
{
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT * FROM user_api.log_in('%1', '%2')")
							.arg(username)
							.arg(password));

	if (ok == false)
	{
		return QString("");
	}

	ok = query.next();
	if (ok == false)
	{
		return QString("");
	}

	QString session_key = query.value(0).toString();
	return session_key;
}

bool FileTests::logOut()
{
	QSqlQuery query;
	bool ok = query.exec("SELECT * FROM user_api.log_out()");
	return ok;
}

void FileTests::initTestCase()
{
	bool ok = createProjectDb();
	QVERIFY2(ok == true, "Cannot create projectdatabase");

	// Log in as Administartor to create test users
	//
	QSqlQuery query;
	ok = query.exec(QString("SELECT * FROM user_api.log_in('%1', '%2')")
						.arg(m_projectAdministratorName)
						.arg(m_projectAdministratorPassword));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QString session_key = query.value(0).toString();

	// Create Testers
	//
	ok = query.exec(QString("SELECT user_api.create_user('%1', '%2', '%2', '%2', '%3', false, false);")
						.arg(session_key)
						.arg(m_user1.username)
						.arg(m_user1.password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText() + ", Request:  " + query.lastQuery()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	m_user1.userId = query.value("create_user").toInt();

	// --
	//
	ok = query.exec(QString("SElECT user_api.create_user('%1', '%2', '%2', '%2', '%3', false, false);")
						.arg(session_key)
						.arg(m_user2.username)
						.arg(m_user2.password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	m_user2.userId = query.value("create_user").toInt();

	// Administartor Log out
	//
	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	return;
}

void FileTests::cleanupTestCase()
{
	dropProjectDb();
}

void FileTests::apiFileExistsTest()
{
	// LogIn as Admin
	//
	QString session_key = logIn(m_user1);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT api.is_file_exists('%1', 0, 'Schemas');").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toInt() != 0, qPrintable("Expected some file id (2)"));

	ok = query.exec(QString("SELECT api.is_file_exists('%1', 0, 'ALDOESND');").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).isNull() == true, qPrintable("Null is expected, as file does not exists"));

	// LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
}

void FileTests::api_set_file_attributes()
{
	// LogIn as Admin
	//
	QString session_key = logIn(m_user1);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// FUNCTION api.set_file_attributes(session_key text, full_file_name text, attr integer)
	//
	{
		QSqlQuery query;
		bool ok = query.exec(QString("SELECT api.set_file_attributes('%1', '%2', 123);")
								.arg(session_key)
								.arg(::AlFileName)	// "$root$/Schemas/ApplicationLogic"
							 );

		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

		ok = query.exec(QString("SELECT F.Attributes FROM File F WHERE FileID = (SELECT api.get_file_id('%1', '%2'));")
								.arg(session_key)
								.arg(::AlFileName)	// "$root$/Schemas/ApplicationLogic"
							 );

		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.value(0).toInt() == 123, qPrintable("Expected new attribute value 123"));
	}

	// FUNCTION api.set_file_attributes(text, integer, integer);
	//
	{
		QSqlQuery query;
		bool ok = query.exec(QString("SELECT api.set_file_attributes('%1', (SELECT api.get_file_id('%1', '%2')), 225);")
								.arg(session_key)
								.arg(::AlFileName)	// "$root$/Schemas/ApplicationLogic"
							 );

		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

		ok = query.exec(QString("SELECT F.Attributes FROM File F WHERE FileID = (SELECT api.get_file_id('%1', '%2'));")
								.arg(session_key)
								.arg(::AlFileName)	// "$root$/Schemas/ApplicationLogic"
							 );

		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.value(0).toInt() == 225, qPrintable("Expected new attribute value 123"));
	}

	// LogOut
	//
	bool ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
}

void FileTests::api_add_file()
{
	// LogIn as Admin
	//
	QString session_key = logIn(m_user1);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	/*
		FUNCTION api.add_file(
			session_key text,
			file_name text,
			parent_id integer,
			file_data bytea,
			details text,
			attributes integer)
		  RETURNS objectstate AS
	*/

	{
		QString fileName = "AddFileTest.txt";
		int attributes = 15;

		QSqlQuery query;
		bool ok = query.exec(QString("SELECT * FROM api.add_file('%1', '%2', (SELECT api.get_file_id('%1', '%3')), '1234567890', '{}', %4);")
								.arg(session_key)
								.arg(fileName)
								.arg(::AlFileName)	// "$root$/Schemas/ApplicationLogic"
								.arg(attributes)
							 );

		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

		ObjectState os;
		DbWorker::db_objectState(query, &os);

		int fileId = os.id;

		QVERIFY2(os.errCode == 0, qPrintable("Expected no error while creating file"));
		QVERIFY2(os.action == VcsItemAction::Added, qPrintable("Expected VcsItemAction::Added"));
		QVERIFY2(os.checkedOut == true, qPrintable("Expected CheckedOut"));
		QVERIFY2(os.deleted == false, qPrintable("Expected not deleted"));

		// Check created file info
		//
		ok = query.exec(QString("SELECT * FROM api.get_file_info('%1', '%2');")
								.arg(session_key)
								.arg(QString(::AlFileName) + "/" + fileName)	// "$root$/Schemas/ApplicationLogic/AddFileTest.txt"
							 );

		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

		DbFileInfo fi;
		bool parseOk = DbWorker::db_dbFileInfo(query, &fi);
		QVERIFY2(parseOk == true, qPrintable("Can't parse DbFileInfo"));

		QVERIFY2(fi.fileId() == fileId, qPrintable("Wrong FileID"));
		QVERIFY2(fi.fileName() == fileName, qPrintable("Wrong FileName"));
		QVERIFY2(fi.deleted() == false, qPrintable("Wrong Deleted attribute"));
		QVERIFY2(fi.attributes() == attributes, qPrintable("Wrong attributes value"));

		// Check that for created file we have one instance
		//
		ok = query.exec(QString("SELECT count(*) FROM FileInstance WHERE FileID = %1;").arg(fileId));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.value(0).toInt() == 1, qPrintable("Expected one record in FileInstance"));

		// Remove File
		//
		ok = query.exec(QString("UPDATE File SET CheckedOutInstanceId = NULL WHERE FileID = %1;").arg(fileId));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

		ok = query.exec(QString("DELETE FROM CheckOut WHERE FileID = %1;").arg(fileId));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

		ok = query.exec(QString("DELETE FROM FileInstance WHERE FileID = %1;").arg(fileId));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

		ok = query.exec(QString("DELETE FROM File WHERE FileID = %1;").arg(fileId));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	}

	// LogOut
	//
	bool ok = logOut();
	QVERIFY2(ok == true, "Log out error");
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

	return;
}

void FileTests::filesExistTest_data()
{
	QTest::addColumn<QString>("fileID");
	QTest::addColumn<QString>("result");

	QTest::newRow("existedFile") << "1" << "true";
	QTest::newRow("arrayExistingFiles") << "4,5,1" << "true,true,true";
	QTest::newRow("invalidFileTest") << "8590" << "false";
	QTest::newRow("arrayRandomFiles") << "67,99,9999" << "true,true,false";
	QTest::newRow("deletedFile") << "3" << "true";
}

void FileTests::filesExistTest()
{
	QFETCH(QString, fileID);
	QFETCH(QString, result);

	QCOMPARE(FileTests::filesExist(fileID), result);
}

void FileTests::is_any_checked_outTest()
{
	QString session_key = logIn("Administrator", m_projectAdministratorPassword);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
	QSqlQuery query;

	QString queryText = QString("SELECT * FROM api.is_any_checked_out('%1');").
							arg(session_key);

	bool ok = query.exec(queryText);

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable("Error: no return value"));
	QVERIFY2(query.value(0).toInt() == 0, qPrintable("Error: no checkedOut files expected"));

	// Add file and chech if there are any checked out files
	//
	queryText = QString("SELECT * FROM add_file(1, 'isAnyCheckedOutTest', 2, 'TEST', '{}')");

	ok = query.exec(queryText);
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	queryText = QString("SELECT * FROM api.is_any_checked_out('%1');").
					arg(session_key);

	ok = query.exec(queryText);
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable("Error: no return value"));
	QVERIFY2(query.value(0).toInt() == 1, qPrintable("Error: some checkedOuted files expected"));

	// --
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");
	return;
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
	QTest::newRow("checkInvalidParentId") << 1 << "TestInvalidParentId" << 999999 << "OneTwoThreeFour" << "{}" << false;
	QTest::newRow("createdByUser") << m_user1.userId << "TestCreatedByUser" << 2 << "OneTwoThreeFourFive" << "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000001}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}" << true;
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

	bool ok = query.exec(QString("SELECT * FROM add_file (%1, 'fileHasChildrenTestParent', 1, 'TESTING', '{}')").arg(m_user1.userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec("SELECT count(*) FROM file WHERE parentid = 1");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int countOfFileIds = query.value(0).toInt();

	// Call function by file owner
	//

	ok = query.exec(QString("SELECT file_has_children (%1,%2);")
					.arg(m_user1.userId)
					.arg(1));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("file_has_children").toInt() == countOfFileIds, qPrintable(QString("Error! Wrong result!\nExpected: %1\nActual: %2\nFirst value: %3\nSecond value: %4").arg(countOfFileIds).arg(query.value("file_has_children").toInt()).arg(m_user1.userId).arg(1)));

	// Call function by other user
	//

	ok = query.exec(QString("SELECT file_has_children (%1,%2);")
					.arg(m_user2.userId)
					.arg(1));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("file_has_children").toInt() == countOfFileIds-1, qPrintable(QString("Error! Wrong result!\nExpected: %1\nActual: %2\nFirst value: %3\nSecond value: %4").arg(countOfFileIds-1).arg(query.value("file_has_children").toInt()).arg(m_user2.userId).arg(1)));

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

	bool ok = query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestFirstFile', 1, 'TESTING', '{}')").arg(m_user1.userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int firstFileOwnerFirstUser = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestSecondFile', 1, 'TESTING', '{}')").arg(m_user1.userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int secondFileOwnerFirstUser = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestThirdFile', 1, 'TESTING', '{}')").arg(m_user1.userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int thirdFileOwnerFirstUser = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestFourthFile', 1, 'TESTING', '{}')").arg(m_user1.userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int checkedInFileOwnerFirstUser = query.value("id").toInt();

	ok = query.exec(QString("SELECT check_in(%1, '{%2}', 'test')")
					.arg(1)
					.arg(checkedInFileOwnerFirstUser));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	if (ok == false)
		qDebug() << "ERROR EXECUTTING check_in QUERY";

	QTest::newRow("deleteFileOwnerFirstUserBySecondUser") << FileTests::m_user2.userId << firstFileOwnerFirstUser << false;
	QTest::newRow("deleteFileOwnerFirstUserByFirstUser") << m_user1.userId << firstFileOwnerFirstUser << true;
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
	QString comment = "TEST";

	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	bool ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestFirstFile', 1, 'TESTING', '%2')").arg(m_user1.userId).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString firstFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestSecondFile', 1, 'TESTING', '%2')").arg(m_user1.userId).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString secondFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestThirdFile', 1, 'TESTING', '%2')").arg(m_user1.userId).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString thirdFile = query.value("id").toString();
	ok = query.exec(QString("UPDATE fileInstance SET action = 3 WHERE fileId = %1").arg(thirdFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestFourthFile', 1, 'TESTING', '%2')").arg(m_user2.userId).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString fourthFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestFifthFile', 1, 'TESTING', '%2')").arg(m_user1.userId).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString fifthFile = query.value("id").toString();
	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', '%3');").arg(m_user1.userId).arg(fifthFile).arg(comment));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));


	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', '%3');").arg(m_user1.userId).arg(QString(firstFile + ", " + secondFile + ", " + thirdFile)).arg(comment));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next())
	{
		QVERIFY2(query.value("errCode").toInt() == 0, qPrintable(QString("Error code %1").arg(query.value("errCode").toInt())));
		int id = query.value("id").toInt();

		QSqlQuery tempQuery;

		ok = tempQuery.exec(QString("SELECT COUNT(*) FROM checkout WHERE fileId = %1").arg(id));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.value(0) == 0, qPrintable(QString("FILEID %1 hasn't been checked in!").arg(id)));

		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(id));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.value("checkedOutInstanceId") == "", qPrintable(QString("File %1: checkedOutInstanceId is not empty after checkIn").arg(id)));

		QString Uuid = tempQuery.value("checkedInInstanceId").toString();

		ok = tempQuery.exec(QString("SELECT * FROM fileinstance WHERE fileId = %1").arg(id));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.value("fileInstanceId").toString() == Uuid, qPrintable(QString("Error: wrong Uuid at file %1!").arg(id)));
		QVERIFY2(tempQuery.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(tempQuery.value("details").toString()).arg(detail)));

		int changeSetId = tempQuery.value("changeSetId").toInt();
		int action = query.value("action").toInt();

		ok = tempQuery.exec(QString("SELECT COUNT(*) FROM changeSet WHERE changeSetId = %1 AND userId = %2 AND comment = '%3'").arg(changeSetId).arg(m_user1.userId).arg(comment));

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

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', '%3');").arg(m_user1.userId).arg(fourthFile).arg("TEST"));
	QVERIFY2(ok == false, qPrintable("User has no rights to checkin file checkouted by another user error expected"));

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', '%3');").arg(m_user1.userId).arg(fourthFile).arg("TEST"));
	QVERIFY2(ok == false, qPrintable("Can not checkin checkinned file error expected"));

	// Let's check in unchanged file
	//

	ok = query.exec(QString("SELECT max(changesetId) FROM FileInstance WHERE fileId = %1").arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fifthFileLastChangeset = query.value(0).toInt();

	ok = query.exec(QString("SELECT * FROM check_out(%1, '{%2}')").arg(m_user1.userId).arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	// Check In withoutChanges
	//

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'checkInFileWithoutChanges')").arg(m_user1.userId).arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT max(changesetId) FROM FileInstance WHERE fileId = %1").arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(fifthFileLastChangeset == query.value(0).toInt(), qPrintable("Error: checked in file without changes"));

	ok = query.exec(QString("SELECT comment FROM changeset WHERE changesetId = %1").arg(fifthFileLastChangeset));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(comment == query.value(0).toString(), qPrintable("Error: comment from unchecked In file has been written to changeset table"));

	ok = query.exec(QString("SELECT * FROM is_file_checkedout(%1)").arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: file was not checked in"));

	// change Action row, do not change Data row
	//

	ok = query.exec(QString("SELECT * FROM check_out(%1, '{%2}')").arg(m_user1.userId).arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("UPDATE fileInstance SET Action=2 WHERE fileId = %1 AND changesetId = %2").arg(fifthFile).arg(fifthFileLastChangeset));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'checkInFileWithoutChanges')").arg(m_user1.userId).arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT max(changesetId) FROM FileInstance WHERE fileId = %1").arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(fifthFileLastChangeset == query.value(0).toInt(), qPrintable("Error: file was checked in without data row changed"));

	ok = query.exec(QString("SELECT * FROM is_file_checkedout(%1)").arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: file was not checked in"));

	// change Action and Data row
	//

	ok = query.exec(QString("SELECT * FROM check_out(%1, '{%2}')").arg(m_user1.userId).arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("UPDATE fileInstance SET Action=2 WHERE fileId = %1 AND changesetId = %2").arg(fifthFile).arg(fifthFileLastChangeset));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("UPDATE fileInstance SET Data='12314124125343gdfjtjfgx bvavt23y45' WHERE fileId = %1 AND changesetId = %2").arg(fifthFile).arg(fifthFileLastChangeset));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'checkInFileWithoutChanges')").arg(m_user1.userId).arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT max(changesetId) FROM FileInstance WHERE fileId = %1").arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(fifthFileLastChangeset != query.value(0).toInt(), qPrintable("Error: file was not checked in (file was fully updated)"));

	ok = query.exec(QString("SELECT * FROM is_file_checkedout(%1)").arg(fifthFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: file was not checked in"));
}

void FileTests::check_outTest()
{
	QSqlQuery query;

	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	bool ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkOutFileTestFirstFile', 1, 'TESTING', '%2')").arg(m_user1.userId).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString firstFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'TEST');").arg(m_user1.userId).arg(firstFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkOutFileTestSecondFile', 1, 'TESTING', '%2')").arg(m_user1.userId).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString secondFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'TEST');").arg(m_user1.userId).arg(secondFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkOutFileTestThirdFile', 1, 'TESTING', '%2')").arg(m_user1.userId).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString thirdFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM check_out(%1, '{%2}');").arg(m_user1.userId).arg(QString(firstFile + ", " + secondFile)));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next())
	{
		int id = query.value("id").toInt();
		QVERIFY2(query.value("errCode").toInt() == 0, qPrintable(QString("Error code %1 AT FILEID %2").arg(query.value("errCode").toInt()).arg(id)));

		QSqlQuery tempQuery;

		ok = tempQuery.exec(QString("SELECT COUNT(*) FROM checkOut WHERE fileId = %1 AND userId = %2").arg(id).arg(m_user1.userId));

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
	// Scenario:
	// 1. LogIn as User1
	// 2. Add File
	// 3. Set workcopy
	// 4. Check FileInstance
	// 5. CheckIn File to check if possible to set workcopy for CheckedIn file
	// 6. Set workcopy for checked in file (error expexted)
	// 7. Add another file to check if it possible to set workcpy by other (non admin) user
	// 8. LogOut User1
	// --
	// 9. LogIn User2
	// 10. Set workcopy (User2) for file checked out by User1 (expected error)
	// 11. LogOut User2
	// --
	// 12. LogIn as Administrtor
	// 13. Set workcopy for file checked out by User1 (expected success)
	// 14. LogOut Administrtor
	//

	// 1. LogIn as User1
	//
	QString session_key = logIn(m_user1);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// 2. Add File
	//
	QSqlQuery query;
	QString data = "TEST";
	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'setWorkcopyTestFirstFile', 1, 'TESTING', '{}')").arg(m_user1.userId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QString firstFile = query.value("id").toString();

	// 3. Set workcopy
	//
	ok = query.exec(QString("SELECT * FROM api.set_workcopy('%1', %2, '%3', '%4');").arg(session_key).arg(firstFile).arg(data).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT checkedOutInstanceId FROM file WHERE FileID = %1").arg(firstFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QString Uuid = query.value(0).toString();

	// 4. Check FileInstance
	//
	ok = query.exec(QString("SELECT * FROM FileInstance WHERE fileInstanceId = '%1'").arg(Uuid));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("data").toString() == data, qPrintable(QString("Column data has not been changed in table fileInstance with fileID %1").arg(firstFile)));
	QVERIFY2(query.value("size").toInt() == data.length(), qPrintable(QString("Column size has not been changed in table fileInstance with fileID %1").arg(firstFile)));
	QVERIFY2(query.value("FileID").toString() == firstFile, qPrintable(QString("Column fileId has not been changed in table fileInstance with checkOutInstanceId %1").arg(Uuid)));
	QVERIFY2(query.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(query.value("details").toString()).arg(detail)));

	// 5. CheckIn File to check if possible to set workcopy for CheckedIn file
	//
	ok = query.exec(QString("SELECT * FROM public.check_in(%1, ARRAY[%2], 'CheckIn Comment');")
						.arg(m_user1.userId)
						.arg(firstFile)
					);
	QVERIFY2(ok == true, "check_in error");

	// 6. Set workcopy for checked in file (error expexted)
	//
	ok = query.exec(QString("SELECT * FROM api.set_workcopy('%1', %2, '%3', '%4');").arg(session_key).arg(firstFile).arg(data).arg(detail));
	QVERIFY2(ok == false, "set_worcopy error expected, as file is checked in.");

	// 7. Add another file to check if it possible to set workcpy by other (non admin) user
	//
	ok = query.exec(QString("SELECT * FROM add_file(%1, 'setWorkcopyTestSecondFile', 1, 'TESTING', '{}')").arg(m_user1.userId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QString secondFile = query.value("id").toString();

	// 8. LogOut User1
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");


	// --
	// 9. LogIn User2
	//
	session_key = logIn(m_user2);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// 10. Set workcopy (User2) for file checked out by User1 (expected error)
	//
	ok = query.exec(QString("SELECT * FROM api.set_workcopy('%1', %2, '%3', '%4');").arg(session_key).arg(secondFile).arg(data).arg(detail));
	QVERIFY2(ok == false, "set_workcopy expected error, as file checked out by another user");

	// 11. LogOutUser2
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	// --
	// 12. LogIn as Administrtor
	//
	session_key = logIn("Administrator", m_projectAdministratorPassword);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// 13. Set workcopy for file checked out by User1 (expected success)
	//
	ok = query.exec(QString("SELECT * FROM api.set_workcopy('%1', %2, '%3', '%4');").arg(session_key).arg(secondFile).arg(data).arg(detail));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// 14. LogOutAdministrtor
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
}

void FileTests::get_workcopyTest()
{
	// Scenario
	// 1. LogIn as User1
	// 2. Add File
	// 3. Get workcopy
	// 4. Check table CheckOut
	// 5. Check table File
	// 6. CheckTable FileInstance
	// 7. LogOut

	// 1. LogIn as User1
	//
	QString session_key = logIn(m_user1);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// 2. Add File
	//
	QSqlQuery query;
	QSqlQuery tempQuery;
	QString data = "TEST";
	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'getWorkcopyTestFirstFile', 1, '%2', '%3')").arg(m_user1.userId).arg(data).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QString firstFile = query.value("id").toString();

	// 3. Get workcopy
	//
	ok = query.exec(QString("SELECT * FROM api.get_workcopy('%1', %2);").arg(session_key).arg(firstFile));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	// 4. Check table CheckOut
	//
	ok = tempQuery.exec(QString("SELECT * FROM CheckOut WHERE FileId = %1").arg(query.value("fileId").toInt()));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value("userId").toInt() == query.value("userId").toInt(), qPrintable(QString("Error: in column \"userId\" - data mistmatch")));
	QVERIFY2(query.value("checkedOut").toBool() == true, qPrintable(QString("Error: in column \"checkOut\" - data mistmatch")));

	// 5. Check table File
	//
	ok = tempQuery.exec(QString("SELECT * FROM File WHERE FileID = %1").arg(query.value("fileId").toInt()));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value("name").toString() == query.value("name").toString(), qPrintable("Error: in column \"name\" - data mistmatch"));
	QVERIFY2(tempQuery.value("parentId").toInt() == query.value("parentId").toInt(), qPrintable("Error: in column \"parentId\" - data mistmatch"));

	// 6. CheckTable FileInstance
	//
	ok = tempQuery.exec(QString("SELECT * FROM FileInstance WHERE FileID = %1").arg(query.value("fileId").toInt()));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("Error: in column \"size\" - data mistmatch")));
	QVERIFY2(data == query.value("data").toString(), qPrintable(QString("Error: in column \"data\" - data mistmatch")));
	QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("Error: in column \"action\" - data mistmatch")));
	QVERIFY2(query.value("details").toString() == detail, qPrintable(QString("Error: details not match! \nActual: %1\nExpected: %2").arg(query.value("details").toString()).arg(detail)));
	QVERIFY2(query.value("changeSetId").toInt() == tempQuery.value("changeSetId").toInt(), qPrintable("Error: in column \"changeSetId\" - data mistmatch"));

	// 7. LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
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

	ok = query.exec(QString("SELECT * FROM check_out (%1, '{%2}');").arg(m_user1.userId).arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT MAX(changesetId) FROM FileInstance WHERE fileID = %1").arg(fileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int changesetIdForDataUpdate = query.value(0).toInt();

	ok = query.exec(QString("UPDATE FileInstance SET Data = 'testtesttest' WHERE changesetId = %1").arg(changesetIdForDataUpdate));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'blah-blah');").arg(m_user1.userId).arg(fileId));

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

	ok = query.exec(QString("SELECT * FROM get_file_history(%1, %2)").arg(m_user1.userId).arg(fileId));
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

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'getFileHistoryTestFailFile', 1, 'TEST', '{}')").arg(m_user1.userId));

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

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'getFileStateTestFirst', 1, 'TEST', '{}')").arg(m_user1.userId));

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
	QString session_key = logIn(m_user1);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'getFileIdTest', 7, 'foxes everywhere', '{}');").arg(m_user1.userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM api.get_file_id('%1', '///$root$/MC/getFileIdTest///');").arg(session_key));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(fileId == query.value(0).toInt(), qPrintable("Error: fileId not match!"));

	ok = query.exec(QString("SELECT * FROM api.get_file_id('%1', '///$root$/MC/getFileIdTestAleluyz/');").arg(session_key));
	QVERIFY(ok == false);

	// 7. LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
}

void FileTests::get_file_IdIntegerIntegerTextTest()
{
	// 1. LogIn as User1
	//
	QString session_key = logIn(m_user1);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'getFileIdTest1', 7, 'foxes everywhere', '{}');").arg(m_user1.userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM get_file_id(%1, 7, 'getFileIdTest1');").arg(m_user1.userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(fileId == query.value(0).toInt(), qPrintable("Error: fileId not match!"));

	ok = query.exec("SELECT * FROM get_file_id(1, 7, 'getFileIdTest1');");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(fileId == query.value(0).toInt(), qPrintable("Error: fileId not match!"));

	ok = query.exec(QString("SELECT * FROM get_file_id(%1, 'getFileIdTest1');").arg(m_user2.userId));
	QVERIFY2(ok == false, qPrintable("Wrong user error expected"));

	ok = query.exec("SELECT * FROM get_file_id(1, 7, 'abcdef');");
	QVERIFY2(ok == false, qPrintable("NULL fileId error expected"));

	ok = query.exec("SELECT * FROM get_file_id(1, 0, 'getFileIdTest1');");
	QVERIFY2(ok == false, qPrintable("NULL fileId error expected"));

	ok = query.exec(QString("SELECT * FROM api.add_or_update_file('%1', '$root$/MC/', 'getFileIdTest1', 'simple comment', 'foxes everywhere', '{}');").arg(session_key));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_file_id(%1, 7, 'getFileIdTest1');").arg(m_user2.userId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(fileId == query.value(0).toInt(), qPrintable("Error: other user must has acsess to file after update"));

	// 7. LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");
}

void FileTests::get_file_infoTest()
{
	// 1. LogIn as User1
	//
	QString session_key = logIn("Administrator", m_projectAdministratorPassword);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
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

	ok = query.exec(QString("SELECT * FROM api.get_file_info('%1', ARRAY[%2, %3]) ORDER BY fileId;")
						.arg(session_key)
						.arg(checkedOutFileId)
						.arg(checkedInFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next())
	{
		ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(fileIds[fileIdNumber]));

		QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
		QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

		int fileId = tempQuery.value("fileId").toInt();
		bool deleted = tempQuery.value("deleted").toBool();
		QString name = tempQuery.value("name").toString();
		int parentId = tempQuery.value("parentId").toInt();

		QVERIFY2(fileId == query.value("fileId").toInt(), qPrintable(QString("Error: fileId not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(deleted == query.value("deleted").toBool(), qPrintable(QString("Error: deleted not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(name == query.value("name").toString(), qPrintable(QString("Error: string not match in fileId %1").arg(fileIds[fileIdNumber])));
		QVERIFY2(parentId == query.value("parentId").toInt(), qPrintable(QString("Error: parentId not match in fileId %1").arg(fileIds[fileIdNumber])));

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

	ok = query.exec(QString("SELECT * FROM api.get_file_info('%1', ARRAY[%2])")
						.arg(session_key)
						.arg(maxValueId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Empty string expected (invalid fileId)"));

	// 7. LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");
}

void FileTests::get_latest_file_versionTest()
{
	// 1. LogIn as User1
	//
	QString session_key = logIn(m_user1);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
	QSqlQuery query;
	QSqlQuery tempQuery;
	QString detail = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";

	// Create file and check in it to test
	//

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'get_latest_file_versionTestCheckedOutFile', 1, 'Hello TUX!', '%2');").arg(m_user1.userId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int checkedInFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'TUX is busy :(');").arg(m_user1.userId).arg(checkedInFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'get_latest_file_versionTestCheckedInFile', 1, 'Hello TUX!', '%2');").arg(m_user1.userId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int checkedOutFileId = query.value("id").toInt();

	int fileIds[2] = {checkedOutFileId, checkedInFileId};

	// Testing checked in file
	//
	for (int NumberOfFileId = 0; NumberOfFileId < 2; NumberOfFileId++)
	{

		ok = query.exec(QString("SElECT * FROM api.get_latest_file_version('%1', %2);").arg(session_key).arg(fileIds[NumberOfFileId]));

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
	ok = query.exec(QString("SElECT * FROM api.get_latest_file_version('%1', %2);")
						.arg(session_key)
						.arg(maxValueId)
					);

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Empty row expected"));

	// 7. LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
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

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'get_file_listFirst', 1, 'Mandriva', '%2');").arg(m_user1.userId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int firstFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'get_file_listSecond', 1, 'Slackware', '%2');").arg(m_user1.userId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int secondFileId = query.value("id").toInt();

	// Check in one of files
	//

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_user1.userId).arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'get_file_listThird', 1, 'FreeBSD', '%2');").arg(m_user1.userId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int thirdFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_user1.userId).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_file(%1, %2);").arg(m_user1.userId).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_user1.userId).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'get_file_listFourth', 0, 'FreeBSD', '%2');").arg(m_user1.userId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fourthFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_user1.userId).arg(fourthFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'letsBreakFunctionByCreatingWrongNames', 1, 'Gentoo', '%2');").arg(m_user1.userId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fifthFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_user1.userId).arg(fifthFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_file_list(%1, 1, 'get_file_list%') ORDER BY fileId").arg(m_user1.userId));

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

	ok = query.exec(QString("SELECT * FROM check_out(%1, '{%2}');").arg(m_user1.userId).arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SElECT * FROM delete_file(%1, %2);").arg(m_user1.userId).arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Try execute function by another user
	//

	ok = query.exec(QString("SELECT * FROM get_file_list(%1, 1, 'get_file_list%') ORDER BY fileId").arg(m_user2.userId));

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


	ok = query.exec(QString("SELECT * FROM get_file_list(%1, 2, 'get_file_list%') ORDER BY fileId").arg(m_user1.userId));

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
	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'randomParentNameForGetFileListTest', 1, 'FatherGentoo', '%2');").arg(m_user1.userId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int parentFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'randomNameForGetFileListTest', %2, 'Mandriva', '%3');").arg(m_user1.userId).arg(parentFileId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int firstFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'randomNameForGetFileListTest1', %2, 'Slackware', '%3');").arg(m_user1.userId).arg(parentFileId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int secondFileId = query.value("id").toInt();

	// Check in one of files
	//

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_user1.userId).arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'randomNameForGetFileListTest2', %2, 'FreeBSD', '%3');").arg(m_user1.userId).arg(parentFileId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int thirdFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_user1.userId).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_file(%1, %2);").arg(m_user1.userId).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_user1.userId).arg(thirdFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'randomNameForGetFileListTest3', 0, 'FreeBSD', '%2');").arg(m_user1.userId).arg(detail));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fourthFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'Suddenly debian');").arg(m_user1.userId).arg(fourthFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_file_list(%1, %2) ORDER BY fileId").arg(m_user1.userId).arg(parentFileId));

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

	ok = query.exec(QString("SELECT * FROM check_out(%1, '{%2}');").arg(m_user1.userId).arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SElECT * FROM delete_file(%1, %2);").arg(m_user1.userId).arg(secondFileId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Try execute function by another user
	//

	ok = query.exec(QString("SELECT * FROM get_file_list(%1, %2) ORDER BY fileId").arg(m_user2.userId).arg(parentFileId));

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

	ok = query.exec(QString("SELECT * FROM get_file_list(%1, %2) ORDER BY fileId").arg(m_user1.userId).arg(parentFileId + 1));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Empty row expected"));
}

void FileTests::get_file_list_tree_test()
{
	// 1. LogIn as User1
	//
	QString session_key = logIn("Administrator", m_projectAdministratorPassword);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	bool ok = true;

	// 2. Create file tree:
	//
	// $root$/TestTreeRoot.ttr/
	//                        File1.ttr
	//                        File2.ttr/
	//                                  File21.ttr
	//                                  File22.asd
	//                                  File23.ttr
	//                       /File3.asd
	//                                  File31.ttr
	//                                  File32.asd
	//
	QSqlQuery query;

	// first - parent file name, second file name
	//
	std::list<std::pair<QString, QString>> createFiles =
		{
			{"$root$",								"TestTreeRoot.ttr"},

			{"$root$/TestTreeRoot.ttr",				"File1.ttr"},

			{"$root$/TestTreeRoot.ttr",				"File2.ttr"},	// File2.ttr -> Deleted and not checked in, so shoud remain in result with all childer
			{"$root$/TestTreeRoot.ttr/File2.ttr",	"File21.ttr"},
			{"$root$/TestTreeRoot.ttr/File2.ttr",	"File22.asd"},
			{"$root$/TestTreeRoot.ttr/File2.ttr",	"File23.ttr"},

			{"$root$/TestTreeRoot.ttr",				"File3.asd"},
			{"$root$/TestTreeRoot.ttr/File3.asd",	"File31.ttr"},
			{"$root$/TestTreeRoot.ttr/File3.asd",	"File32.asd"},

			{"$root$/TestTreeRoot.ttr",				"File4.ttr"},	// Deleted and checked in, it will not be in result if RemoveFromDeleted
			{"$root$/TestTreeRoot.ttr/File4.ttr",	"File41.ttr"},	// Parent deleted and checked in, it will not be in result if RemoveFromDeleted
			{"$root$/TestTreeRoot.ttr/File4.ttr",	"File42.ttr"}	// Parent deleted and checked in, it will not be in result if RemoveFromDeleted
		};

	std::set<int> fileIds;
	int fileId_File2_ttr = -1;
	int fileId_File4_ttr = -1;

	for (auto[parentFileName, fileName] : createFiles)
	{
		QString request = QString("SELECT * FROM api.add_or_update_file('%1', '%2', '%3', 'Test function get_file_list_tree_test', '', '{}');")
						  .arg(session_key)
						  .arg(parentFileName)
						  .arg(fileName);

		ok = query.exec(request);
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.size() == 1, "api.add_or_update_file, expected 1 record result");

		query.next();
		int fileId = query.value(0).toInt();

		fileIds.insert(fileId);

		// --
		//
		if (fileName == "File2.ttr")
		{
			fileId_File2_ttr = fileId;
		}

		if (fileName == "File4.ttr")
		{
			fileId_File4_ttr = fileId;
		}
	}

	// Delete file: File2.ttr -> Deleted and NOT checked in, so shoud remain in result with all childer
	//
	ok = query.exec(QString("SELECT * FROM public.delete_file(1, %1);").arg(fileId_File2_ttr));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Delete file: File4.ttr -> Deleted and check in
	//
	ok = query.exec(QString("SELECT * FROM public.delete_file(1, %1);").arg(fileId_File4_ttr));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM public.check_in(1, ARRAY[%1], 'Delete fileId_File4_ttr');").arg(fileId_File4_ttr));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// 3. Test 1, get files tree, filter by 'ttr', don't get deleted files
	//
	int parentId = *fileIds.begin();

	ok = query.exec(QString("SELECT * FROM api.get_file_list_tree('%1', %2, '%ttr', true);").arg(session_key).arg(parentId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.numRowsAffected() == 5, "Expected 5 files, as filetre for %%ttr is applied");

	while (query.next())
	{
		int fileId = query.value(0).toInt();
		QVERIFY2(fileIds.count(fileId) == 1, "Not all files are found.");
		fileIds.erase(fileId);	// Remove this file to avoid comparision to it again
	}

	// 7. LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
}

void FileTests::get_latest_file_tree_versionTest()
{
	// 1. LogIn as Admin
	//
	QString session_key = logIn(m_user1);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
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

	ok = query.exec(QString("SElECT * FROM api.get_latest_file_tree_version('%1', %2);")
						.arg(session_key)
						.arg(firstFileId)
					);

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

	ok = query.exec(QString("SElECT * FROM api.get_latest_file_tree_version('%1', %2);")
						.arg(session_key)
						.arg(maxValueId)
					);

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Invalid fileId error expected"));

	ok = query.exec(QString("SELECT * FROM api.get_latest_file_tree_version('%1', %2);")
						.arg(session_key)
						.arg(firstFileId)
					);

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Invalid user error expected"));


	// 7. LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
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

	ok = query.exec(QString("SELECT * FROM undo_changes(%1, '{%2}');").arg(m_user1.userId).arg(errorCallFileId));

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
	// 1. LogIn as Admin
	//
	QString session_key = logIn("Administrator", m_projectAdministratorPassword);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
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

	ok = query.exec(QString("SELECT * FROM api.add_or_update_file('%1', '$root$/MC/', 'addOrUpdateFileTest', '%2', '%3', '%5');")
						.arg(session_key)
						.arg(comment)
						.arg(fileData)
						.arg(detail));

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
	ok = query.exec(QString("SELECT * FROM api.add_or_update_file('%1', '$root$/MC/', 'addOrUpdateCreateFileTest', '%2', '%3', '%4');")
						.arg(session_key)
						.arg(comment)
						.arg(fileData)
						.arg(detail));

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

//	ok = query.exec(QString("SELECT * FROM api.add_or_update_file(%1, '$root$/MC/', 'addOrUpdateErrorUserTest', 'deleted', 'deleted', '{}');").arg(m_user1.userId));

//	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

//	ok = query.exec(QString("SELECT * FROM file WHERE fileId = %1 AND deleted = false").arg(fileId+1));

//	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
//	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	// 7. LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
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
	// 1. LogIn as Admin
	//
	QString session_key = logIn("Administrator", m_projectAdministratorPassword);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
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

	ok = query.exec(QString("SELECT * FROM api.get_checked_out_files('%1', ARRAY[%2, %3]) ORDER BY fileId")
						.arg(session_key)
						.arg(fileIds[0])
						.arg(fileIds[4]));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int numberOfFileId = 0;

	while (query.next())
	{
		QVERIFY2(query.value("fileId").toInt() != fileIds[6], qPrintable ("Error: function show checkedIn files"));

		QVERIFY2(query.value("fileId").toInt() == fileIds[numberOfFileId], qPrintable(QString("Error: fileId's mistmatch. Unexpected fileId's:\nActual: %1\nExpected: %2").arg(query.value("fileId").toInt()).arg(fileIds[numberOfFileId])));
		numberOfFileId++;
	}

	QVERIFY2(numberOfFileId == 6, qPrintable("Wrong count of records in result"));

	// 7. LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");
}

void FileTests::check_in_treeTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	int fileIds[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	int changesetIdUnchangedFile = -1; // File was checked in, checked out, and check_in_tree without changes
	int changesetIdChangedFile = -1; // File was checked in, checked out, and check_in_tree with changes

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

	// Create file. Check it in, check out, and do checkInTree without changes
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'checkInTreeTestUnchangedFile', %1, 'GNU/Hurd', '{}');").arg(fileIds[0]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[7] = query.value("id").toInt();

	// Create file. Check it in, check it out, make some changes, checkInTree.
	//

	ok = query.exec(QString("SElECT * FROM add_file(1, 'checkInTreeTestChangedFile', %1, 'GNU/Linux', '{}');").arg(fileIds[0]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	fileIds[8] = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM check_in (1, '{%1, %2, %3, %4}', 'TEST');").arg(fileIds[6]).arg(fileIds[5]).arg(fileIds[7]).arg(fileIds[8]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT MAX(changesetId) FROM fileInstance WHERE fileId = %1").arg(fileIds[7]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	changesetIdUnchangedFile = query.value(0).toInt();

	ok = query.exec(QString("SELECT MAX(changesetId) FROM fileInstance WHERE fileId = %1").arg(fileIds[8]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	changesetIdChangedFile = query.value(0).toInt();

	ok = query.exec(QString("SELECT * FROM check_out(1, '{%1, %2, %3}');").arg(fileIds[5]).arg(fileIds[7]).arg(fileIds[8]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("UPDATE fileInstance SET Action=2 WHERE fileId = %1 AND changesetId = %2").arg(fileIds[8]).arg(changesetIdChangedFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("UPDATE fileInstance SET Data='12314124125343gdfjtjfgx bvavt23y45' WHERE fileId = %1 AND changesetId = %2").arg(fileIds[8]).arg(changesetIdChangedFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM delete_file (1, %1);").arg(fileIds[5]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_in_tree(1, '{%1, %2}', 'TEST') ORDER BY id").arg(fileIds[0]).arg(fileIds[4]));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int fileNumber = 0;

	while (query.next())
	{
		// 6th file in array - file, which already checked in. Function must not return 6th file id, and
		// need to skip it
		//

		QVERIFY2(query.value("id").toInt() != fileIds[6], qPrintable("Error: checked in file has been checked_in twice"));

		if (fileNumber == 6)
		{
			fileNumber++;
		}

		int currentFileId = query.value("id").toInt();

		if (currentFileId == fileIds[5])
		{
			// Check deleted file
			//

			ok = tempQuery.exec(QString("SELECT deleted FROM file WHERE fileId = %1").arg(currentFileId));

			QVERIFY2 (ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2 (tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2 (tempQuery.value(0).toBool() == true, qPrintable ("File has not been deleted"));
		}
		else if (currentFileId == fileIds[7])
		{
			ok = tempQuery.exec(QString("SELECT max(changesetId) FROM fileInstance WHERE fileId = %1").arg(currentFileId));

			QVERIFY2 (ok == true, qPrintable(tempQuery.lastError().databaseText()));
			QVERIFY2 (tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

			QVERIFY2 (tempQuery.value(0).toInt() == changesetIdUnchangedFile, qPrintable("Error: file without changes has been checked in"));
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

			if (currentFileId == fileIds[8])
			{
				ok = tempQuery.exec(QString("SELECT max(changesetId) FROM fileInstance WHERE fileId = %1").arg(currentFileId));

				QVERIFY2 (ok == true, qPrintable(tempQuery.lastError().databaseText()));
				QVERIFY2 (tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

				QVERIFY2 (tempQuery.value(0).toInt() != changesetIdUnchangedFile, qPrintable("Error: file with changes was not checkedIn"));
			}
		}

		fileNumber++;
	}

	// Check amount of results
	//

	QVERIFY2 (fileNumber == 9, qPrintable ("Error: wrong file amount"));

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
