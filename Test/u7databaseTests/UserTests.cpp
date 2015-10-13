#include <QtSql>
#include <QString>
#include <QTest>
#include "UserTests.h"



UserTests::UserTests()
{
}

void UserTests::initTestCase()
{
}

void UserTests::cleanupTestCase()
{
}

void UserTests::createUserTest()
{	
	QSqlQuery query;
	QSqlQuery tempQuery;

	// Create user parent Admin
	//

	QString parentUser = "1";
	QString userName = "TestUser1";
	QString firstName = "Jack";
	QString lastName = "Toaster";
	QString password = "qwerty";
	QString administrator = "false";
	QString readOnly = "false";
	QString disabled = "false";

	bool ok = query.exec(QString("SELECT create_user(%1, '%2', '%3', '%4', '%5', %6, %7, %8);")
								   .arg(parentUser)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(administrator)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(("SELECT * FROM users WHERE username='" + userName + "'"));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(tempQuery.value("username").toString() == userName, qPrintable("ERROR in column userName: data not match! (Create user parent Admin test)"));
	QVERIFY2(tempQuery.value("firstName").toString() == firstName, qPrintable("ERROR in column firstName: data not match! (Create user parent Admin test)"));
	QVERIFY2(tempQuery.value("lastName").toString() == lastName, qPrintable("ERROR in column lastName: data not match! (Create user parent Admin test)"));
	QVERIFY2(tempQuery.value("password").toString() == password, qPrintable("ERROR in column password: data not match! (Create user parent Admin test)"));
	QVERIFY2(tempQuery.value("administrator").toBool() == false, qPrintable("ERROR in column isAdmin: data not match! (Create user parent Admin test)"));
	QVERIFY2(tempQuery.value("readOnly").toBool() == false, qPrintable("ERROR in column isReadOnly: data not match! (Create user parent Admin test)"));
	QVERIFY2(tempQuery.value("disabled").toBool() == false, qPrintable("ERROR in column isDisabled: data not match! (Create user parent Admin test)"));

	// Create administrator with parent user
	//

	parentUser = "2";
	userName = "TestUser2";
	firstName = "Jack";
	lastName = "Toaster";
	password = "qwerty";
	administrator = "true";
	readOnly = "false";
	disabled = "false";

	ok = query.exec(QString("SELECT create_user(%1, '%2', '%3', '%4', '%5', %6, %7, %8);")
								   .arg(parentUser)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(administrator)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == false, qPrintable("User can't be parent of administrator error expected"));

	// Create user with parent user
	//

	parentUser = "2";
	userName = "TestUser3";
	firstName = "Jack";
	lastName = "Toaster";
	password = "qwerty";
	administrator = "false";
	readOnly = "false";
	disabled = "false";

	ok = query.exec(QString("SELECT create_user(%1, '%2', '%3', '%4', '%5', %6, %7, %8);")
								   .arg(parentUser)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(administrator)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == false, qPrintable("User can't be parent of another user error expected"));

	// Create Administrator with parent Administrator
	//

	parentUser = "1";
	userName = "TestUser4";
	firstName = "Jack";
	lastName = "Toaster";
	password = "qwerty";
	administrator = "true";
	readOnly = "false";
	disabled = "false";

	ok = query.exec(QString("SELECT create_user(%1, '%2', '%3', '%4', '%5', %6, %7, %8);")
								   .arg(parentUser)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(administrator)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(("SELECT * FROM users WHERE username='" + userName + "'"));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(tempQuery.value("username").toString() == userName, qPrintable("ERROR in column userName: data not match! (Create Administrator with parent Administrator test)"));
	QVERIFY2(tempQuery.value("firstName").toString() == firstName, qPrintable("ERROR in column firstName: data not match! (Create Administrator with parent Administrator test)"));
	QVERIFY2(tempQuery.value("lastName").toString() == lastName, qPrintable("ERROR in column lastName: data not match! (Create Administrator with parent Administrator test)"));
	QVERIFY2(tempQuery.value("password").toString() == password, qPrintable("ERROR in column password: data not match! (Create Administrator with parent Administrator test)"));
	QVERIFY2(tempQuery.value("administrator").toBool() == true, qPrintable("ERROR in column isAdmin: data not match! (Create Administrator with parent Administrator test)"));
	QVERIFY2(tempQuery.value("readOnly").toBool() == false, qPrintable("ERROR in column isReadOnly: data not match! (Create Administrator with parent Administrator test)"));
	QVERIFY2(tempQuery.value("disabled").toBool() == false, qPrintable("ERROR in column isDisabled: data not match! (Create Administrator with parent Administrator test)"));

	// Create user what already exists
	//

	parentUser = "1";
	userName = "TestUser1";
	firstName = "Jack";
	lastName = "Toaster";
	password = "qwerty";
	administrator = "false";
	readOnly = "false";
	disabled = "false";

	ok = query.exec(QString("SELECT create_user(%1, '%2', '%3', '%4', '%5', %6, %7, %8);")
								   .arg(parentUser)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(administrator)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == false, qPrintable("User already exists error expected"));

	// Create readonly user
	//

	parentUser = "1";
	userName = "TestUser5";
	firstName = "Jack";
	lastName = "Toaster";
	password = "qwerty";
	administrator = "false";
	readOnly = "true";
	disabled = "false";

	ok = query.exec(QString("SELECT create_user(%1, '%2', '%3', '%4', '%5', %6, %7, %8);")
								   .arg(parentUser)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(administrator)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(("SELECT * FROM users WHERE username='" + userName + "'"));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(tempQuery.value("username").toString() == userName, qPrintable("ERROR in column userName: data not match! (Create readonly user test)"));
	QVERIFY2(tempQuery.value("firstName").toString() == firstName, qPrintable("ERROR in column firstName: data not match! (Create readonly user test)"));
	QVERIFY2(tempQuery.value("lastName").toString() == lastName, qPrintable("ERROR in column lastName: data not match! (Create readonly user test)"));
	QVERIFY2(tempQuery.value("password").toString() == password, qPrintable("ERROR in column password: data not match! (Create readonly user test)"));
	QVERIFY2(tempQuery.value("administrator").toBool() == false, qPrintable("ERROR in column isAdmin: data not match! (Create readonly user test)"));
	QVERIFY2(tempQuery.value("readOnly").toBool() == true, qPrintable("ERROR in column isReadOnly: data not match! (Create readonly user test)"));
	QVERIFY2(tempQuery.value("disabled").toBool() == false, qPrintable("ERROR in column isDisabled: data not match! (Create readonly user test)"));

	// Create disabled user
	//

	parentUser = "1";
	userName = "TestUser6";
	firstName = "Jack";
	lastName = "Toaster";
	password = "qwerty";
	administrator = "false";
	readOnly = "false";
	disabled = "true";

	ok = query.exec(QString("SELECT create_user(%1, '%2', '%3', '%4', '%5', %6, %7, %8);")
								   .arg(parentUser)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(administrator)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(("SELECT * FROM users WHERE username='" + userName + "'"));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(tempQuery.value("username").toString() == userName, qPrintable("ERROR in column userName: data not match! (Create disabled user test)"));
	QVERIFY2(tempQuery.value("firstName").toString() == firstName, qPrintable("ERROR in column firstName: data not match! (Create disabled user test)"));
	QVERIFY2(tempQuery.value("lastName").toString() == lastName, qPrintable("ERROR in column lastName: data not match! (Create disabled user test)"));
	QVERIFY2(tempQuery.value("password").toString() == password, qPrintable("ERROR in column password: data not match! (Create disabled user test)"));
	QVERIFY2(tempQuery.value("administrator").toBool() == false, qPrintable("ERROR in column isAdmin: data not match! (Create disabled user test)"));
	QVERIFY2(tempQuery.value("readOnly").toBool() == false, qPrintable("ERROR in column isReadOnly: data not match! (Create disabled user test)"));
	QVERIFY2(tempQuery.value("disabled").toBool() == true, qPrintable("ERROR in column isDisabled: data not match! (Create disabled user test)"));

	// Call small password error
	//

	parentUser = "1";
	userName = "TestUser7";
	firstName = "Jack";
	lastName = "Toaster";
	password = "small";
	administrator = "false";
	readOnly = "false";
	disabled = "true";

	ok = query.exec(QString("SELECT create_user(%1, '%2', '%3', '%4', '%5', %6, %7, %8);")
								   .arg(parentUser)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(administrator)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == false, qPrintable("Small password error expected"));
}

void UserTests::getUserIDTest()
{
	QSqlQuery query;

	// Create user to test
	//

	bool ok = query.exec("SELECT create_user (1, 'getUserIdTest', 'getUserIdTest', 'getUserIdTest', 'getUserIdTest', false, false, false);");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int userId = query.value("create_user").toInt();

	ok = query.exec(QString("SELECT get_user_id('%1', '%2');").arg("getUserIdTest").arg("getUserIdTest"));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("get_user_id").toInt() == userId, qPrintable("Error: userId not match"));

	ok = query.exec(QString("SELECT get_user_id('%1', '%2');").arg("getUserIdTest").arg("abc"));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("get_user_id").toInt() == 0, qPrintable("Error: 0 id expected"));

	ok = query.exec(QString("SELECT get_user_id('%1', '%2');").arg("def").arg("getUserIdTest"));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("get_user_id").toInt() == 0, qPrintable("Error: 0 id expected"));
}

void UserTests::isAdminTest()
{
	QSqlQuery query;

	// Create user without admin rights to test
	//

	bool ok = query.exec("SELECT create_user (1, 'AdminTest', 'TEST', 'TEST', 'TESTPASS', false, false, false);");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int m_isAdminTempDataID = query.value("create_user").toInt();

	// Create and delete user to make NULL row for test
	//

	ok = query.exec("SELECT create_user (1, 'AdminTest1', 'TEST', 'TEST', 'TESTPASS', false, false, false);");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int m_isAdminTempDataNullID = query.value("create_user").toInt();

	ok = query.exec(("DELETE FROM users WHERE username='AdminTest1'"));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT is_admin (%1);").arg(1));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("is_admin").toBool() == true, qPrintable("Error: \"true\" expected"));

	ok = query.exec(QString("SELECT is_admin (%1);").arg(m_isAdminTempDataID));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("is_admin").toBool() == false, qPrintable("Error: \"false\" expected"));

	ok = query.exec(QString("SELECT is_admin (%1);").arg(m_isAdminTempDataNullID));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("is_admin").toBool() == false, qPrintable("Data error expected"));
}

void UserTests::check_user_passwordIntegerTextTest()
{
	QSqlQuery query;
	QString password = "12341234";

	// Create user to test
	//

	bool ok = query.exec(QString("SELECT * FROM create_user(1, 'Tester', 'Richard', 'Stollman', '%1', false, false, false)").arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int userId = query.value("create_user").toInt();

	ok = query.exec(QString("SELECT * FROM check_user_password(%1, '%2');").arg(userId).arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == true, qPrintable("Error: password or username is wrong!"));

	ok = query.exec(QString("SELECT * FROM check_user_password(%1, '%2');").arg(userId).arg("password"));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == false, qPrintable("Wrong username or password error expected"));

	ok = query.exec(QString("SELECT * FROM check_user_password(%1, '%2');").arg(9999999).arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == false, qPrintable("Wrong username or password error expected"));

	ok = query.exec(QString("SELECT * FROM check_user_password(%1, '%2');").arg(userId).arg(""));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == false, qPrintable("Wrong username or password error expected"));

	ok = query.exec(QString("SELECT * FROM check_user_password(%1, '%2');").arg(0).arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == false, qPrintable("Wrong username or password error expected"));
}

void UserTests::check_user_passwordTextTextTest()
{
	QSqlQuery query;
	QString password = "12341234";
	QString name = "Linus";

	// Create user to test
	//

	bool ok = query.exec(QString("SELECT * FROM create_user(1, '%1', 'Richard', 'Stollman', '%2', false, false, false)").arg(name).arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_user_password('%1', '%2');").arg(name).arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == true, qPrintable("Error: password or username is wrong!"));

	ok = query.exec(QString("SELECT * FROM check_user_password('%1', '%2');").arg(name).arg("password"));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == false, qPrintable("Wrong username or password error expected"));

	ok = query.exec(QString("SELECT * FROM check_user_password('%1', '%2');").arg("name").arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == false, qPrintable("Wrong username or password error expected"));

	ok = query.exec(QString("SELECT * FROM check_user_password('%1', '%2');").arg(name).arg(""));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == false, qPrintable("Wrong username or password error expected"));

	ok = query.exec(QString("SELECT * FROM check_user_password('%1', '%2');").arg("").arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == false, qPrintable("Wrong username or password error expected"));
}

void UserTests::update_userTest()
{
	QSqlQuery query;
	QString oldDataForTest = "updateUserTest";
	QString newDataForTest = "updateUserTestUpdated";
	QString simplePassword = "1234";

	bool ok = query.exec(QString("SELECT * FROM create_user(1, '%1', '%1', '%1', '%1', false, false, false)").arg(oldDataForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM update_user(1, '%1', '%2', '%2', '%1', '%2', true, true, true)").arg(oldDataForTest).arg(newDataForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM users WHERE userId = %1").arg(query.value(0).toInt()));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("firstName").toString() == newDataForTest, qPrintable("Error: firstName is not correct!"));
	QVERIFY2(query.value("lastName").toString() == newDataForTest, qPrintable("Error: lastName is not correct!"));
	QVERIFY2(query.value("password").toString() == newDataForTest, qPrintable("Error: lastName is not correct!"));
	QVERIFY2(query.value("administrator").toBool() == true, qPrintable("Error: lastName is not correct!"));
	QVERIFY2(query.value("readonly").toBool() == true, qPrintable("Error: lastName is not correct!"));
	QVERIFY2(query.value("disabled").toBool() == true, qPrintable("Error: lastName is not correct!"));


	// Call too simple password error
	//

	ok = query.exec(QString("SELECT * FROM update_user(1, '%1', '%1', '%1', '%2', '%3', false, false, false)").arg(oldDataForTest).arg(newDataForTest).arg(simplePassword));
	QVERIFY2(ok == false, qPrintable("Too small password error expected!"));

	// Call wrong user error
	//

	ok = query.exec("SElECT * FROM create_user(1, 'UpdateUserWrongUserErrorTest', 'TEST', 'TEST', 'testPass', false, false, false)");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int userForErrorTest = query.value(0).toInt();

	ok = query.exec(QString("SELECT * FROM update_user(%2, '%1', 'TEST', 'TEST', 'testPass', '%1', false, false, false)").arg(oldDataForTest).arg(userForErrorTest));
	QVERIFY2(ok == false, qPrintable("Wrong user error expected!"));

	// Try to change one user by another
	//

	ok = query.exec("SElECT * FROM create_user(1, 'UpdateUserWrongUserChangePassErrorTest', 'firstName', 'lastName', 'testPassword', false, false, false)");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM update_user(%2, 'UpdateUserWrongUserChangePassErrorTest', 'firstName', 'lastName', 'testPassword', 'newPassword', false, false, false)").arg(userForErrorTest));
	QVERIFY2(ok == false, qPrintable("Error: user must not be able to change another user's password"));

	// Try to change user flags by user
	//
	ok = query.exec("SElECT * FROM create_user(1, 'UpdateUserHackAdminTest', 'firstName', 'lastName', 'testPassword', false, false, false)");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int setAdminUpdateUserTest = query.value(0).toInt();

	ok = query.exec(QString("SELECT * FROM update_user(%2, 'UpdateUserHackAdminTest', 'firstName', 'lastName', 'testPassword', 'newPassword', true, true, true)").arg(setAdminUpdateUserTest));
	QVERIFY2(ok == false, qPrintable("Error: user must not be able to change his rights"));

	// Call wrong password error
	//

	ok = query.exec("SElECT * FROM create_user(1, 'UpdateUserWrongPasswordAdministrator', 'TEST', 'TEST', 'TESTPASS', true, false, false)");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int AdministratorForErrorTest = query.value(0).toInt();

	ok = query.exec(QString("SELECT * FROM update_user(%1, 'UpdateUserWrongPasswordAdministrator', 'TEST', 'TEST', 'errorPassword', 'newPassword', false, false, false)").arg(AdministratorForErrorTest));
	QVERIFY2(ok == false, qPrintable("Wrong password error expected!"));

	// Call user is not exist error
	//

	ok = query.exec(QString("SELECT * FROM update_user(1, 'errorNameForUpdateUserTest', '%1', '%1', '%2', '%1', false, false, false)").arg(oldDataForTest).arg(newDataForTest));
	QVERIFY2(ok == false, qPrintable("Wrong userName error expected!"));
}
