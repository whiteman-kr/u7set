#include <QtSql>
#include <QString>
#include <QTest>
#include "FileTests.h"

void FileTests::getObjectState(QSqlQuery& q, ObjectState& os)
{
	os.newFileId = q.value(0).toInt();
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

	//files_exist & file_exist
	bool ok = query.exec("UPDATE file SET Deleted = true WHERE fileid = 3");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	//Testers
	ok = query.exec("SElECT create_user(1, 'FIRSTTEST', 'FIRSTTEST', 'FIRSTTEST', '12341234', false, false, false);");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	FileTests::m_firstUserForTest = query.value("create_user").toInt();

	ok = query.exec("SElECT create_user(1, 'SECONDTEST', 'SECONDTEST', 'SECONDTEST', '12341234', false, false, false);");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	FileTests::m_secondUserForTest = query.value("create_user").toInt();
}

void FileTests::cleanupTestCase()
{
}

void FileTests::fileExistsTest_data()
{
	QTest::addColumn<int>("fileID");
	QTest::addColumn<bool>("result");

	QTest::newRow("existedFileTest") << 1 << true;
	QTest::newRow("nullFileTest") << 9999 << false;
	QTest::newRow("deletedFileTest") << 3 << false;
}

void FileTests::fileExistsTest()
{
	QFETCH(int, fileID);
	QFETCH(bool, result);

	QCOMPARE(FileTests::fileExists(fileID), result);
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
	QCOMPARE (FileTests::is_any_checked_out(), false);

	QSqlQuery query;
	bool ok = query.exec("SELECT * FROM add_file (1, 'isAnyCheckedOutTest', 2, 'TEST')");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	QCOMPARE (FileTests::is_any_checked_out(), true);
}

void FileTests::filesAddTest_data()
{
	QTest::addColumn<int>("user_id");
	QTest::addColumn<QString>("file_name");
	QTest::addColumn<int>("parent_id");
	QTest::addColumn<QString>("file_data");
	QTest::addColumn<bool>("result");

	QTest::newRow("checkingRandomFileRandomData") << 1 << "Test" << 2 << "Test" << true;
	QTest::newRow("existingFile") << 1 << "Test" << 2 << "Test" << false;
	QTest::newRow("checkInvalidUserId") << 999 << "TestInvalidUserId" << 2 << "OneTwoThree" << false;
	QTest::newRow("checkInvalidParentId") << 1 << "TestInvalidParentId" << 999 << "OneTwoThreeFour" << false;
	QTest::newRow("createdByUser") << FileTests::m_firstUserForTest << "TestCreatedByUser" << 2 << "OneTwoThreeFourFive" << true;
}

void FileTests::filesAddTest()
{
	QFETCH(int, user_id);
	QFETCH(QString, file_name);
	QFETCH(int, parent_id);
	QFETCH(QString, file_data);
	QFETCH(bool, result);

	QCOMPARE(FileTests::add_file(user_id, file_name, parent_id, file_data), result);
}

void FileTests::is_file_checkedoutTest_data()
{
	QTest::addColumn<int>("fileId");
	QTest::addColumn<bool>("result");

	QSqlQuery query;
	bool ok = query.exec("SELECT * FROM add_file (1, 'isFileCheckedOutTest', 2, 'TESTING')");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QTest::newRow("fileExistTest") << query.value(0).toInt() << true;
	QTest::newRow("nullFileTest") << 1 << false;
}

void FileTests::is_file_checkedoutTest()
{
	QFETCH (int, fileId);
	QFETCH (bool, result);

	QCOMPARE (FileTests::is_file_checkedout(fileId), result);
}

void FileTests::file_has_childrenTest_data()
{
	QTest::addColumn<int>("userId");
	QTest::addColumn<int>("fileId");
	QTest::addColumn<int>("result");

	QSqlQuery query;

	bool ok = query.exec(QString("SELECT * FROM add_file (%1, 'fileHasChildrenTestParent', 1, 'TESTING')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec("SELECT count(*) FROM file WHERE parentid = 1");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QTest::newRow("CreatedByFirstCalledByFirstUser") << FileTests::m_firstUserForTest << 1 << query.value(0).toInt();
	QTest::newRow("CreatedByFirstUserCalledBySecondUser") << FileTests::m_secondUserForTest << 1 << query.value(0).toInt()-1;
	QTest::newRow("CreatedByFirstUserCalledByAdmin") << 1 << 1 << query.value(0).toInt();
	QTest::newRow("InvalidUserFile") << 999 << 1 << -1; //???
	QTest::newRow("AdminInvalidFile") << 1 << 999 << 0;
}

void FileTests::file_has_childrenTest()
{
	QFETCH (int, userId);
	QFETCH (int, fileId);
	QFETCH (int, result);

	QCOMPARE (FileTests::file_has_children(userId, fileId), result);
}

void FileTests::delete_fileTest_data()
{
	QTest::addColumn<int>("userId");
	QTest::addColumn<int>("fileId");
	QTest::addColumn<bool>("result");

	QSqlQuery query;

	bool ok = query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestFirstFile', 1, 'TESTING')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int firstFileOwnerFirstUser = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestSecondFile', 1, 'TESTING')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int secondFileOwnerFirstUser = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestThirdFile', 1, 'TESTING')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int thirdFileOwnerFirstUser = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestFourthFile', 1, 'TESTING')").arg(FileTests::m_firstUserForTest));
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

	bool ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestFirstFile', 1, 'TESTING')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString firstFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestSecondFile', 1, 'TESTING')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString secondFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestThirdFile', 1, 'TESTING')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString thirdFile = query.value("id").toString();
	ok = query.exec(QString("UPDATE fileInstance SET action = 3 WHERE fileId = %1").arg(thirdFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestFourthFile', 1, 'TESTING')").arg(FileTests::m_secondUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString fourthFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkInFileTestFifthFile', 1, 'TESTING')").arg(FileTests::m_firstUserForTest));
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

	bool ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkOutFileTestFirstFile', 1, 'TESTING')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString firstFile = query.value("id").toString();
	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'TEST');").arg(m_firstUserForTest).arg(firstFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkOutFileTestSecondFile', 1, 'TESTING')").arg(FileTests::m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString secondFile = query.value("id").toString();
	ok = query.exec(QString("SELECT * FROM check_in(%1, '{%2}', 'TEST');").arg(m_firstUserForTest).arg(secondFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file (%1, 'checkOutFileTestThirdFile', 1, 'TESTING')").arg(FileTests::m_firstUserForTest));
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
	}

	ok = query.exec(QString("SELECT * FROM check_out(%1, '{%2}');").arg(m_firstUserForTest).arg(thirdFile));

	QVERIFY2(ok == false, "Already checked file error expected");
}

void FileTests::set_workcopyTest()
{
	QSqlQuery query;
	QString data = "TEST";


	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'setWorkcopyTestFirstFile', 1, 'TESTING')").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString firstFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM add_file(%1, 'setWorkcopyTestSecondFile', 1, 'TESTING')").arg(m_secondUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString secondFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM set_workcopy(%1, %2, '%3');").arg(m_firstUserForTest).arg(firstFile).arg(data));
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

	ok = query.exec(QString("SELECT * FROM set_workcopy(%1, %2, '%3');").arg(m_firstUserForTest).arg(secondFile).arg(data));
	QVERIFY2(ok == false, qPrintable("User is not allowed set workcopy error expected"));
}

void FileTests::get_workcopyTest()
{
	QSqlQuery query;

	QString data = "TEST";
	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'getWorkcopyTestFirstFile', 1, '%2')").arg(m_firstUserForTest).arg(data));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QString firstFile = query.value("id").toString();

	ok = query.exec(QString("SELECT * FROM get_workcopy(%1, %2);").arg(m_firstUserForTest).arg(firstFile));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QSqlQuery tempQuery;
	ok = tempQuery.exec(QString("SELECT * FROM checkOut WHERE fileId = %1").arg(query.value("fileId").toInt()));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value("userId").toInt() == query.value("userId").toInt(), qPrintable(QString("Error: in column \"userId\" - data mistmatch")));

	ok = tempQuery.exec(QString("SELECT * FROM file WHERE fileId = %1").arg(query.value("fileId").toInt()));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value("name").toString() == query.value("name").toString(), qPrintable(QString("Error: in column \"name\" - data mistmatch")));
	QVERIFY2(tempQuery.value("parentId").toInt() == query.value("parentId").toInt(), qPrintable(QString("Error: in column \"parentId\" - data mistmatch")));

	ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileId = %1").arg(query.value("fileId").toInt()));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value("size").toInt() == query.value("size").toInt(), qPrintable(QString("Error: in column \"size\" - data mistmatch")));
	QVERIFY2(data == query.value("data").toString(), qPrintable(QString("Error: in column \"data\" - data mistmatch")));
	QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable(QString("Error: in column \"action\" - data mistmatch")));
}

void FileTests::get_file_historyTest()
{
	QSqlQuery query;

	// Check get_file_history for fileid 1, it should have at least one record
	//
	int fileId = 1;

	bool ok = query.exec(QString("SELECT * FROM get_file_history(%1)").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QSqlQuery tempQuery;

	ok = tempQuery.exec(QString("SELECT * FROM fileInstance WHERE fileId = %1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value("changeSetId").toInt() == query.value("changesetId").toInt(), qPrintable("Error: changesetId is not match!"));
	QVERIFY2(tempQuery.value("action").toInt() == query.value("action").toInt(), qPrintable("Error: action is not match"));
	int changeSetId = query.value("changesetId").toInt();

	ok = tempQuery.exec(QString("SELECT * FROM changeSet WHERE changeSetId = %1").arg(changeSetId));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value("userId").toInt() == query.value("userId").toInt(), qPrintable("Error: userId not match!"));
	QVERIFY2(tempQuery.value("comment").toString() == query.value("comment").toString(), qPrintable("Error: comment is not match!"));

	// Check if function returns nothing
	//
	ok = query.exec(QString("SELECT * FROM get_file_history(%1)").arg(FileTests::maxValue));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Invalid fileId error expected"));

	// Just added file must have no history
	//
	ok = query.exec(QString("SELECT * FROM add_file(%1, 'getFileHistoryTest', 1, 'TEST')").arg(m_firstUserForTest));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM get_file_history(%1)").arg(query.value("id").toInt()));
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
	bool ok = query.exec(QString("SELECT * FROM add_file(%1, 'getFileStateTestFirst', 1, 'TEST')").arg(m_firstUserForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int fileId = query.value("id").toInt();

	// Add file and delete it to set "delete" flag
	//
	ok = query.exec("SELECT * FROM add_file(1, 'getFileStateTestSecond', 1, 'TEST')");
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

	ok = query.exec("SELECT * FROM get_file_state(99999)");
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Invalid fileId error expected"));
}

void FileTests::get_last_changesetTest()
{
	QSqlQuery query;

	bool ok = query.exec("SELECT * FROM get_last_changeset()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QSqlQuery tempQuery;

	ok = tempQuery.exec("SElECT MAX(changesetId) FROM changeSet");
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.value(0).toInt() == query.value(0).toInt(), qPrintable("Error: wrong changeset!"));
}

bool FileTests::fileExists(int fileID)
{
	QSqlQuery query;

	bool result = query.exec(QString("SELECT file_exists(%1);").arg(fileID));

	if (result == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	result = query.first();
	if (result == false)
	{
		qDebug() << "Cannot get first record";
		return false;
	}

	return query.value("file_exists").toBool();
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

bool FileTests::add_file(int userId, QString fileName, int parentId, QString fileData)
{
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT * FROM add_file(%1, '%2', %3, '%4');")
						 .arg(userId)
						 .arg(fileName)
						 .arg(parentId)
						 .arg(fileData));

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

	int fileId = resultQueryObject.newFileId;
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

bool FileTests::is_file_checkedout(int fileId)
{
	QSqlQuery query;
	bool ok = query.exec(QString("SELECT is_file_checkedout(%1);").arg(QString::number(fileId)));
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return ok;
	}

	ok = query.first();
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return ok;
	}

	if (query.value("is_file_checkedout").toBool() == false)
	{
		return false;
	}

	if (query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId)) == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.first() == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.value("checkedOutInstanceId").toUuid() == 0)
	{
		qDebug() << "Error: no record in column \"checkedOutInstanceId\" from \"file\"";
		return false;
	}

	if (query.value("checkedInInstanceId").toUuid() != 0)
	{
		qDebug() << "Error: there must not be record in column \"checkedInInstanceId\" from \"file\"";
		return false;
	}

	return true;
}

bool FileTests::is_any_checked_out()
{
	QSqlQuery query;
	bool ok = query.exec("Select is_any_checked_out();");
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

	return query.value("is_any_checked_out").toBool();
}

int FileTests::file_has_children(int user_id, int fileId)
{
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT file_has_children (%1,%2);")
						 .arg(user_id)
						 .arg(fileId));
	if (ok == false)
	{
		//qDebug() << query.lastError().databaseText();
		return -1;
	}

	ok = query.first();
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return -1;
	}

	return query.value("file_has_children").toInt();
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
