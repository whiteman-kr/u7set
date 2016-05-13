#include "DbControllerFileManagementTests.h"
#include <QSql>
#include <QSqlError>

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

	QVERIFY2 (db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

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
}



void DbControllerFileTests::cleanupTestCase()
{
	m_dbController->deleteProject(m_databaseName, m_adminPassword, true, 0);
}

