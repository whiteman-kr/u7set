#include "DbControllerProjectTests.h"
#include <QSql>
#include <QSqlError>

DbControllerProjectTests::DbControllerProjectTests()
{
	m_db.disableProgress();
	m_db.setHost(m_databaseHost);
	m_db.setPort(m_databaseHostPort);
	m_db.setServerPassword(m_databaseUserPassword);
	m_db.setServerUsername(m_databaseUser);
}

void DbControllerProjectTests::setProjectVersion(int version)
{
	m_databaseVersion = version;
}

void DbControllerProjectTests::initTestCase()
{
	dropProjectDb();
}

void DbControllerProjectTests::cleanupTestCase()
{
	dropProjectDb();
}

void DbControllerProjectTests::createOpenUpgradeCloseDeleteProject()
{
	// Create, open, close, and delete simple database
	//
	bool ok = m_db.createProject(m_projectName, m_projectAdministratorPassword, nullptr);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	// Upgrade project
	//
	ok = m_db.upgradeProject(m_projectName, m_projectAdministratorPassword, true, nullptr);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	ok = m_db.openProject(m_projectName, "Administrator", m_projectAdministratorPassword, nullptr);
	QVERIFY2 (m_db.currentProject().databaseName() == qPrintable("u7_" + m_projectName), qPrintable("Error: openProject() function is not opened project"));

	QVERIFY2 (m_db.databaseVersion() == m_databaseVersion, qPrintable(QString("Wrong database version. Actual: %1, Expected: %2 ").arg(m_db.databaseVersion()).arg(m_databaseVersion)));

	// Try open project twice
	//
	ok = m_db.openProject(m_projectName, "Administrator", m_projectAdministratorPassword, nullptr);
	QVERIFY2 (ok == false, qPrintable("Error: Project already opened error exected"));

	ok = m_db.closeProject(0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	// Try open project with invalid user
	//
	ok = m_db.openProject(m_projectName, "TEST", m_projectAdministratorPassword, nullptr);
	QVERIFY2 (ok == false, qPrintable("Error: wrong user error expected"));

	// Try open project with invalid password
	//
	ok = m_db.openProject(m_projectName, "Administrator", "testtesttest", nullptr);
	QVERIFY2 (ok == false, qPrintable("Error: wrong pass error expected"));

	// Delete project
	//
	ok = m_db.deleteProject (m_projectName, m_projectAdministratorPassword, true, nullptr);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	return;
}

void DbControllerProjectTests::getProjectListTest()
{
	bool ok = m_db.createProject(m_projectName, m_projectAdministratorPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	ok = m_db.upgradeProject(m_projectName, m_projectAdministratorPassword, false, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	// Get project list from db directly
	//
	QVector<QString> databasesFromServer;

	{
		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "GetListConnection");

		db.setHostName(m_databaseHost);
		db.setPort(m_databaseHostPort);
		db.setUserName(m_databaseUser);
		db.setPassword(m_databaseUserPassword);
		db.setDatabaseName("postgres");

		bool ok = db.open();
		QVERIFY2(ok == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

		QSqlQuery query(db);
		ok = query.exec("SELECT datname FROM pg_database WHERE datname LIKE 'u7_%' AND NOT datname LIKE 'u7u%'");
		QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

		while (query.next() == true)
		{
			databasesFromServer.push_back(query.value(0).toString());
		}

		db.close();
	}

	QSqlDatabase::removeDatabase("GetListConnection");

	// Get project list from db controller
	//
	ok = m_db.openProject(m_projectName, "Administrator", m_projectAdministratorPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	std::vector<DbProject> outputData;
	ok = m_db.getProjectList(&outputData, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	for (DbProject projectFromDb : outputData)
	{
		QVERIFY2(databasesFromServer.contains(projectFromDb.databaseName()), qPrintable("Error: function getProjectList() returned wrong database names!"));
	}

	ok = m_db.closeProject(0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	ok = m_db.deleteProject (m_projectName, m_projectAdministratorPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	return;
}

void DbControllerProjectTests::ProjectPropertyTest()
{
	bool ok = m_db.createProject(m_projectName, m_projectAdministratorPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	ok = m_db.upgradeProject(m_projectName, m_projectAdministratorPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	ok = m_db.openProject(m_projectName, "Administrator", m_projectAdministratorPassword, 0);
	QVERIFY2 (m_db.currentProject().databaseName() == qPrintable("u7_" + m_projectName), qPrintable("Error: openProject() function is not opened project"));

	QString testPropertyName = "TestPropertyName";
	QString testPropertyValue = "TestPropertyValue";

	ok = m_db.setProjectProperty(testPropertyName, testPropertyValue, 0);
	QVERIFY2(ok == true, qPrintable(m_db.lastError()));

	// --
	//
	{
		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "TestPropsConnection");

		db.setHostName(m_databaseHost);
		db.setPort(m_databaseHostPort);
		db.setUserName(m_databaseUser);
		db.setPassword(m_databaseUserPassword);
		db.setDatabaseName("u7_" + m_projectName);

		ok = db.open();
		QVERIFY2(ok == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

		QSqlQuery query(db);
		ok = query.exec(QString("SELECT COUNT(*) FROM ProjectProperties WHERE name = '%1' AND value = '%2'").arg(testPropertyName).arg(testPropertyValue));

		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.value(0).toInt() == 1, qPrintable("Error: project property has not been written to database"));

		db.close();
	}
	QSqlDatabase::removeDatabase("TestPropsConnection");

	// --
	//
	QString propertyValueResult;

	ok = m_db.getProjectProperty(testPropertyName, &propertyValueResult, 0);
	QVERIFY2(ok == true, qPrintable(m_db.lastError()));

	QVERIFY2(propertyValueResult == testPropertyValue, qPrintable("Error: wrong property value returned!"));

	ok = m_db.closeProject(0);
	QVERIFY2 (ok == true, qPrintable("Error: can not close project"));

	ok = m_db.deleteProject (m_projectName, m_projectAdministratorPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	return;
}

void DbControllerProjectTests::isProjectOpenedTest()
{
	bool ok = m_db.isProjectOpened();

	QVERIFY2 (ok == false, qPrintable("Error: function returns, that project is opened, when it is NOT opened"));

	ok = m_db.createProject(m_projectName, m_projectAdministratorPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	ok = m_db.upgradeProject(m_projectName, m_projectAdministratorPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	ok = m_db.openProject(m_projectName, "Administrator", m_projectAdministratorPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	ok = m_db.isProjectOpened();
	QVERIFY2 (ok == true, qPrintable("Error: function returns, that project is not opened, when it is opened"));

	ok = m_db.closeProject(0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	ok = m_db.deleteProject (m_projectName, m_projectAdministratorPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	return;
}

void DbControllerProjectTests::connectionInfoTest()
{
	bool ok = m_db.createProject(m_projectName, m_projectAdministratorPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	ok = m_db.upgradeProject(m_projectName, m_projectAdministratorPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	ok = m_db.openProject(m_projectName, "Administrator", m_projectAdministratorPassword, 0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	// Host testing
	//
	QVERIFY2(m_db.host() == m_databaseHost, qPrintable("Error: wrong host returned"));
	m_db.setHost("0.0.0.0");
	QVERIFY2(m_db.host() == "0.0.0.0", qPrintable("Error: wrong host returned"));
	m_db.setHost(m_databaseHost);

	// Port testing
	//
	QVERIFY2 (m_db.port() == m_databaseHostPort, qPrintable("Error: wrong port returned"));
	m_db.setPort(1234);
	QVERIFY2 (m_db.port() == 1234, qPrintable("Error: wrong port returned"));
	m_db.setPort(m_databaseHostPort);

	// Server username testing
	//
	QVERIFY2(m_db.serverUsername() == m_databaseUser, qPrintable("Error: wrong server username returned"));
	m_db.setServerUsername("Tester");
	QVERIFY2(m_db.serverUsername() == "Tester", qPrintable("Error: wrong server username returned"));
	m_db.setServerUsername(m_databaseUser);

	// Server pass test
	//
	QVERIFY2(m_db.serverPassword() == m_projectAdministratorPassword, qPrintable("Error: empty pass expected"));
	m_db.setServerPassword("Test");
	QVERIFY2 (m_db.serverPassword() == "Test", qPrintable("Error: \"Test\" pass expected"));
	m_db.setServerPassword(m_projectAdministratorPassword);

	// Current user testing
	//
	QVERIFY2 (m_db.currentUser().username() == "Administrator", qPrintable("Error: wrong current user returned"));

	ok = m_db.closeProject(0);
	QVERIFY2 (ok == true, qPrintable(m_db.lastError()));

	return;
}
