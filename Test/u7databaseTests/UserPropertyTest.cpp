#include "UserPropertyTest.h"
#include <QtSql>
#include <QString>
#include <QDebug>

UserPropertyTests::UserPropertyTests()
{

}

void UserPropertyTests::initTestCase()
{
	bool ok = createProjectDb();
	QVERIFY2(ok == true, "Cannot create projectdatabase");
}

void UserPropertyTests::cleanupTestCase()
{
	dropProjectDb();
}

void UserPropertyTests::set_user_property()
{
	// LogIn as Admin
	//
	QString session_key = logIn(m_projectAdministratorName, m_projectAdministratorPassword);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT api.set_user_property('%1', 'AdminProperty1', 'AdminProperty1Value1');").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error creating property with right parameters. True (sucsessful result) expected"));

	ok = query.exec("SELECT * FROM UserProperties WHERE UserID = 1 AND name = 'AdminProperty1' AND value = 'AdminProperty1Value1';");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT api.set_user_property('%1', 'AdminProperty1', 'AdminProperty1Value2');").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error creating property with right parameters. True (sucsessful result) expected"));

	ok = query.exec("SELECT COUNT(*) FROM UserProperties WHERE UserID = 1 AND name = 'AdminProperty1' AND value = 'AdminProperty1Value1';");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toInt() == 0, qPrintable("Error: Function set_user_property() do not updated (delete old) the record!"));

	ok = query.exec("SELECT COUNT(*) FROM UserProperties WHERE UserID = 1 AND name = 'AdminProperty1' AND value = 'AdminProperty1Value2';");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toInt() == 1, qPrintable("Error: Function set_user_property() do not updated the record!"));

	ok = query.exec(QString("SELECT api.set_user_property('%1', '', '');").arg(session_key));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: false expected. Can not create property with NULL name"));

	// LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
}

void UserPropertyTests::set_user_property_check_user_influence()
{
	// 1. LogIn as Admin
	// 2. Create new user
	//
	QString session_key = logIn(m_projectAdministratorName, m_projectAdministratorPassword);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	int serhiyUserId = createUser(session_key, "Serhiy", "P2ssw0rd");

	// 1. Create a property as Admin
	// 2. LoginIn as Serhiy and create a property with the same name
	// 3. Check that Admin's and Serhiy's property have right values
	//

	QSqlQuery query;
	bool ok = false;

	// 1.
	//
	ok = query.exec(QString("SELECT api.set_user_property('%1', 'Property1', 'AdminCreated');").arg(session_key));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// 2.
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	session_key = logIn("Serhiy", "P2ssw0rd");
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	ok = query.exec(QString("SELECT api.set_user_property('%1', 'Property1', 'SerhiyCreated');").arg(session_key));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// 3.
	//
	ok = query.exec(QString("SELECT * FROM UserProperties WHERE UserID = %1 AND name = 'Property1' AND value = 'SerhiyCreated';").arg(serhiyUserId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	session_key = logIn(m_projectAdministratorName, m_projectAdministratorPassword);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	ok = query.exec("SELECT * FROM UserProperties WHERE UserID = 1 AND name = 'Property1' AND value = 'AdminCreated';");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	// LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
}

void UserPropertyTests::get_user_property()
{
	// LogIn as Admin
	//
	QString session_key = logIn(m_projectAdministratorName, m_projectAdministratorPassword);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT api.set_user_property('%1', 'getUserPropertyTest', 'justValue');").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM api.get_user_property('%1', 'getUserPropertyTest');").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toString() == "justValue", qPrintable("Error: get_user_property() returned wrong value!"));

	ok = query.exec(QString("SELECT * FROM api.get_user_property('%1', 'testErrName');").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toString() == "", qPrintable("Error: no answer expected in case of wrong name"));

	// LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
}
