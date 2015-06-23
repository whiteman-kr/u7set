#pragma once
#include <QtSql>
#include <QString>
#include <QTest>
#include "UserTests.h"

UserTests::UserTests()
{
}

void UserTests::initTestCase()
{
	QSqlQuery query;

	//getUserIDTests data
	query.exec ("SELECT create_user (1, 'TEST', 'TEST', 'TEST', 'TEST', false, false, false);");
	query.first();
	UserTests::m_getUserTempDataID = query.value("create_user").toInt();

	//isAdminTests data
	query.exec ("SELECT create_user (1, 'AdminTest', 'TEST', 'TEST', 'TEST', false, false, false);");
	query.first();
	UserTests::m_isAdminTempDataID = query.value("create_user").toInt();
	query.exec ("SELECT create_user (1, 'AdminTest1', 'TEST', 'TEST', 'TEST', false, false, false);");
	query.first();
	UserTests::m_isAdminTempDataNullID = query.value("create_user").toInt();
	query.exec(("DELETE FROM \"User\" WHERE \"Username\"='AdminTest1'"));
}

void UserTests::cleanupTestCase()
{
	QSqlQuery query;

	//createUserTests litter
	query.exec(("DELETE FROM \"User\" WHERE \"Username\"='TestUser1'"));
	query.exec(("DELETE FROM \"User\" WHERE \"Username\"='TestUser4'"));
	query.exec(("DELETE FROM \"User\" WHERE \"Username\"='TestUser5'"));
	query.exec(("DELETE FROM \"User\" WHERE \"Username\"='TestUser6'"));
	query.exec(("DELETE FROM \"User\" WHERE \"Username\"='TestUser17'"));

	//getUserIDTests litter
	query.exec ("DELETE FROM \"User\" WHERE \"Username\"='TEST';");

	//isAdminTests litter
	query.exec(("DELETE FROM \"User\" WHERE \"Username\"='AdminTest'"));
}

void UserTests::createUserTest_data()
{
	QTest::addColumn<QString>("parentUser");
	QTest::addColumn<QString>("userName");
	QTest::addColumn<QString>("firstName");
	QTest::addColumn<QString>("lastName");
	QTest::addColumn<QString>("password");
	QTest::addColumn<bool>("isAdmin");
	QTest::addColumn<bool>("isReadOnly");
	QTest::addColumn<bool>("isDisabled");
	QTest::addColumn<bool>("Result");

	QTest::newRow("createUserParentAdmin") << "1" << "TestUser1" << "Jack" << "Toaster" << "qwerty" << false << false << false << true;
	QTest::newRow("createUserParentUser") << "2" << "TestUser2" << "Jack" << "Toaster" << "qwerty" << false << false << false << false;
	QTest::newRow("createAdministratorParentUser") << "2" << "TestUser3" << "Jack" << "Toaster" << "qwerty" << true << false << false << false;
	QTest::newRow("createAdministratorParentAdministrator") << "1" << "TestUser4" << "Jack" << "Toaster" << "qwerty" << true << false << false << true;
	QTest::newRow("createUserAlreadyExists") << "1" << "TestUser1" << "Jack" << "Toaster" << "qwerty" << true << false << false << false;
	QTest::newRow("createDisabledUser") << "1" << "TestUser5" << "Jack" << "Toaster" << "qwerty" << false << false << true << true;
	QTest::newRow("createReadOnlyUser") << "1" << "TestUser6" << "Jack" << "Toaster" << "qwerty" << false << true << false << true;
	QTest::newRow("createUserRussianSymbols") << "1" << "TestUser17" << "Тестер" << "Тестеров" << "пароль" << false << false << false << true;
}

void UserTests::createUserTest()
{
	QFETCH (QString, parentUser);
	QFETCH (QString, userName);
	QFETCH (QString, firstName);
	QFETCH (QString, lastName);
	QFETCH (QString, password);
	QFETCH (bool, isAdmin);
	QFETCH (bool, isReadOnly);
	QFETCH (bool, isDisabled);
	QFETCH (bool, Result);

	QCOMPARE (UserTests::createUserTest(parentUser, userName, firstName, lastName, password, isAdmin, isReadOnly, isDisabled), Result);
}

void UserTests::getUserIDTest_data()
{
	QTest::addColumn<QString>("login");
	QTest::addColumn<QString>("password");
	QTest::addColumn<int>("result");

	QTest::newRow("TestTrueLoginPassword") << "TEST" << "TEST" << UserTests::m_getUserTempDataID;
	QTest::newRow("TestTrueLoginFalsePassword") << "TEST" << "abc" << 0;
	QTest::newRow("TestFalseLoginTruePassword") << "def" << "TEST" << 0;
	QTest::newRow("TestAllFalse") << "Test" << "Test" << 0;
}

void UserTests::getUserIDTest()
{
	QFETCH (QString, login);
	QFETCH (QString, password);
	QFETCH (int, result);

	QCOMPARE (UserTests::getUserIdTest(login, password), result);
}

void UserTests::isAdminTest_data()
{
	QTest::addColumn<int>("userID");
	QTest::addColumn<bool>("result");

	QTest::newRow("AdminTest") << 1 << true;
	QTest::newRow("NoAdminTest") << UserTests::m_isAdminTempDataID << false;
	QTest::newRow("NullUserTest") << UserTests::m_isAdminTempDataNullID << false;
}

void UserTests::isAdminTest()
{
	QFETCH (int, userID);
	QFETCH (bool, result);

	QCOMPARE (UserTests::isAdmin(userID), result);
}



bool UserTests::createUserTest(const QString& parentUser, const QString& userName, const QString& firstName, const QString& lastName, const QString& password, bool isAdmin, bool isReadOnly, bool isDisabled)
{
	QSqlQuery queryCreateUser;
	bool Error = false;

	QString administrator = isAdmin ? "true" : "false";
	QString readOnly = isReadOnly ? "true" : "false";
	QString disabled = isDisabled ? "true" : "false";

	bool ok = queryCreateUser.exec(QString("SELECT create_user(%1, '%2', '%3', '%4', '%5', %6, %7, %8);")
								   .arg(parentUser)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(administrator)
								   .arg(readOnly)
								   .arg(disabled));

	if (ok == false)
	{
		//qDebug() << queryCreateUser.lastError().databaseText();
		return false;
	}

	QSqlQuery queryCheckUser;

	ok = queryCheckUser.exec(("SELECT * FROM \"User\" WHERE \"Username\"='" + userName + "'"));

	if (ok == false)
	{
		qDebug() << queryCheckUser.lastError().databaseText();
		return false;
	}

	ok = queryCheckUser.first();

	if (ok == false)
	{
		qDebug() << "Cannot get first record";
		return false;
	}

	QString dbUserName = queryCheckUser.value("Username").toString();
	QString dbFirstName = queryCheckUser.value("FirstName").toString();
	QString dbLastName = queryCheckUser.value("LastName").toString();
	QString dbPassword = queryCheckUser.value("Password").toString();
	bool dbAdministrator = queryCheckUser.value("Administrator").toBool();
	bool dbReadOnly = queryCheckUser.value("ReadOnly").toBool();
	bool dbDisabled = queryCheckUser.value("Disabled").toBool();

	if (dbUserName != userName)
	{
		qDebug() << "ERROR in column Username\nActual: " << dbUserName << "\nExpected: " << userName;
		Error = true;
	}

	if (dbFirstName != firstName)
	{
		qDebug() << "ERROR in column FirstName\nActual: " << dbFirstName << "\nExpected: " << firstName;
		Error = true;
	}

	if (dbLastName != lastName)
	{
		qDebug() << "ERROR in column LastName\nActual: " << dbLastName << "\nExpected: " << lastName;
		Error = true;
	}

	if (dbPassword != password)
	{
		qDebug() << "ERROR in column Password\nActual: " << dbPassword << "\nExpected: " << password;
		Error = true;
	}

	if (dbAdministrator != isAdmin)
	{
		qDebug() << "ERROR in column Administrator\nActual: " << dbAdministrator << "\nExpected: " << isAdmin;
		Error = true;
	}

	if (dbReadOnly != isReadOnly)
	{
		qDebug() << "ERROR in column ReadOnly\nActual: " << dbReadOnly << "\nExpected: " << isReadOnly;
		Error = true;
	}

	if (dbDisabled != isDisabled)
	{
		qDebug() << "ERROR in column Disabled\nActual: " << dbDisabled << "\nExpected: " << isDisabled;
		Error = true;
	}

	return Error ? "ERROR" : "OK";
}

int UserTests::getUserIdTest(const QString& login, const QString& password)
{
	QSqlQuery query;
	if (query.exec(("SELECT \"GetUserID\"('" + login + "', '" + password + "')")) != true)
	{
		qDebug() << query.lastError().text();
		return -1;
	}
	query.first();
	return query.value("GetUserID").toInt();
}

bool UserTests::isAdmin(int userID)
{
	QSqlQuery query;

	bool result = query.exec(QString("SELECT is_admin (%1);").arg(userID));

	if (result == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	query.first();
	return query.value("is_admin").toBool();
}
