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
	query.exec("UPDATE file SET Deleted = true WHERE fileid = 3");

	//Testers
	query.exec("SElECT create_user(1, 'FIRSTTEST', 'FIRSTTEST', 'FIRSTTEST', '12341234', false, false, false);");
	query.first();
	FileTests::firstUserForTest = query.value("create_user").toInt();

	query.exec("SElECT create_user(1, 'SECONDTEST', 'SECONDTEST', 'SECONDTEST', '12341234', false, false, false);");
	query.first();
	FileTests::secondUserForTest = query.value("create_user").toInt();
}

void FileTests::cleanupTestCase()
{
	QSqlQuery query;

	//files_exist & file_exist litter
	query.exec("UPDATE file SET Deleted = false WHERE fileid = 3");
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
	QTest::newRow("arrayRandomFiles") << "99,9999,67" << "true,false,true";
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
	query.exec("SELECT * FROM add_file (1, 'isAnyCheckedOutTest', 2, 'TEST')");
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
	QTest::newRow("createdByUser") << FileTests::firstUserForTest << "TestCreatedByUser" << 2 << "OneTwoThreeFourFive" << true;
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

/*void FileTests::get_file_stateTest_data()
{
	QTest::addColumn<int>("fileId");
	QTest::addColumn<QString>("result");

	QTest::newRow("existedFilecheckedOut") << 416 << "(416,f,t,1,1,0)";
	QTest::newRow("existedFileNoCheckout") << 1 << "(1,f,f,1,1,0)";
	QTest::newRow("InvalidFile") << 999 << "MAIN QUERY EXECUTING ERROR"; //(999,t,f,,,0) ???
}

void FileTests::get_file_stateTest()
{
	QFETCH (int, fileId);
	QFETCH (QString, result);

	QCOMPARE (FileTests::get_file_state(fileId), result);
}
*/
void FileTests::is_file_checkedoutTest_data()
{
	QTest::addColumn<int>("fileId");
	QTest::addColumn<bool>("result");

	QSqlQuery query;
	query.exec("SELECT * FROM add_file (1, 'isFileCheckedOutTest', 2, 'TESTING')");
	query.first();

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

	query.exec(QString("SELECT * FROM add_file (%1, 'fileHasChildrenTestParent', 1, 'TESTING')").arg(FileTests::firstUserForTest));

	query.exec("SELECT count(*) FROM file WHERE parentid = 1");
	query.first();

	QTest::newRow("CreatedByFirstCalledByFirstUser") << FileTests::firstUserForTest << 1 << query.value(0).toInt();
	QTest::newRow("CreatedByFirstUserCalledBySecondUser") << FileTests::secondUserForTest << 1 << query.value(0).toInt()-1;
	QTest::newRow("CreatedByFirstUserCalledByAdmin") << 1 << 1 << query.value(0).toInt();
	QTest::newRow("InvalidUserFile") << 999 << 1 << 0; //???
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

	query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestFirstFile', 1, 'TESTING')").arg(FileTests::firstUserForTest));
	query.first();
	int firstFileOwnerFirstUser = query.value("id").toInt();

	query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestSecondFile', 1, 'TESTING')").arg(FileTests::firstUserForTest));
	query.first();
	int secondFileOwnerFirstUser = query.value("id").toInt();

	query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestThirdFile', 1, 'TESTING')").arg(FileTests::firstUserForTest));
	query.first();
	int thirdFileOwnerFirstUser = query.value("id").toInt();

	query.exec(QString("SELECT * FROM add_file (%1, 'deleteFileTestFourthFile', 1, 'TESTING')").arg(FileTests::firstUserForTest));
	query.first();
	int checkedInFileOwnerFirstUser = query.value("id").toInt();

	bool ok = query.exec(QString("SELECT check_in(%1, '{%2}', 'test')")
			   .arg(1)
			   .arg(checkedInFileOwnerFirstUser));
	if (ok == false)
		qDebug() << "ERROR EXECUTTING check_in QUERY";

	QTest::newRow("deleteFileOwnerFirstUserBySecondUser") << FileTests::secondUserForTest << firstFileOwnerFirstUser << false;
	QTest::newRow("deleteFileOwnerFirstUserByFirstUser") << FileTests::firstUserForTest << firstFileOwnerFirstUser << true;
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
		qDebug() << "ERROR: ERROR CODE IN RESULT FUNCTION IS NOT 0!";
		return false;
	}

	// Checking Table "file"
	//
	ok = query.exec("SELECT * FROM file WHERE fileId = " + QString::number(fileId));
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
		qDebug() << query.lastError().databaseText();
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
		qDebug() << "ERROR: RECORD IS NOT MATCHED TO BE DELETED";
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
		qDebug() << "RESULT ERROR: FILE HAS NOT BEEN DELETED FROM TABLE file";
		return false;
	}

	return true;
}

/*QString FileTests::get_file_state(int fileId)
{
	QSqlQuery query;
	bool ok = query.exec(QString("SELECT get_file_state(%1);").arg(fileId));
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return "MAIN QUERY EXECUTING ERROR";
	}

	ok = query.first();
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return "ERROR GET FIRST RECORD IN MAIN FUNCTION";
	}

	return query.value ("get_file_state").toString();
}*/
