#include "DbControllerProjectManagementTests.h"
#include <QSql>
#include <QSqlError>

DbControllerProjectTests::DbControllerProjectTests() :
	m_db(new DbController())
{
	m_db->setServerUsername(m_databaseUser);
	m_db->setServerPassword(m_adminPassword);

	return;
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

	QVERIFY2(db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

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
	bool ok = m_db->createProject(m_databaseName, m_adminPassword, nullptr);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	QVERIFY2 (db.open() == true, qPrintable("Error: project has not been created! " + db.lastError().databaseText()));
	db.close();

	// Upgrade project
	//
	ok = m_db->upgradeProject(m_databaseName, m_adminPassword, true, nullptr);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->openProject(m_databaseName, "Administrator", m_adminPassword, nullptr);
	QVERIFY2 (m_db->currentProject().databaseName() == qPrintable("u7_" + m_databaseName), qPrintable("Error: openProject() function is not opened project"));

	QVERIFY2 (m_db->databaseVersion() == m_databaseVersion, qPrintable(QString("Wrong database version. Actual: %1, Expected: %2 ").arg(m_db->databaseVersion()).arg(m_databaseVersion)));

	// Try open project twice
	//

	ok = m_db->openProject(m_databaseName, "Administrator", m_adminPassword, nullptr);
	QVERIFY2 (ok == false, qPrintable("Error: Project already opened error exected"));

	ok = m_db->closeProject(0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	// Try open project with invalid user
	//

	ok = m_db->openProject(m_databaseName, "TEST", m_adminPassword, nullptr);
	QVERIFY2 (ok == false, qPrintable("Error: wrong user error expected"));

	// Try open project with invalid password
	//

	ok = m_db->openProject(m_databaseName, "Administrator", "testtesttest", nullptr);
	QVERIFY2 (ok == false, qPrintable("Error: wrong pass error expected"));

	// Try open project with disabled user
	//

	QVERIFY2 (db.open() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_adminPassword));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QString session_key = query.value(0).toString();

	ok = query.exec (QString("SELECT * FROM user_api.create_user('%1', 'Tester', 'Tester', 'Tester', 'TesterTester', false, true)").arg(session_key));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = m_db->openProject(m_databaseName, "Tester", "TesterTester", nullptr);
	QVERIFY2 (ok == false, qPrintable("Error: Disabled user"));

	db.close();

	ok = m_db->deleteProject (m_databaseName, m_adminPassword, true, nullptr);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	// Do the same things, but now with backup
	//

	ok = m_db->createProject(m_databaseName, m_adminPassword, nullptr);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	QVERIFY2 (db.open() == true, qPrintable("Error: project has not been created! " + db.lastError().databaseText()));
	db.close();

	// Upgrade project
	//

	ok = m_db->upgradeProject(m_databaseName, m_adminPassword, false, nullptr);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->openProject(m_databaseName, "Administrator", m_adminPassword, nullptr);
	QVERIFY2 (m_db->currentProject().databaseName() == qPrintable("u7_" + m_databaseName), qPrintable("Error: openProject() function is not opened project"));

	QVERIFY2 (m_db->databaseVersion() == m_databaseVersion, qPrintable(QString("Wrong database version. Actual: %1, Expected: %2 ").arg(m_db->databaseVersion()).arg(m_databaseVersion)));

	ok = m_db->closeProject(nullptr);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->deleteProject (m_databaseName, m_adminPassword, false, nullptr);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	return;
}

void DbControllerProjectTests::getProjectListTest()
{
	bool ok = m_db->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->upgradeProject(m_databaseName, m_adminPassword, false, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

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

	ok = m_db->getProjectList(&outputData, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	for (DbProject projectFromDb : outputData)
	{
		QVERIFY2(databasesFromServer.contains(projectFromDb.databaseName()), qPrintable("Error: function getProjectList() returned wrong database names!"));
	}

	ok = m_db->closeProject(0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->deleteProject (m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));


	db.close();
}

void DbControllerProjectTests::ProjectPropertyTest()
{
	bool ok = m_db->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->upgradeProject(m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (m_db->currentProject().databaseName() == qPrintable("u7_" + m_databaseName), qPrintable("Error: openProject() function is not opened project"));

	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

	QString testPropertyName = "TestPropertyName";
	QString testPropertyValue = "TestPropertyValue";

	QSqlQuery query;

	ok = m_db->setProjectProperty(testPropertyName, testPropertyValue, 0);
	QVERIFY2(ok == true, qPrintable(m_db->lastError()));

	ok = query.exec(QString("SELECT COUNT(*) FROM projectProperties WHERE name = '%1' AND value = '%2'").arg(testPropertyName).arg(testPropertyValue));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.value(0).toInt() == 1, qPrintable("Error: project property has not been written to database"));

	QString propertyValueResult;

	ok = m_db->getProjectProperty(testPropertyName, &propertyValueResult, 0);
	QVERIFY2(ok == true, qPrintable(m_db->lastError()));

	QVERIFY2(propertyValueResult == testPropertyValue, qPrintable("Error: wrong property value returned!"));

	ok = m_db->closeProject(0);
	QVERIFY2 (ok == true, qPrintable("Error: can not close project"));

	db.close();

	ok = m_db->deleteProject (m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));
}

void DbControllerProjectTests::isProjectOpenedTest()
{
	bool ok = m_db->isProjectOpened();

	QVERIFY2 (ok == false, qPrintable("Error: function returns, that project is opened, when it is NOT opened"));

	ok = m_db->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->upgradeProject(m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->isProjectOpened();

	QVERIFY2 (ok == true, qPrintable("Error: function returns, that project is not opened, when it is opened"));

	ok = m_db->closeProject(0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->deleteProject (m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));
}

void DbControllerProjectTests::connectionInfoTest()
{
	bool ok = m_db->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->upgradeProject(m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = m_db->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	// Host testing
	//

	QVERIFY2 (m_db->host() == m_databaseHost, qPrintable("Error: wrong host returned"));
	m_db->setHost("0.0.0.0");
	QVERIFY2 (m_db->host() == "0.0.0.0", qPrintable("Error: wrong host returned"));
	m_db->setHost(m_databaseHost);

	// Port testing
	//

	QVERIFY2 (m_db->port() == m_databasePort, qPrintable("Error: wrong port returned"));
	m_db->setPort(1234);
	QVERIFY2 (m_db->port() == 1234, qPrintable("Error: wrong port returned"));
	m_db->setPort(m_databasePort);

	// Server username testing
	//

	QVERIFY2 (m_db->serverUsername() == m_databaseUser, qPrintable("Error: wrong server username returned"));
	m_db->setServerUsername("Tester");
	QVERIFY2 (m_db->serverUsername() == "Tester", qPrintable("Error: wrong server username returned"));
	m_db->setServerUsername(m_databaseUser);

	/*qDebug() << "Pass before: " << m_dbController->serverPassword();
	m_dbController->setServerPassword("Test");
	qDebug() << "Pass after: " << m_dbController->serverPassword();*/

	// Server pass test
	//

	QVERIFY2 (m_db->serverPassword() == "", qPrintable("Error: empty pass expected"));
	m_db->setServerPassword("Test");
	QVERIFY2 (m_db->serverPassword() == "Test", qPrintable("Error: \"Test\" pass expected"));
	m_db->setServerPassword("");

	// Current user testing
	//

	QVERIFY2 (m_db->currentUser().username() == "Administrator", qPrintable("Error: wrong current user returned"));

	ok = m_db->closeProject(0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));
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
