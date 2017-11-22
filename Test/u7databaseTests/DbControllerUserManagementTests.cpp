#include "DbControllerUserManagementTests.h"
#include <QSql>
#include <QSqlError>

DbControllerUserTests::DbControllerUserTests() :
	m_db(new DbController())
{
}

void DbControllerUserTests::initTestCase()
{
	for (QString connection : QSqlDatabase::connectionNames())
	{
		QSqlDatabase::removeDatabase(connection);
	}

	m_db->setServerUsername(m_databaseUser);
	m_db->setServerPassword(m_adminPassword);

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

	ok = m_db->createProject(m_databaseName, m_adminPassword, nullptr);
	QVERIFY2 (ok == true, qPrintable ("Error: can not create project: " + m_db->lastError()));

	ok = m_db->upgradeProject(m_databaseName, m_adminPassword, true, nullptr);
	QVERIFY2 (ok == true, qPrintable ("Error: can not upgrade project: " + m_db->lastError()));

	ok = m_db->openProject(m_databaseName, "Administrator", m_adminPassword, nullptr);
	QVERIFY2 (ok == true, qPrintable ("Error: can not open project: " + m_db->lastError()));
}

void DbControllerUserTests::createUserTest()
{
	QString userName = "TESTNAME";
	QString firstName = "TESTFIRSTNAME";
	QString lastName = "TESTLASTNAME";
	QString password = "TESTTEST";

	qRegisterMetaType<DbUser>("DbUser");

	DbUser newUser;
	newUser.setUserId(1);
	newUser.setUsername(userName);
	newUser.setFirstName(firstName);
	newUser.setLastName(lastName);
	newUser.setPassword(password);
	newUser.setNewPassword(password);
	newUser.setAdministrator(false);
	newUser.setDisabled(false);
	newUser.setReadonly(false);

	bool ok = m_db->createUser(newUser, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	if (db.open() == false)
	{
		QFAIL(qPrintable(db.lastError().databaseText()));
	}

	QSqlQuery query;
	ok = query.exec(QString("SELECT * from Users WHERE username = \'%1\'").arg(userName));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.first();
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value("username").toString() == userName, qPrintable("Wrong userName after createUser() function of dbController"));
	QVERIFY2 (query.value("firstname").toString() == firstName, qPrintable("Wrong firstName after createUser() function of dbController"));
	QVERIFY2 (query.value("lastname").toString() == lastName, qPrintable("Wrong lastName after createUser() function of dbController"));
	QVERIFY2 (query.value("administrator").toBool() == false, qPrintable("Wrong administrator flag after createUser() function of dbController"));
	QVERIFY2 (query.value("readonly").toBool() == false, qPrintable("Wrong readOnly flag after createUser() function of dbController"));
	QVERIFY2 (query.value("disabled").toBool() == false, qPrintable("Wrong disabled flag after createUser() function of dbController"));


	// Test createUser function with special symbols
	//

	userName = "\'\"\\" + userName + "\'\"\\";
	firstName = "\'\"\\" + firstName + "\'\"\\";
	lastName = "\'\"\\" + lastName + "\'\"\\";
	password = "\'\"\\" + password + "\'\"\\";

	newUser.setUsername(userName);
	newUser.setFirstName(firstName);
	newUser.setLastName(lastName);
	newUser.setPassword(password);
	newUser.setNewPassword(password);

	ok = m_db->createUser(newUser, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = query.exec("SELECT * from Users");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	bool userCreated =  false;

	while (query.next())
	{
		if (query.value("username").toString() == newUser.username())
		{
			QVERIFY2 (query.value("username").toString() == userName, qPrintable("Wrong userName after createUser() function of dbController"));
			QVERIFY2 (query.value("firstname").toString() == firstName, qPrintable("Wrong firstName after createUser() function of dbController"));
			QVERIFY2 (query.value("lastname").toString() == lastName, qPrintable("Wrong lastName after createUser() function of dbController"));
			QVERIFY2 (query.value("administrator").toBool() == false, qPrintable("Wrong administrator flag after createUser() function of dbController"));
			QVERIFY2 (query.value("readonly").toBool() == false, qPrintable("Wrong readOnly flag after createUser() function of dbController"));
			QVERIFY2 (query.value("disabled").toBool() == false, qPrintable("Wrong disabled flag after createUser() function of dbController"));
			userCreated = true;
			continue;
		}
	}

	if (userCreated == false)
	{
		QFAIL ("Error: user with external symbols was not created");
	}

	db.close();
}

void DbControllerUserTests::updateUserTest()
{
	QString userName = "UPDATETESTNAME";
	QString firstName = "UPDATETESTFIRSTNAME";
	QString lastName = "UPDATETESTLASTNAME";
	QString password = "UPDATETESTTEST";

	qRegisterMetaType<DbUser>("DbUser");

	DbUser newUser;
	newUser.setUserId(1);
	newUser.setUsername(userName);
	newUser.setFirstName(firstName);
	newUser.setLastName(lastName);
	newUser.setPassword(password);
	newUser.setNewPassword(password);
	newUser.setAdministrator(false);
	newUser.setDisabled(false);
	newUser.setReadonly(false);

	bool ok = m_db->createUser(newUser, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	firstName = firstName + "UPDATED";
	lastName = lastName + "UPDATED";
	password = password + "UPDATED";
	newUser.setFirstName(firstName);
	newUser.setLastName(lastName);
	newUser.setPassword(password);
	newUser.setNewPassword(password);
	newUser.setAdministrator(true);
	newUser.setDisabled(true);
	newUser.setReadonly(true);

	ok = m_db->updateUser(newUser, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	if (db.open() == false)
	{
		QFAIL(qPrintable(db.lastError().databaseText()));
	}

	QSqlQuery query;
	ok = query.exec(QString("SELECT * from Users WHERE username = \'%1\'").arg(userName));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.first();
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2 (query.value("username").toString() == userName, qPrintable("Wrong userName after createUser() function of dbController"));
	QVERIFY2 (query.value("firstname").toString() == firstName, qPrintable("Wrong firstName after createUser() function of dbController"));
	QVERIFY2 (query.value("lastname").toString() == lastName, qPrintable("Wrong lastName after createUser() function of dbController"));
	QVERIFY2 (query.value("administrator").toBool() == false, qPrintable("Wrong administrator flag after createUser() function of dbController"));
	QVERIFY2 (query.value("readonly").toBool() == true, qPrintable("Wrong readOnly flag after createUser() function of dbController"));
	QVERIFY2 (query.value("disabled").toBool() == true, qPrintable("Wrong disabled flag after createUser() function of dbController"));

	// Testing with special symbols
	//

	userName = "\"\'\\UPDATETESTNAME\"\'\\";
	firstName = "\"\'\\UPDATETESTFIRSTNAME\"\'\\";
	lastName = "\"\'\\UPDATETESTLASTNAME\"\'\\";
	password = "\"\'\\UPDATETESTTEST\"\'\\";
	newUser.setUsername(userName);
	newUser.setFirstName(firstName);
	newUser.setLastName(lastName);
	newUser.setPassword(password);
	newUser.setNewPassword(password);
	newUser.setAdministrator(false);
	newUser.setDisabled(false);
	newUser.setReadonly(false);

	ok = m_db->createUser(newUser, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	firstName = firstName + "UPDATED";
	lastName = lastName + "UPDATED";
	password = password + "UPDATED";
	newUser.setFirstName(firstName);
	newUser.setLastName(lastName);
	newUser.setPassword(password);
	newUser.setNewPassword(password);
	newUser.setAdministrator(true);
	newUser.setDisabled(true);
	newUser.setReadonly(true);

	ok = m_db->updateUser(newUser, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	ok = query.exec("SELECT * from Users");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	bool userUpdated = false;

	while (query.next())
	{
		if (query.value("username").toString() == newUser.username())
		{
			QVERIFY2 (query.value("username").toString() == userName, qPrintable(QString("Wrong userName after updateUser() function of dbController Actual: %1\nExpected: %2").arg(query.value("username").toString()).arg(userName)));
			QVERIFY2 (query.value("firstname").toString() == firstName, qPrintable(QString("Wrong firstName after updateUser() function of dbController \nActual: %1\nExpected: %2").arg(query.value("firstname").toString()).arg(firstName)));
			QVERIFY2 (query.value("lastname").toString() == lastName, qPrintable(QString("Wrong lastName after updateUser() function of dbController \nActual: %1\nExpected: %2").arg(query.value("lastname").toString()).arg(lastName)));
			QVERIFY2 (query.value("administrator").toBool() == false, qPrintable(QString("Wrong administrator flag after updateUser() function of dbController \nActual: %1\nExpected: %2").arg(query.value("administrator").toBool()).arg("true")));
			QVERIFY2 (query.value("readonly").toBool() == true, qPrintable(QString("Wrong readOnly flag after updateUser() function of dbController \nActual: %1\nExpected: %2").arg(query.value("readonly").toBool()).arg("true")));
			QVERIFY2 (query.value("disabled").toBool() == true, qPrintable(QString("Wrong disabled flag after updateUser() function of dbController \nActual: %1\nExpected: %2").arg(query.value("disabled").toBool()).arg("true")));
			userUpdated = true;
			continue;
		}
	}

	if (userUpdated == false)
	{
		QFAIL("Error: user with external symbols was not updated");
	}

	db.close();
}

void DbControllerUserTests::getUserListTest()
{
	std::vector<DbUser> usersList;

	bool ok = m_db->getUserList(&usersList, 0);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	if (db.open() == false)
	{
		QFAIL(qPrintable(db.lastError().databaseText()));
	}

	QSqlQuery query;
	ok = query.exec("SELECT * from Users");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	QVector<DbUser> usersFromQuery;

	while (query.next())
	{
		DbUser buff;
		buff.setUsername(query.value("username").toString());
		buff.setFirstName(query.value("firstName").toString());
		buff.setLastName(query.value("lastName").toString());
		buff.setAdministrator(query.value("administrator").toBool());
		buff.setDisabled(query.value("disabled").toBool());
		buff.setReadonly(query.value("readonly").toBool());
		buff.setDate(query.value("date").toString());

		usersFromQuery.push_back(buff);
	}

	QVERIFY2 (uint(usersFromQuery.size()) == usersList.size(), qPrintable("Error: function getUsersList returned wrong amount of users"));

	bool userExist = false;

	for (DbUser buff : usersList)
	{
		for (DbUser buffQuery : usersFromQuery)
		{
			if (buff.username() == buffQuery.username())
			{
				QVERIFY2 (buff.firstName() == buffQuery.firstName(), qPrintable("Error: function getUserList returned wrong output (wrong firstName)"));
				QVERIFY2 (buff.lastName() == buffQuery.lastName(), qPrintable("Error: function getUserList returned wrong output (wrong lastName)"));
				QVERIFY2 (buff.date().toString() == buffQuery.date().toString(), qPrintable("Error: function getUserList returned wrong output (wrong date)"));
				QVERIFY2 (buff.isAdminstrator() == buffQuery.isAdminstrator(), qPrintable("Error: function getUserList returned wrong output (wrong administrator flag)"));
				QVERIFY2 (buff.isDisabled() == buffQuery.isDisabled(), qPrintable("Error: function getUserList returned wrong output (wrong disabled flag)"));
				QVERIFY2 (buff.isReadonly() == buffQuery.isReadonly(), qPrintable("Error: function getUserList returned wrong output (wrong readonly flag)"));
				userExist = true;
			}
		}

		QVERIFY2 (userExist == true, qPrintable("Error: no such user. Function getUserList returned wrong output"));
		userExist = false;
	}

	db.close();
}

void DbControllerUserTests::cleanupTestCase()
{
	bool ok = m_db->closeProject(0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not close project: " + m_db->lastError()));

	ok = m_db->deleteProject(m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not delete project: " + m_db->lastError()));

	for (QString connection : QSqlDatabase::connectionNames())
	{
		QSqlDatabase::removeDatabase(connection);
	}

	return;
}
