#include "DbControllerFileManagementTests.h"
#include <QSql>
#include <QSqlError>
#include <QFile>
#include <QDebug>

DbControllerFileTests::DbControllerFileTests()
{
	m_dbController = new DbController();

	m_databaseHost = "127.0.0.1";
	m_databaseName = "dbcontrollerfiletesting";
	m_databaseUser = "u7";
	m_adminPassword = "P2ssw0rd";
}

void DbControllerFileTests::initTestCase()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("postgres");

	QVERIFY2 (db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

	QSqlQuery query, tempQuery;
	bool ok = query.exec("SELECT datname FROM pg_database WHERE datname LIKE 'u7_%' AND NOT datname LIKE 'u7u%'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next() == true)
	{
		if (query.value(0).toString() == "u7_" + m_databaseName)
		{
			ok = tempQuery.exec(QString("DROP DATABASE %1").arg(query.value(0).toString()));
			QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
			qDebug() << "Project " << query.value(0).toString() << "dropped!";
		}
	}

	db.close();

	ok = m_dbController->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not create project: " + m_dbController->lastError()));

	ok = m_dbController->upgradeProject(m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not upgrade project: " + m_dbController->lastError()));

	ok = m_dbController->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not open project: " + m_dbController->lastError()));
}

void DbControllerFileTests::getFileListTest()
{
	std::vector<DbFileInfo> files;

	bool ok = m_dbController->getFileList(&files, 1, false, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(QString("Error: Can not connect to %1 database! ").arg("u7_" + m_databaseName) + db.lastError().databaseText()));

	QSqlQuery query;

	ok = query.exec("SELECT * from get_file_list(1, 1, '%');");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	QVector<QString> filesForCheck;

	while (query.next())
	{
		filesForCheck.push_back(query.value("name").toString());
	}

	db.close();

	QVERIFY2 (uint(filesForCheck.size()) == files.size(), qPrintable("Error: getFileList() function returned wrong amount of files!"));

	for (DbFileInfo buff : files)
	{
		QVERIFY2 (filesForCheck.contains(buff.fileName()) == true, qPrintable("Error: wrong files has been returned by getFileList() function"));
	}
}

void DbControllerFileTests::addFileTest()
{
	QString details = "{\"Type\": \".hws\", \"Uuid\": \"{00000000-0000-0000-0000-000000000000}\", \"Place\": 0, \"StrID\": \"$(PARENT)_WS00\", \"Caption\": \"Workstation\"}";
	QString name = "\'\"\\FileForAddFilesTest%\'\"\\";

	std::shared_ptr<DbFile> file(new DbFile);
	file.get()->setUserId(1);
	file.get()->setParentId(1);
	file.get()->setDetails(details);

	QFile fileFromDisk(name);

	if (fileFromDisk.exists())
	{
		QVERIFY2 (fileFromDisk.remove(), qPrintable(QString("Can not remove old test file from disk in function addFileTest of DbController tests: %1").arg(fileFromDisk.errorString())));
	}

	if (fileFromDisk.open(QIODevice::ReadWrite))
	{
		QVERIFY2 (fileFromDisk.write("Testing data"), qPrintable(QString("Can not create file to read from in function addFileTest of DbController tests: %1").arg(fileFromDisk.errorString())));
		fileFromDisk.close();
	}

	file.get()->readFromDisk(name);


	QVERIFY2 (fileFromDisk.remove(), qPrintable(QString("Can not remove old test file from disk in function addFileTest of DbController tests: %1").arg(fileFromDisk.errorString())));

	bool ok = m_dbController->addFile(file, 1, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QSqlQuery query;
	QString nameForDb = name;
	nameForDb.replace("\'","\'\'");
	ok = query.exec(QString("SELECT * FROM file WHERE name = \'%1\'").arg(nameForDb));
	QVERIFY2 (ok, qPrintable(query.lastError().databaseText()));

	ok = query.first();
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value("name").toString() == name, qPrintable("Error: file created by function addFile from DbController has wrong name"));
	QVERIFY2 (query.value("parentId").toInt() == 1, qPrintable("Error: file created by function addFile from DbController has wrong parentId"));
	QVERIFY2 (query.value("Deleted").toBool() == false, qPrintable("Error: file created by function addFile from DbController has wrong deleted flag"));

	QString fileInstanceId = query.value("checkedOutInstanceId").toString();

	ok = query.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = \'%1\'").arg(fileInstanceId));
	QVERIFY2 (ok, qPrintable(query.lastError().databaseText()));

	ok = query.first();
	QVERIFY2 (ok, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value("details").toString() == details, qPrintable("Error: wrong detais after addFile function of DbController"));
	db.close();
}

void DbControllerFileTests::addFilesTest()
{
	QString details = "{}";
	QString nameFirst = "\'\"\\FirstFileForAddFilesTest%\'\"\\";
	QString nameSecond = "\'\"\\SuddenlySecondFileAddFilesTest%\'\"\\";

	std::vector<std::shared_ptr<DbFile>> files;

	std::shared_ptr<DbFile> file1(new DbFile);
	std::shared_ptr<DbFile> file2(new DbFile);

	QFile firstFileFromDisk(nameFirst);
	QFile secondFileFromDisk(nameSecond);


	// Create and read data from first file
	//

	file1.get()->setUserId(1);
	file1.get()->setParentId(1);
	file1.get()->setDetails(details);

	if (firstFileFromDisk.exists())
	{
		QVERIFY2 (firstFileFromDisk.remove(), qPrintable(QString("Can not remove old test file from disk in function addFileTest of DbController tests: %1").arg(firstFileFromDisk.errorString())));
	}

	if (firstFileFromDisk.open(QIODevice::ReadWrite))
	{
		QVERIFY2 (firstFileFromDisk.write("Testing data"), qPrintable(QString("Can not create file to read from in function addFileTest of DbController tests: %1").arg(firstFileFromDisk.errorString())));
		firstFileFromDisk.close();
	}

	QVERIFY2 (file1.get()->readFromDisk(nameFirst), qPrintable("Can not read file (first)"));

	QVERIFY2 (firstFileFromDisk.remove(), qPrintable(QString("Can not remove old test file from disk in function addFileTest of DbController tests: %1").arg(firstFileFromDisk.errorString())));

	files.push_back(file1);

	// Create and read data from second file
	//

	file2.get()->setUserId(1);
	file2.get()->setParentId(1);
	file2.get()->setDetails(details);
	file2.get()->setFileName(nameSecond);

	if (secondFileFromDisk.exists())
	{
		QVERIFY2 (secondFileFromDisk.remove(), qPrintable(QString("Can not remove old test file from disk in function addFileTest of DbController tests: %1").arg(secondFileFromDisk.errorString())));
	}

	if (secondFileFromDisk.open(QIODevice::ReadWrite))
	{
		QVERIFY2 (secondFileFromDisk.write("Testing data 2"), qPrintable(QString("Can not create file to read from in function addFileTest of DbController tests: %1").arg(secondFileFromDisk.errorString())));
		secondFileFromDisk.close();
	}

	QVERIFY2 (file2.get()->readFromDisk(nameSecond), qPrintable("Can not read file (second)"));

	QVERIFY2 (secondFileFromDisk.remove(), qPrintable(QString("Can not remove old test file from disk in function addFileTest of DbController tests: %1").arg(secondFileFromDisk.errorString())));

	files.push_back(file2);

	bool ok = m_dbController->addFiles(&files, 1, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QSqlQuery query;
	QString nameForDb;
	for (std::shared_ptr<DbFile> buff : files)
	{
		nameForDb = buff.get()->fileName();
		nameForDb.replace("\'","\'\'");
		ok = query.exec(QString("SELECT * FROM file WHERE name = \'%1\'").arg(nameForDb));
		QVERIFY2 (ok, qPrintable(query.lastError().databaseText()));

		ok = query.first();
		QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2 (query.value("name").toString() == buff.get()->fileName(), qPrintable("Error: file created by function addFile from DbController has wrong name"));
		QVERIFY2 (query.value("parentId").toInt() == 1, qPrintable("Error: file created by function addFile from DbController has wrong parentId"));
		QVERIFY2 (query.value("Deleted").toBool() == false, qPrintable("Error: file created by function addFile from DbController has wrong deleted flag"));

		QString fileInstanceId = query.value("checkedOutInstanceId").toString();

		ok = query.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceId = \'%1\'").arg(fileInstanceId));
		QVERIFY2 (ok, qPrintable(query.lastError().databaseText()));

		ok = query.first();
		QVERIFY2 (ok, qPrintable(query.lastError().databaseText()));

		QVERIFY2 (query.value("details").toString() == details, qPrintable("Error: wrong detais after addFile function of DbController"));
	}
	db.close();
}

void DbControllerFileTests::deleteFileTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	QString fileOne = "firstFile";
	QString fileTwo = "\'\"\\secondFile%\'\"\\";

	// Before deleting some files by function, we must add them to database by
	// using sql-queries. To do that, we need manually represent string to
	// sql-format.
	//

	QString fileTwoForSqlQuery = "\'\'\"\"\\secondFile%\'\'\"\"\\";
	QString data = "dataForFile";

	QSqlQuery query;
	QSqlQuery instanceQuery;

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, '%2', '{}')").arg(fileOne).arg(data));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, '%2', '{}')").arg(fileTwoForSqlQuery).arg(data));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	std::vector<DbFileInfo> files;
	DbFile buffFile;

	ok = query.exec(QString("SELECT * FROM file WHERE name='%1'").arg(fileOne));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	buffFile.setFileName(fileOne);
	buffFile.setSize(instanceQuery.value("Size").toInt());
	buffFile.setUserId(1);
	buffFile.setParentId(1);
	buffFile.setDetails(instanceQuery.value("details").toString());
	buffFile.setFileId(query.value("fileId").toInt());

	files.push_back(buffFile);

	buffFile.clearData();

	ok = query.exec(QString("SELECT * FROM file WHERE name='%1'").arg(fileTwoForSqlQuery));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	buffFile.setFileName(fileTwo);
	buffFile.setSize(instanceQuery.value("Size").toInt());
	buffFile.setUserId(1);
	buffFile.setParentId(1);
	buffFile.setDetails(instanceQuery.value("details").toString());
	buffFile.setFileId(query.value("fileId").toInt());

	files.push_back(buffFile);

	buffFile.clearData();

	ok = m_dbController->deleteFiles(&files, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM file WHERE name='%1' OR name='%2'").arg(fileTwoForSqlQuery).arg(fileOne));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Error: files expected to be deleted"));

	// Test another overloaded function
	//

	std::vector<std::shared_ptr<DbFileInfo>> filesAnotherFunction;

	std::shared_ptr<DbFileInfo> file1(new DbFile);
	std::shared_ptr<DbFileInfo> file2(new DbFile);

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileOne));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileTwoForSqlQuery));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM file WHERE name='%1'").arg(fileOne));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file1.get()->setFileName(fileOne);
	file1.get()->setSize(instanceQuery.value("Size").toInt());
	file1.get()->setUserId(1);
	file1.get()->setParentId(1);
	file1.get()->setDetails(instanceQuery.value("details").toString());
	file1.get()->setFileId(query.value("fileId").toInt());

	filesAnotherFunction.push_back(file1);

	ok = query.exec(QString("SELECT * FROM file WHERE name='%1'").arg(fileTwoForSqlQuery));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file2.get()->setFileName(fileTwo);
	file2.get()->setSize(instanceQuery.value("Size").toInt());
	file2.get()->setUserId(1);
	file2.get()->setParentId(1);
	file2.get()->setDetails(instanceQuery.value("details").toString());
	file2.get()->setFileId(query.value("fileId").toInt());

	filesAnotherFunction.push_back(file2);

	ok = m_dbController->deleteFiles(&filesAnotherFunction, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM file WHERE name='%1' OR name='%2'").arg(fileTwoForSqlQuery).arg(fileOne));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == false, qPrintable("Error: files expected to be deleted"));

	db.close();
}

void DbControllerFileTests::getFileInfo()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	QString fileOne = "firstFile";

	QSqlQuery query;
	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileOne));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	DbFileInfo fileInfo;

	ok = m_dbController->getFileInfo(fileId, &fileInfo, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM get_file_info(1, '{%1}')").arg(fileId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(fileInfo.fileId() == query.value("fileId").toInt(), qPrintable("Wrong fileId returned by dbController function()"));
	QVERIFY2(fileInfo.deleted() == query.value("deleted").toBool(), qPrintable("Wrong deleted flag returned by dbController function()"));
	QVERIFY2(fileInfo.fileName() == query.value("name").toString(), qPrintable("Wrong name returned by dbController function()"));
	QVERIFY2(fileInfo.parentId() == query.value("parentId").toInt(), qPrintable("Wrong parentId returned by dbController function()"));
	QVERIFY2(fileInfo.changeset() == query.value("changesetId").toInt(), qPrintable("Wrong changeset returned by dbController function()"));
	QVERIFY2(fileInfo.size() == query.value("size").toInt(), qPrintable("Wrong size returned by dbController function()"));
	QVERIFY2(fileInfo.userId() == query.value("userId").toInt(), qPrintable("Wrong userId returned by dbController function()"));
	QVERIFY2(fileInfo.action().toInt() == query.value("action").toInt(), qPrintable("Wrong action returned by dbController function()"));
	QVERIFY2(fileInfo.details() == query.value("details").toString(), qPrintable("Wrong details returned by dbController function()"));
	db.close();
}

void DbControllerFileTests::checkInTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	DbFileInfo file1;

	QSqlQuery query, instanceQuery;

	QString fileOne = "FirstFileCheckIn";

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileOne));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int firstFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(firstFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file1.setFileName(fileOne);
	file1.setSize(instanceQuery.value("Size").toInt());
	file1.setUserId(1);
	file1.setParentId(1);
	file1.setDetails(instanceQuery.value("details").toString());
	file1.setFileId(query.value("fileId").toInt());

	QString comment = "Testing function check_in of dbController with one file";

	ok = m_dbController->checkIn(file1, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT COUNT(*) FROM  checkout WHERE fileId = %1").arg(firstFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value(0).toInt() == 0, qPrintable("Error: function check_in has not checked in one file (table checkout has record about checked in file)"));

	ok = query.exec(QString("SELECT * FROM changeset WHERE comment = '%1'").arg(comment));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT COUNT(*) FROM fileInstance WHERE changesetId = %1").arg(query.value("changesetId").toInt()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	QVERIFY2 (instanceQuery.value(0).toInt() == 1, qPrintable("Error: function check_in has not checked in one file (wrong record in fileInstance)"));

	ok = query.exec(QString("SELECT * FROM check_out(1, '{%1}')").arg(firstFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	DbFileInfo file2;
	QString fileTwo = "SecondFileCheckIn";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileTwo));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int secondFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(secondFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file2.setFileName(fileTwo);
	file2.setSize(instanceQuery.value("Size").toInt());
	file2.setUserId(1);
	file2.setParentId(1);
	file2.setDetails(instanceQuery.value("details").toString());
	file2.setFileId(query.value("fileId").toInt());

	comment = "Testing function check_in of dbController with two files";

	std::vector<DbFileInfo> files;

	files.push_back(file1);
	files.push_back(file2);

	ok = m_dbController->checkIn(files, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT COUNT(*) FROM checkout WHERE fileId = %1 OR fileId = %2").arg(firstFileId).arg(secondFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value(0).toInt() == 0, qPrintable("Error: function check_in has not checked in one file (table checkout has record about checked in file)"));

	ok = query.exec(QString("SELECT * FROM changeset WHERE comment = '%1'").arg(comment));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT COUNT(*) FROM fileInstance WHERE changesetId = %1").arg(query.value("changesetId").toInt()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	QVERIFY2 (instanceQuery.value(0).toInt() == 2, qPrintable("Error: function check_in has not checked in one file (wrong record in fileInstance)"));

	DbFileInfo file3;

	QString fileThree = "\\\'\"SpecialSymbolsFileCheckIn%\\\'\"";
	QString fileThreeForSqlQuery = "\\\'\'\"\"SpecialSymbolsFileCheckIn%\\\'\'\"\"";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileThreeForSqlQuery));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int thirdFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(thirdFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file3.setFileName(fileThree);
	file3.setSize(instanceQuery.value("Size").toInt());
	file3.setUserId(1);
	file3.setParentId(1);
	file3.setDetails(instanceQuery.value("details").toString());
	file3.setFileId(query.value("fileId").toInt());

	comment = "Testing function check_in of dbController with file which name contains special symbols\\'\"";

	ok = m_dbController->checkIn(file3, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT COUNT(*) FROM  checkout WHERE fileId = %1").arg(thirdFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value(0).toInt() == 0, qPrintable("Error: function check_in has not checked in one file (table checkout has record about checked in file)"));

	comment.remove(comment.size()-4, 4);

	ok = query.exec(QString("SELECT * FROM changeset WHERE comment LIKE '%1%'").arg(comment));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT COUNT(*) FROM fileInstance WHERE changesetId = %1").arg(query.value("changesetId").toInt()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	QVERIFY2 (instanceQuery.value(0).toInt() == 1, qPrintable("Error: function check_in has not checked in one file (wrong record in fileInstance)"));

	db.close();
}

void DbControllerFileTests::checkOutTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	DbFileInfo file1;

	QSqlQuery query, instanceQuery;

	QString fileOne = "FirstFileCheckOut";

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileOne));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int firstFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(firstFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file1.setFileName(fileOne);
	file1.setSize(instanceQuery.value("Size").toInt());
	file1.setUserId(1);
	file1.setParentId(1);
	file1.setDetails(instanceQuery.value("details").toString());
	file1.setFileId(query.value("fileId").toInt());

	QString comment = "Testing function check_out of dbController";

	ok = m_dbController->checkIn(file1, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(file1, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT COUNT(*) FROM  checkout WHERE fileId = %1").arg(firstFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value(0).toInt() == 1, qPrintable("Error: function check_out was not create record in checkout table (one file)"));

	DbFileInfo file2;
	QString fileTwo = "SecondFileCheckOut";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileTwo));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int secondFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(secondFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file2.setFileName(fileTwo);
	file2.setSize(instanceQuery.value("Size").toInt());
	file2.setUserId(1);
	file2.setParentId(1);
	file2.setDetails(instanceQuery.value("details").toString());
	file2.setFileId(query.value("fileId").toInt());

	std::vector<DbFileInfo> files;

	files.push_back(file1);
	files.push_back(file2);

	ok = m_dbController->checkIn(files, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(files, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT COUNT(*) FROM checkout WHERE fileId = %1 OR fileId = %2").arg(firstFileId).arg(secondFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value(0).toInt() == 2, qPrintable("Error: function check_out was not create record in checkout table (two files)"));

	DbFileInfo file3;

	QString fileThree = "CheckOutFileWithSpecialSymbols\\'\"";
	QString fileThreeForSql = "CheckOutFileWithSpecialSymbols\\''\"\"";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileThreeForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int thirdFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(thirdFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file3.setFileName(fileThree);
	file3.setSize(instanceQuery.value("Size").toInt());
	file3.setUserId(1);
	file3.setParentId(1);
	file3.setDetails(instanceQuery.value("details").toString());
	file3.setFileId(query.value("fileId").toInt());

	comment = "Testing function check_out of dbController with special symbols";

	ok = m_dbController->checkIn(file3, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(file3, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT COUNT(*) FROM  checkout WHERE fileId = %1").arg(firstFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value(0).toInt() == 1, qPrintable("Error: function check_out was not create record in checkout table (one file)"));

	db.close();
}

void DbControllerFileTests::fileHasChildrenTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	DbFileInfo parentFile, childFile;

	QSqlQuery query, instanceQuery;

	QString parentFileName = "FirstFilehasChildren\\'\"";
	QString childFileName = "SecndFilehasChildren\\'\"";

	QString parentFileForSql = "FirstFilehasChildren\\''\"\"";
	QString childFileForSql = "SecndFilehasChildren\\''\"\"";

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(parentFileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int parentFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(parentFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	parentFile.setFileName(parentFileName);
	parentFile.setSize(instanceQuery.value("Size").toInt());
	parentFile.setUserId(1);
	parentFile.setParentId(1);
	parentFile.setDetails(instanceQuery.value("details").toString());
	parentFile.setFileId(query.value("fileId").toInt());

	QString comment = "ParentFile for test function fileHasChildren of dbController";

	ok = m_dbController->checkIn(parentFile, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	bool result;

	ok = m_dbController->fileHasChildren(&result, parentFile, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2 (result == false, qPrintable("Error: function fileHasChildren from dbController returned wrong result (false expected)"));

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', %2, 'LOL', '{}')").arg(childFileForSql).arg(parentFileId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int childFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(childFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	childFile.setFileName(childFileName);
	childFile.setSize(instanceQuery.value("Size").toInt());
	childFile.setUserId(1);
	childFile.setParentId(1);
	childFile.setDetails(instanceQuery.value("details").toString());
	childFile.setFileId(query.value("fileId").toInt());

	comment = "Testing function fileHasChildren of dbController";

	ok = m_dbController->checkIn(childFile, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->fileHasChildren(&result, parentFile, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2 (result == true, qPrintable("Error: function fileHasChildren from dbController returned wrong result (true expected)"));

	db.close();
}

void DbControllerFileTests::getCheckedOutFilesTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	DbFileInfo parentFile, firstChildFile, secondChildFile;

	QSqlQuery query, instanceQuery;

	QString parentFileName = "ParentFileForGetCheckedOutFiles\\'\"";
	QString firstChildFileName = "FirstChildFileForGetCheckedOutFiles\\'\"";
	QString secondChildFileName = "SecondChildFileForGetCheckedOutFiles\\'\"";

	QString parentFileForSql = "ParentFileForGetCheckedOutFiles\\''\"\"";
	QString firstChildFileForSql = "FirstChildFileForGetCheckedOutFiles\\''\"\"";
	QString secondChildFileForSql = "SecondChildFileForGetCheckedOutFiles\\''\"\"";

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(parentFileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int parentFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(parentFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	parentFile.setFileName(parentFileName);
	parentFile.setSize(instanceQuery.value("Size").toInt());
	parentFile.setUserId(1);
	parentFile.setParentId(1);
	parentFile.setDetails(instanceQuery.value("details").toString());
	parentFile.setFileId(query.value("fileId").toInt());

	QString comment = "ParentFile for test function getCheckedoutFiles of dbController";

	ok = m_dbController->checkIn(parentFile, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', %2, 'LOL', '{}')").arg(firstChildFileForSql).arg(parentFileId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int childFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(childFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	firstChildFile.setFileName(firstChildFileName);
	firstChildFile.setSize(instanceQuery.value("Size").toInt());
	firstChildFile.setUserId(1);
	firstChildFile.setParentId(1);
	firstChildFile.setDetails(instanceQuery.value("details").toString());
	firstChildFile.setFileId(query.value("fileId").toInt());

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', %2, 'LOL', '{}')").arg(secondChildFileForSql).arg(parentFileId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	childFileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(childFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	secondChildFile.setFileName(secondChildFileName);
	secondChildFile.setSize(instanceQuery.value("Size").toInt());
	secondChildFile.setUserId(1);
	secondChildFile.setParentId(1);
	secondChildFile.setDetails(instanceQuery.value("details").toString());
	secondChildFile.setFileId(query.value("fileId").toInt());

	std::vector<DbFileInfo> vectorWithParentFile;
	std::vector<DbFileInfo> result;

	vectorWithParentFile.push_back(parentFile);

	ok = m_dbController->getCheckedOutFiles(&vectorWithParentFile, &result, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2(result.size() == 2, qPrintable("Error: wrong amount of records in result"));

	DbFileInfo resultFile = result.at(0);

	QVERIFY2(resultFile.fileId() == firstChildFile.fileId(), qPrintable("Error: wrong file has been returned"));

	resultFile = result.at(1);

	QVERIFY2(resultFile.fileId() == secondChildFile.fileId(), qPrintable("Error: wrong file has been returned"));
}

void DbControllerFileTests::getFileHistoryTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	DbFileInfo file;

	QSqlQuery query, instanceQuery;

	QString fileName = "FileForGetFileHistory\\'\"";
	QString fileForSql = "FileForGetFileHistory\\''\"\"";

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file.setFileName(fileName);
	file.setSize(instanceQuery.value("Size").toInt());
	file.setUserId(1);
	file.setParentId(1);
	file.setDetails(instanceQuery.value("details").toString());
	file.setFileId(query.value("fileId").toInt());

	QString comment = "First file commit. I have no idea: what I must do";

	ok = m_dbController->checkIn(file, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(file, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	comment = "Second file commit. I still have no idea, what I must do";

	ok = m_dbController->checkIn(file, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(file, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	comment = "Third file commit. I have idea, what to do";

	ok = m_dbController->checkIn(file, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM get_file_history(%1);").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	std::vector<DbChangesetInfo> result;

	ok = m_dbController->getFileHistory(file, &result, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	while (query.next())
	{
		bool exist = false;

		for (uint currentElement = 0; currentElement < result.size(); currentElement++)
		{
			DbChangesetInfo buff = result.at(currentElement);
			if (buff.changeset() == query.value("changesetId").toInt())
			{
				exist = true;
			}
		}

		QVERIFY2 (exist == true, qPrintable ("Error: function getFileHistory of DbController has returned wrong result!"));
	}

	db.close();
}

void DbControllerFileTests::getLatestFileVersionTest()
{
	std::shared_ptr<DbFile> singleFileOutput(new DbFile);
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	DbFileInfo file;

	QSqlQuery query, instanceQuery;

	QString fileName = "FileForGetLatestFileVersion\\'\"";
	QString fileForSql = "FileForGetLatestFileVersion\\''\"\"";

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file.setFileName(fileName);
	file.setSize(instanceQuery.value("Size").toInt());
	file.setUserId(1);
	file.setParentId(1);
	file.setDetails(instanceQuery.value("details").toString());
	file.setFileId(query.value("fileId").toInt());

	QString comment = "First file commit. I have no idea: what I must do";

	ok = m_dbController->checkIn(file, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(file, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	comment = "Second file commit. I still have no idea, what I must do";

	ok = m_dbController->checkIn(file, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(file, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	comment = "Third file commit. I have idea, what to do";

	ok = m_dbController->checkIn(file, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->getLatestVersion(file, &singleFileOutput, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM get_latest_file_version (1, %1)").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("changesetId").toInt() == singleFileOutput.get()->changeset(), qPrintable ("Error: wrong changeset Id in get_latest_file function of DbController"));

	DbFileInfo secondFile;

	fileName = "SecondFileForGetLatestFileVersion\\'\"";
	fileForSql = "SecondFileForGetLatestFileVersion\\''\"\"";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	secondFile.setFileName(fileName);
	secondFile.setSize(instanceQuery.value("Size").toInt());
	secondFile.setUserId(1);
	secondFile.setParentId(1);
	secondFile.setDetails(instanceQuery.value("details").toString());
	secondFile.setFileId(query.value("fileId").toInt());

	comment = "First file commit. I have no idea: what I must do";

	ok = m_dbController->checkIn(secondFile, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(secondFile, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	comment = "Second file commit. I still have no idea, what I must do";

	ok = m_dbController->checkIn(secondFile, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(secondFile, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	comment = "Third file commit. I have idea, what to do";

	ok = m_dbController->checkIn(secondFile, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	std::vector<DbFileInfo> files;
	files.push_back(file);
	files.push_back(secondFile);

	std::vector<std::shared_ptr<DbFile>> multiFileOutput;

	ok = m_dbController->getLatestVersion(files, &multiFileOutput, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	for (std::shared_ptr<DbFile> buff : multiFileOutput)
	{

		ok = query.exec(QString("SELECT * FROM get_latest_file_version(1, %1)").arg(buff.get()->fileId()));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

		QVERIFY2(query.value("changesetId").toInt() == buff.get()->changeset(), qPrintable ("Error: wrong changeset Id in get_latest_file function of DbController"));
	}

	db.close();
}

void DbControllerFileTests::getLatestTreeVersionTest()
{
	qRegisterMetaType<DbFileInfo>("DbFileInfo");
	std::vector<std::shared_ptr<DbFile>> result;

	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	DbFileInfo file;

	QSqlQuery query, instanceQuery;

	QString fileName = "ParentFileForGetLatestTreeVersionTestOfDbController\\'\"";
	QString fileForSql = "ParentFileForGetLatestTreeVersionTestOfDbController\\''\"\"";

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file.setFileName(fileName);
	file.setSize(instanceQuery.value("Size").toInt());
	file.setUserId(1);
	file.setParentId(1);
	file.setDetails(instanceQuery.value("details").toString());
	file.setFileId(query.value("fileId").toInt());

	QString comment = "Parent file for GetLatestTreeVersionTest created";

	ok = m_dbController->checkIn(file, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(file, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	comment = "Parent file for GetLatestTreeVersionTest updated";

	ok = m_dbController->checkIn(file, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	DbFileInfo childFile;

	fileName = "ChildFileForGetLatestTreeVersionTestOfDbController\\'\"";
	fileForSql = "ChildFileForGetLatestTreeVersionTestOfDbController\\''\"\"";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', %2, 'LOL', '{}')").arg(fileForSql).arg(fileId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int parentFileId = fileId;
	fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	childFile.setFileName(fileName);
	childFile.setSize(instanceQuery.value("Size").toInt());
	childFile.setUserId(1);
	childFile.setParentId(1);
	childFile.setDetails(instanceQuery.value("details").toString());
	childFile.setFileId(query.value("fileId").toInt());

	comment = "Child file for GetLatestTreeVersionTest created";

	ok = m_dbController->checkIn(childFile, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(childFile, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	comment = "Child file for GetLatestTreeVersionTest updated";

	ok = m_dbController->checkIn(childFile, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->getLatestTreeVersion(file, &result, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2 (result.size() == 2, qPrintable ("Error: function getLatestTreeVersion returned wrong amount of files"));

	ok = query.exec(QString("SELECT * FROM get_latest_file_tree_version(1, %1)").arg(parentFileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next())
	{
		bool exist = false;

		for (std::shared_ptr<DbFile> buff : result)
		{
			if (buff.get()->fileId() == query.value("fileId").toInt())
			{
				exist = true;
			}
		}

		QVERIFY2 (exist == true, qPrintable("Error: function getLatestTreeVersion returned wrong data!"));
	}

	db.close();
}

void DbControllerFileTests::getWorkcopyTest()
{
	std::shared_ptr<DbFile> result(new DbFile);

	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	DbFileInfo file;

	QSqlQuery query, instanceQuery;

	QString fileName = "FirstFileForGetWorkcopyTestOfDbController\\'\"";
	QString fileForSql = "FirstFileForGetWorkcopyTestOfDbController\\''\"\"";

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file.setFileName(fileName);
	file.setSize(instanceQuery.value("Size").toInt());
	file.setUserId(1);
	file.setParentId(1);
	file.setDetails(instanceQuery.value("details").toString());
	file.setFileId(query.value("fileId").toInt());

	QString comment = "File for getWorkcopy test of dbController created";

	ok = m_dbController->checkIn(file, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(file, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->getWorkcopy(file, &result, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM get_workcopy(1, %1)").arg(fileId));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (result.get()->fileId() == query.value("fileId").toInt(), qPrintable("Error: wrong file Id"));
	QVERIFY2 (result.get()->details() == query.value("details").toString(), qPrintable("Error: wrong file Id"));
	QVERIFY2 (result.get()->action().toInt() == query.value("action").toInt(), qPrintable("Error: wrong file Id"));
	QVERIFY2 (result.get()->userId() == query.value("userId").toInt(), qPrintable("Error: wrong file Id"));
	QVERIFY2 (result.get()->fileName() == query.value("name").toString(), qPrintable("Error: wrong file Id"));
	QVERIFY2 (result.get()->parentId() == query.value("parentId").toInt(), qPrintable("Error: wrong file Id"));

	DbFileInfo secondFile;

	fileName = "SecondFileForGetWorkcopyTestOfDbController\\'\"";
	fileForSql = "SecondFileForGetWorkcopyTestOfDbController\\''\"\"";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	secondFile.setFileName(fileName);
	secondFile.setSize(instanceQuery.value("Size").toInt());
	secondFile.setUserId(1);
	secondFile.setParentId(1);
	secondFile.setDetails(instanceQuery.value("details").toString());
	secondFile.setFileId(query.value("fileId").toInt());

	comment = "Second file for getWorkcopy test of dbController created";

	ok = m_dbController->checkIn(secondFile, comment, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(secondFile, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	std::vector<DbFileInfo> files;
	std::vector<std::shared_ptr<DbFile>> multipleResult;

	files.push_back(file);
	files.push_back(secondFile);

	ok = m_dbController->getWorkcopy(files, &multipleResult, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2 (multipleResult.size() == files.size(), qPrintable("Error: wrong amount of records in vector from function result"));

	for (std::shared_ptr<DbFile> buff : multipleResult)
	{
		ok = query.exec(QString("SELECT * FROM get_workcopy(1, %1)").arg(buff.get()->fileId()));
		QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));

		QVERIFY2 (buff.get()->fileId() == query.value("fileId").toInt(), qPrintable("Error: wrong file Id"));
		QVERIFY2 (buff.get()->details() == query.value("details").toString(), qPrintable("Error: wrong file Id"));
		QVERIFY2 (buff.get()->action().toInt() == query.value("action").toInt(), qPrintable("Error: wrong file Id"));
		QVERIFY2 (buff.get()->userId() == query.value("userId").toInt(), qPrintable("Error: wrong file Id"));
		QVERIFY2 (buff.get()->fileName() == query.value("name").toString(), qPrintable("Error: wrong file Id"));
		QVERIFY2 (buff.get()->parentId() == query.value("parentId").toInt(), qPrintable("Error: wrong file Id"));
	}

	db.close();
}

void DbControllerFileTests::setWorkcopyTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	std::shared_ptr<DbFile> file(new DbFile);
	std::shared_ptr<DbFile> secondFile(new DbFile);

	QSqlQuery query, instanceQuery;

	QString fileName = "FirstFileForSetWorkcopyTestOfDbController\\'\"";
	QString fileForSql = "FirstFileForSetWorkcopyTestOfDbController\\''\"\"";

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file.get()->setFileName(fileName);
	file.get()->setSize(instanceQuery.value("Size").toInt());
	file.get()->setUserId(1);
	file.get()->setParentId(1);
	file.get()->setDetails(instanceQuery.value("details").toString());
	file.get()->setFileId(query.value("fileId").toInt());

	QString name = "FileForSetWorkcopyFunctionFromDbControllerTest";

	QFile fileFromDisk(name);

	if (fileFromDisk.exists())
	{
		QVERIFY2 (fileFromDisk.remove(), qPrintable(QString("Can not remove old test file from disk in function addFileTest of DbController tests: %1").arg(fileFromDisk.errorString())));
	}

	if (fileFromDisk.open(QIODevice::ReadWrite))
	{
		QVERIFY2 (fileFromDisk.write("Testing data"), qPrintable(QString("Can not create file to read from in function addFileTest of DbController tests: %1").arg(fileFromDisk.errorString())));
		fileFromDisk.close();
	}

	file.get()->readFromDisk(name);


	QVERIFY2 (fileFromDisk.remove(), qPrintable(QString("Can not remove old test file from disk in function addFileTest of DbController tests: %1").arg(fileFromDisk.errorString())));


	ok = m_dbController->setWorkcopy(file, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM get_workcopy(1, %1)").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value("data").toString() == file.get()->data(), qPrintable("Error: function set_wor"));
	QVERIFY2 (query.value("size").toInt() == file.get()->size(), qPrintable("Error: wrong size"));
	QVERIFY2 (query.value("details").toString() == file.get()->details(), qPrintable("Errpr: wrong details"));

	fileName = "SecondFileForSetWorkcopyTestOfDbController\\'\"";
	fileForSql = "SecondFileForSetWorkcopyTestOfDbController\\''\"\"";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	secondFile.get()->setFileName(fileName);
	secondFile.get()->setSize(instanceQuery.value("Size").toInt());
	secondFile.get()->setUserId(1);
	secondFile.get()->setParentId(1);
	secondFile.get()->setDetails(instanceQuery.value("details").toString());
	secondFile.get()->setFileId(query.value("fileId").toInt());

	name = "SecondFileForSetWorkcopyFunctionFromDbControllerTest";

	fileFromDisk.setFileName(name);

	if (fileFromDisk.exists())
	{
		QVERIFY2 (fileFromDisk.remove(), qPrintable(QString("Can not remove old test file from disk in function addFileTest of DbController tests: %1").arg(fileFromDisk.errorString())));
	}

	if (fileFromDisk.open(QIODevice::ReadWrite))
	{
		QVERIFY2 (fileFromDisk.write("Testing data For Second File"), qPrintable(QString("Can not create file to read from in function addFileTest of DbController tests: %1").arg(fileFromDisk.errorString())));
		fileFromDisk.close();
	}

	secondFile.get()->readFromDisk(name);


	QVERIFY2 (fileFromDisk.remove(), qPrintable(QString("Can not remove old test file from disk in function addFileTest of DbController tests: %1").arg(fileFromDisk.errorString())));

	std::vector<std::shared_ptr<DbFile>> files;

	files.push_back(file);
	files.push_back(secondFile);

	m_dbController->setWorkcopy(files, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	for (std::shared_ptr<DbFile> buff : files)
	{
		ok = query.exec(QString("SELECT * FROM get_workcopy(1, %1)").arg(buff.get()->fileId()));
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

		QVERIFY2 (query.value("data").toString() == buff.get()->data(), qPrintable("Error: function set_wor"));
		QVERIFY2 (query.value("size").toInt() == buff.get()->size(), qPrintable("Error: wrong size"));
		QVERIFY2 (query.value("details").toString() == buff.get()->details(), qPrintable("Errpr: wrong details"));
	}

	db.close();
}

void DbControllerFileTests::getSpecificCopyTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	DbFileInfo file, secondFile;

	QSqlQuery query, instanceQuery;

	QString fileName = "FirstFileForGetSpecificCopyTestOfDbController\\'\"";
	QString fileForSql = "FirstFileForGetSpecificCopyTestOfDbController\\''\"\"";

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file.setFileName(fileName);
	file.setSize(instanceQuery.value("Size").toInt());
	file.setUserId(1);
	file.setParentId(1);
	file.setDetails(instanceQuery.value("details").toString());
	file.setFileId(query.value("fileId").toInt());

	QString comment = "First checkIn for file from getSpecificCopy of dbController test";

	ok = m_dbController->checkIn(file, comment, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	int lastChangesetId = 0;

	ok = m_dbController->lastChangesetId(&lastChangesetId);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(file, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	comment = comment.replace("First", "Second");

	ok = m_dbController->checkIn(file, comment, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	std::shared_ptr<DbFile> result(new DbFile);

	ok = m_dbController->getSpecificCopy(file, lastChangesetId, &result, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM get_specific_copy(1, %1, %2)").arg(fileId).arg(lastChangesetId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("changesetId").toInt() == result.get()->changeset(), qPrintable ("Error: Wrong changeset has been returned"));
	QVERIFY2(query.value("data").toString() == result.get()->data(), qPrintable ("Error: Wrong data in changeset"));


	fileName = "SecondFileForGetSpecificCopyTestOfDbController\\'\"";
	fileForSql = "SecondFileForGetSpecificCopyTestOfDbController\\''\"\"";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	secondFile.setFileName(fileName);
	secondFile.setSize(instanceQuery.value("Size").toInt());
	secondFile.setUserId(1);
	secondFile.setParentId(1);
	secondFile.setDetails(instanceQuery.value("details").toString());
	secondFile.setFileId(query.value("fileId").toInt());

	comment = "Checking in two files for getSpecificCopy of dbController test";

	ok = m_dbController->checkOut(file, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	std::vector<DbFileInfo> files;

	files.push_back(file);
	files.push_back(secondFile);

	ok = m_dbController->checkIn(files, comment, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->lastChangesetId(&lastChangesetId);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(secondFile, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	comment = "Just for test check out second file in getSpecificCopy of dbController test";

	ok = m_dbController->checkIn(secondFile, comment, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	std::vector<std::shared_ptr<DbFile>> out;

	ok = m_dbController->getSpecificCopy(files, lastChangesetId, &out, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	for (std::shared_ptr<DbFile> buff : out)
	{
		ok = query.exec(QString("SELECT * FROM get_specific_copy(1, %2, %3)").arg(buff.get()->fileId()).arg(lastChangesetId));
		QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));

		QVERIFY2(query.value("changesetId").toInt() == buff.get()->changeset(), qPrintable ("Error: Wrong changeset has been returned"));
		QVERIFY2(query.value("data").toString() == buff.get()->data(), qPrintable ("Error: Wrong data in changeset"));
	}

	db.close();
}

void DbControllerFileTests::checkInTreeTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	DbFileInfo firstParentFile, firstChildFile, secondParentFile, secondChildFile;
	QSqlQuery query, instanceQuery;

	QString fileName = "FirstParentForCheckInTreeTestOfDbController\\'\"";
	QString fileForSql = "FirstParentForCheckInTreeTestOfDbController\\''\"\"";

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	firstParentFile.setFileName(fileName);
	firstParentFile.setSize(instanceQuery.value("Size").toInt());
	firstParentFile.setUserId(1);
	firstParentFile.setParentId(1);
	firstParentFile.setDetails(instanceQuery.value("details").toString());
	firstParentFile.setFileId(query.value("fileId").toInt());

	fileName = "FirstChildForCheckInTreeTestOfDbController\\'\"";
	fileForSql = "FirstChildForCheckInTreeTestOfDbController\\''\"\"";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', %2, 'LOL', '{}')").arg(fileForSql).arg(firstParentFile.fileId()));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	firstChildFile.setFileName(fileName);
	firstChildFile.setSize(instanceQuery.value("Size").toInt());
	firstChildFile.setUserId(1);
	firstChildFile.setParentId(1);
	firstChildFile.setDetails(instanceQuery.value("details").toString());
	firstChildFile.setFileId(query.value("fileId").toInt());

	fileName = "SecondParentForCheckInTreeTestOfDbController\\'\"";
	fileForSql = "SecondParentForCheckInTreeTestOfDbController\\''\"\"";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	secondParentFile.setFileName(fileName);
	secondParentFile.setSize(instanceQuery.value("Size").toInt());
	secondParentFile.setUserId(1);
	secondParentFile.setParentId(1);
	secondParentFile.setDetails(instanceQuery.value("details").toString());
	secondParentFile.setFileId(query.value("fileId").toInt());

	fileName = "SecondChildForCheckInTreeTestOfDbController";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', %2, 'LOL', '{}')").arg(fileName).arg(secondParentFile.fileId()));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	secondChildFile.setFileName(fileName);
	secondChildFile.setSize(instanceQuery.value("Size").toInt());
	secondChildFile.setUserId(1);
	secondChildFile.setParentId(1);
	secondChildFile.setDetails(instanceQuery.value("details").toString());
	secondChildFile.setFileId(query.value("fileId").toInt());

	std::vector<DbFileInfo> files;

	files.push_back(firstParentFile);
	files.push_back(secondParentFile);

	std::vector<DbFileInfo> out;

	QString comment = "Check for checkInTree";

	ok = m_dbController->checkInTree(files, &out, comment, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	for (DbFileInfo buff : out)
	{
		ok = query.exec(QString("SELECT * FROM get_file_state(%1)").arg(buff.fileId()));
		QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));

		QVERIFY2 (query.value("checkedout").toBool() == false, qPrintable ("Error: file was not checked in"));
	}
}

void DbControllerFileTests::undoChangestest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	DbFileInfo file, secondFile;

	QSqlQuery query, instanceQuery;

	QString fileName = "FirstFileForUndoChangesTestOfDbController\\'\"";
	QString fileForSql = "FirstFileForUndoChangesTestOfDbController\\''\"\"";

	bool ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	int fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	file.setFileName(fileName);
	file.setSize(instanceQuery.value("Size").toInt());
	file.setUserId(1);
	file.setParentId(1);
	file.setDetails(instanceQuery.value("details").toString());
	file.setFileId(query.value("fileId").toInt());

	QString comment = "Wow, checkIn!";

	ok = m_dbController->checkIn(file, comment, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->checkOut(file, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->undoChanges(file, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM is_file_checkedout(%1)").arg(file.fileId()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: changes in checkedOut file was not been removed"));

	ok = m_dbController->checkOut(file, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	fileName = "SecondFileForUndoChangesTestOfDbController\\'\"";
	fileForSql = "SecondFileForUndoChangesTestOfDbController\\''\"\"";

	ok = query.exec(QString("SELECT * FROM add_file(1, '%1', 1, 'LOL', '{}')").arg(fileForSql));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	fileId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM file WHERE fileId=%1").arg(fileId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = instanceQuery.exec(QString("SELECT * FROM fileInstance WHERE fileInstanceid = '%1'").arg(query.value("checkedOutInstanceId").toString()));
	QVERIFY2(ok == true, qPrintable(instanceQuery.lastError().databaseText()));
	QVERIFY2(instanceQuery.first() == true, qPrintable(instanceQuery.lastError().databaseText()));

	secondFile.setFileName(fileName);
	secondFile.setSize(instanceQuery.value("Size").toInt());
	secondFile.setUserId(1);
	secondFile.setParentId(1);
	secondFile.setDetails(instanceQuery.value("details").toString());
	secondFile.setFileId(query.value("fileId").toInt());

	std::vector<DbFileInfo> files;
	files.push_back(file);
	files.push_back(secondFile);

	ok = m_dbController->undoChanges(files, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT * FROM file_exists(%1)").arg(secondFile.fileId()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: file was not removed by function"));

	ok = query.exec(QString("SELECT * FROM is_file_checkedout(%1)").arg(file.fileId()));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: file was not checked in by function"));
}

void DbControllerFileTests::systemFilesTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable(db.lastError().databaseText()));

	QSqlQuery query;

	int afbl = 0;
	int al = 0;
	int mc = 0;
	int mvs = 0;
	int dvs = 0;
	int hc = 0;
	int hp = 0;

	bool ok = query.exec("SELECT * FROM file WHERE name = 'AFBL'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));

	afbl = query.value("fileId").toInt();

	ok = query.exec("SELECT * FROM file WHERE name = 'AL'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));

	al = query.value("fileId").toInt();

	ok = query.exec("SELECT * FROM file WHERE name = 'MC'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));

	mc = query.value("fileId").toInt();

	ok = query.exec("SELECT * FROM file WHERE name = 'MVS'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));

	mvs = query.value("fileId").toInt();

	ok = query.exec("SELECT * FROM file WHERE name = 'DVS'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));

	dvs = query.value("fileId").toInt();

	ok = query.exec("SELECT * FROM file WHERE name = 'HC'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));

	hc = query.value("fileId").toInt();

	ok = query.exec("SELECT * FROM file WHERE name = 'HP'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));

	hp = query.value("fileId").toInt();

	QVERIFY2 (m_dbController->rootFileId() == 0, qPrintable("Error: root fileId must be 0, function returned wrong id"));
	QVERIFY2 (m_dbController->afblFileId() == afbl, qPrintable("Error: Wrong AFBL id returned"));
	QVERIFY2 (m_dbController->alFileId() == al, qPrintable("Error: Wrong AL id returned"));
	QVERIFY2 (m_dbController->mcFileId() == mc, qPrintable("Error: Wrong MC id returned"));
	QVERIFY2 (m_dbController->mvsFileId() == mvs, qPrintable("Error: Wrong MVS id returned"));
	QVERIFY2 (m_dbController->dvsFileId() == dvs, qPrintable("Error: Wrong DVS id returned"));
	QVERIFY2 (m_dbController->hcFileId() == hc, qPrintable("Error: Wrong HC id returned"));
	QVERIFY2 (m_dbController->hpFileId() == hp, qPrintable("Error: Wrong HP id returned"));

	std::vector<DbFileInfo> systemFiles;
	QVector<int> fileIds;

	systemFiles = m_dbController->systemFiles();

	for (DbFileInfo file : systemFiles)
	{
		fileIds.push_back(file.fileId());
	}

	QVERIFY2 (fileIds.contains(0) == true, qPrintable("Error: systemFiles function has not added root file to output!"));
	QVERIFY2 (fileIds.contains(afbl) == true, qPrintable("Error: systemFiles function has not added AFBL file to output!"));
	QVERIFY2 (fileIds.contains(al) == true, qPrintable("Error: systemFiles function has not added AL file to output!"));
	QVERIFY2 (fileIds.contains(mc) == true, qPrintable("Error: systemFiles function has not added MC file to output!"));
	QVERIFY2 (fileIds.contains(mvs) == true, qPrintable("Error: systemFiles function has not added MVS file to output!"));
	QVERIFY2 (fileIds.contains(dvs) == true, qPrintable("Error: systemFiles function has not added DVS file to output!"));
	QVERIFY2 (fileIds.contains(hc) == true, qPrintable("Error: systemFiles function has not added HC file to output!"));
	QVERIFY2 (fileIds.contains(hp) == true, qPrintable("Error: systemFiles function has not added HP file to output!"));

	QVERIFY2 (m_dbController->systemFileInfo("AFBL").fileId() == afbl, qPrintable("Error: function systemFileInfo returned wrong fileId"));
	QVERIFY2 (m_dbController->systemFileInfo(hc).fileName() == "HC", qPrintable("Error: function systemFileInfo returned wrong fileName"));

	ok = query.exec("SELECT * FROM add_file(1, 'systemFileName', 1, 'file_data', '{}')");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (m_dbController->systemFileInfo(query.value("id").toInt()).fileId() == -1, qPrintable("Error: -1 fileId expected"));

	db.close();
}

void DbControllerFileTests::cleanupTestCase()
{
	for (QString connection : QSqlDatabase::connectionNames())
	{
		QSqlDatabase::removeDatabase(connection);
	}

	m_dbController->deleteProject(m_databaseName, m_adminPassword, true, 0);
}

