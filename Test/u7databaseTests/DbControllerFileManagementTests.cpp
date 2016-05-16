#include "DbControllerFileManagementTests.h"
#include <QSql>
#include <QSqlError>
#include <QFile>

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

	QSqlQuery query;
	bool ok = query.exec("SELECT datname FROM pg_database WHERE datname LIKE 'u7_%' AND NOT datname LIKE 'u7u%'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next() == true)
	{
		if (query.value(0).toString() == "u7_" + m_databaseName)
			m_dbController->deleteProject(m_databaseName, m_adminPassword, true, 0);
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

	QVERIFY2 (filesForCheck.size() == files.size(), qPrintable("Error: getFileList() function returned wrong amount of files!"));

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



void DbControllerFileTests::cleanupTestCase()
{
	m_dbController->deleteProject(m_databaseName, m_adminPassword, true, 0);
}

