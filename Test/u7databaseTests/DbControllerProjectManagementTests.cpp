#include "DbControllerProjectManagementTests.h"
#include <QSql>
#include <QSqlError>

DbControllerProjectTests::DbControllerProjectTests()
{
	m_dbController = new DbController();

	m_databaseHost = "127.0.0.1";
	m_databaseName = "dbcontrollertesting";
	m_databaseUser = "u7";
	m_adminPassword = "P2ssw0rd";
}

void DbControllerProjectTests::setProjectVersion(int version)
{
	m_databaseVersion = version;
}

void DbControllerProjectTests::initTestCase()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("postgres");

	QVERIFY2 (db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

	QSqlQuery query, tempQuery;

	bool ok = query.exec("SELECT datname FROM pg_database WHERE datname LIKE 'u7_%' OR datname LIKE 'u7upgrade%' OR datname LIKE 'u7deleted%'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next() == true)
	{
		if (query.value(0).toString().contains(m_databaseName))
		{
			//m_dbController->deleteProject(m_databaseName, m_adminPassword, true, 0);
			ok = tempQuery.exec(QString("DROP DATABASE %1").arg(query.value(0).toString()));
			QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
			qDebug() << "Project " << query.value(0).toString() << "dropped!";
		}
	}

	db.close();
}

void DbControllerProjectTests::createOpenUpgradeCloseDeleteProject()
{
	QSqlDatabase db = QSqlDatabase::database("qt_sql_default_connection");

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QSqlQuery query;

	// Create, open, close, and delete simple database
	//

	bool ok = m_dbController->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2 (db.open() == true, qPrintable("Error: project has not been created! " + db.lastError().databaseText()));
	db.close();

	// Upgrade project
	//

	ok = m_dbController->upgradeProject(m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (m_dbController->currentProject().databaseName() == qPrintable("u7_" + m_databaseName), qPrintable("Error: openProject() function is not opened project"));

	QVERIFY2 (m_dbController->databaseVersion() == m_databaseVersion, qPrintable(QString("Wrong database version. Actual: %1, Expected: %2 ").arg(m_dbController->databaseVersion()).arg(m_databaseVersion)));

	// Try open project twice
	//

	ok = m_dbController->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (ok == false, qPrintable("Error: Project already opened error exected"));

	ok = m_dbController->closeProject(0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	// Try open project with invalid user
	//

	ok = m_dbController->openProject(m_databaseName, "TEST", m_adminPassword, 0);
	QVERIFY2 (ok == false, qPrintable("Error: wrong user error expected"));

	// Try open project with invalid password
	//

	ok = m_dbController->openProject(m_databaseName, "Administrator", "testtesttest", 0);
	QVERIFY2 (ok == false, qPrintable("Error: wrong pass error expected"));

	// Try open project with disabled user
	//

	QVERIFY2 (db.open() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec ("SELECT * FROM create_user(1, 'Tester', 'Tester', 'Tester', 'TesterTester', false, false, true)");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	ok = m_dbController->openProject(m_databaseName, "Tester", "TesterTester", 0);
	QVERIFY2 (ok == false, qPrintable("Error: Disabled user"));

	db.close();

	ok = m_dbController->deleteProject (m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	// Do the same things, but now with backup
	//

	ok = m_dbController->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2 (db.open() == true, qPrintable("Error: project has not been created! " + db.lastError().databaseText()));
	db.close();

	// Upgrade project
	//

	ok = m_dbController->upgradeProject(m_databaseName, m_adminPassword, false, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (m_dbController->currentProject().databaseName() == qPrintable("u7_" + m_databaseName), qPrintable("Error: openProject() function is not opened project"));

	QVERIFY2 (m_dbController->databaseVersion() == m_databaseVersion, qPrintable(QString("Wrong database version. Actual: %1, Expected: %2 ").arg(m_dbController->databaseVersion()).arg(m_databaseVersion)));

	ok = m_dbController->closeProject(0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->deleteProject (m_databaseName, m_adminPassword, false, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));
}

void DbControllerProjectTests::getProjectListTest()
{
	bool ok = m_dbController->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("postgres");

	std::vector<DbProject> outputData;
	QVector<QString> databasesFromServer;

	QSqlQuery query;

	QVERIFY2 (db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

	ok = query.exec("SELECT datname FROM pg_database WHERE datname LIKE 'u7_%' AND NOT datname LIKE 'u7u%'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next() == true)
	{
		databasesFromServer.push_back(query.value(0).toString());
	}

	ok = m_dbController->getProjectList(&outputData, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	for (DbProject projectFromDb : outputData)
	{
		QVERIFY2(databasesFromServer.contains(projectFromDb.databaseName()), qPrintable("Error: function getProjectList() returned wrong database names!"));
	}

	ok = m_dbController->deleteProject (m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));


	db.close();
}

void DbControllerProjectTests::ProjectPropertyTest()
{
	bool ok = m_dbController->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->upgradeProject(m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (m_dbController->currentProject().databaseName() == qPrintable("u7_" + m_databaseName), qPrintable("Error: openProject() function is not opened project"));

	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

	QString testPropertyName = "TestPropertyName";
	QString testPropertyValue = "TestPropertyValue";

	QSqlQuery query;

	ok = m_dbController->setProjectProperty(testPropertyName, testPropertyValue, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	ok = query.exec(QString("SELECT COUNT(*) FROM projectProperties WHERE name = '%1' AND value = '%2'").arg(testPropertyName).arg(testPropertyValue));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.value(0).toInt() == 1, qPrintable("Error: project property has not been written to database"));

	QString propertyValueResult;

	ok = m_dbController->getProjectProperty(testPropertyName, &propertyValueResult, 0);
	QVERIFY2(ok == true, qPrintable(m_dbController->lastError()));

	QVERIFY2(propertyValueResult == testPropertyValue, qPrintable("Error: wrong property value returned!"));

	ok = m_dbController->closeProject(0);
	QVERIFY2 (ok == true, qPrintable("Error: can not close project"));

	db.close();

	ok = m_dbController->deleteProject (m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));
}

void DbControllerProjectTests::isProjectOpenedTest()
{
	bool ok = m_dbController->isProjectOpened();

	QVERIFY2 (ok == false, qPrintable("Error: function returns, that project is opened, when it is NOT opened"));

	ok = m_dbController->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->upgradeProject(m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->isProjectOpened();

	QVERIFY2 (ok == true, qPrintable("Error: function returns, that project is not opened, when it is opened"));

	ok = m_dbController->closeProject(0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->deleteProject (m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));
}

void DbControllerProjectTests::connectionInfoTest()
{
	bool ok = m_dbController->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->upgradeProject(m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	ok = m_dbController->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));

	// Host testing
	//

	QVERIFY2 (m_dbController->host() == m_databaseHost, qPrintable("Error: wrong host returned"));
	m_dbController->setHost("0.0.0.0");
	QVERIFY2 (m_dbController->host() == "0.0.0.0", qPrintable("Error: wrong host returned"));
	m_dbController->setHost(m_databaseHost);

	// Port testing
	//

	QVERIFY2 (m_dbController->port() == m_databasePort, qPrintable("Error: wrong port returned"));
	m_dbController->setPort(1234);
	QVERIFY2 (m_dbController->port() == 1234, qPrintable("Error: wrong port returned"));
	m_dbController->setPort(m_databasePort);

	// Server username testing
	//

	QVERIFY2 (m_dbController->serverUsername() == m_databaseUser, qPrintable("Error: wrong server username returned"));
	m_dbController->setServerUsername("Tester");
	QVERIFY2 (m_dbController->serverUsername() == "Tester", qPrintable("Error: wrong server username returned"));
	m_dbController->setServerUsername(m_databaseUser);

	/*qDebug() << "Pass before: " << m_dbController->serverPassword();
	m_dbController->setServerPassword("Test");
	qDebug() << "Pass after: " << m_dbController->serverPassword();*/

	// Current user testing
	//

	QVERIFY2 (m_dbController->currentUser().username() == "Administrator", qPrintable("Error: wrong current user returned"));

	ok = m_dbController->closeProject(0);
	QVERIFY2 (ok == true, qPrintable(m_dbController->lastError()));
}

void DbControllerProjectTests::cleanupTestCase()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("postgres");

	QVERIFY2 (db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

	QSqlQuery query, tempQuery;

	bool ok = query.exec("SELECT datname FROM pg_database WHERE datname LIKE 'u7_%' OR datname LIKE 'u7upgrade%' OR datname LIKE 'u7deleted%'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next() == true)
	{
		if (query.value(0).toString().contains(m_databaseName))
		{
			//m_dbController->deleteProject(m_databaseName, m_adminPassword, true, 0);
			ok = tempQuery.exec(QString("DROP DATABASE %1").arg(query.value(0).toString()));
			QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
		}
	}

	db.close();

	for (QString connection : QSqlDatabase::connectionNames())
	{
		QSqlDatabase::removeDatabase(connection);
	}
}
