#include "UserTests.h"

UserTests::UserTests()
{
}

void UserTests::setDatabaseHost(QString host)
{
	assert(host.isEmpty() == false);
	m_dbHost = host;
}

void UserTests::setDatabaseUser(QString userName)
{
	assert(userName.isEmpty() == false);
	m_dbUser = userName;
}

void UserTests::setDatabaseUserPassword(QString password)
{
	assert(password.isEmpty() == false);
	m_dbUserPassword == password;
}

void UserTests::setProjectName(QString projectName)
{
	assert(projectName.isEmpty() == false);
	m_projectName = projectName;
}

void UserTests::setAdminPassword(QString password)
{
	assert(password.isEmpty() == false);
	m_adminPassword = password;
}

void UserTests::initTestCase()
{
}

void UserTests::cleanupTestCase()
{
}

void UserTests::logInOutTest()
{
	QSqlQuery query;
	QString session_key;

	// Alter Administrator user. Set administrator password "123412341234"
	//

	bool ok = query.exec("SELECT salt FROM users WHERE username = 'Administrator'");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QString passwordHashQuery = QString("user_api.password_hash('%1', '%2')").arg(query.value(0).toString()).arg(m_adminPassword);

	ok = query.exec(QString("UPDATE users SET passwordhash = %1 WHERE username = 'Administrator'").arg(passwordHashQuery));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Try log in with wrong password
	//

	ok = query.exec("SELECT * FROM user_api.log_in('Administrator', 'wrong')");

	QVERIFY2 (ok == false, qPrintable("Expected error: can not log in with wrong password"));

	// Try log in with wrong user
	//

	ok = query.exec("SELECT * FROM user_api.log_in('UserWithNoName', 'wrong')");

	QVERIFY2 (ok == false, qPrintable("Expected error: can not log in with wrong user"));

	// Try normal log in
	//

	ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_adminPassword));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	session_key = query.value(0).toString();

	ok = query.exec(QString("SELECT COUNT(*) FROM session_table WHERE session_key = '%1' AND userId = 1").arg(session_key));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toInt() == 1, qPrintable(QString("Error: wrong record in session_table: wrong amount of records with same session_key. Expected 1, actually: %1")
	                                                 .arg(query.value(0).toInt())));

	// Try log in twice
	//

	ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_adminPassword));

	QVERIFY2 (ok == false, qPrintable("Expected error: can not log in twice"));

	// Try to log out
	//

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toInt() == 1, qPrintable("Error: wrong userId returned after log_out"));

	// Try to log out twice
	//

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == false, qPrintable("Expected error: user can not log_out twice"));

	//
	// Create other user. Log_in from administrator. Log_in from new user.
	// Log_out from administrator. Check user is still logged in. Log_out from user
	//

	ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_adminPassword));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	session_key = query.value(0).toString();

	query.exec(QString("SELECT * FROM user_api.create_user('%1', 'userForLoggingTest', 'test', 'test', 'testtest', false, false);").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int userId = query.value(0).toInt();

	QSqlDatabase tempDbConnection = QSqlDatabase::addDatabase("QPSQL", "tempConnection");

	tempDbConnection.setHostName(m_dbHost);
	tempDbConnection.setUserName(m_dbUser);
	tempDbConnection.setPassword(m_dbUserPassword);
	tempDbConnection.setDatabaseName(QString("u7_") + m_projectName);

	QVERIFY2(tempDbConnection.open() == true, qPrintable(tempDbConnection.lastError().databaseText()));

	QSqlQuery tempQuery(tempDbConnection);

	ok = tempQuery.exec(QString("SELECT * FROM user_api.log_in('%1','%2');")
	               .arg("userForLoggingTest")
	               .arg("testtest"));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	QString tempSession_key = query.value(0).toString();

	// Now two users are logged in. Log_out Administrator.
	//

	assert (session_key != tempSession_key);

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toInt() == 1, qPrintable("Error: wrong userId returned after log_out"));

	ok = tempQuery.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(tempQuery.value(0).toInt() == userId, qPrintable("Error: wrong userId returned after log_out"));

	tempDbConnection.close();
	QSqlDatabase::removeDatabase("tempConnection");
}

void UserTests::createUserTest()
{	
	QSqlQuery query;
	QSqlQuery tempQuery;

	// Create user
	//

	QString session_key;
	QString userName = "TestUser1";
	QString firstName = "Jack";
	QString lastName = "Toaster";
	QString password = "qwerty";
	QString readOnly = "false";
	QString disabled = "false";

	bool ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_adminPassword));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	session_key = query.value(0).toString();

	ok = query.exec(QString("SELECT user_api.create_user('%1', '%2', '%3', '%4', '%5', %6, %7);")
	                               .arg(session_key)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(("SELECT * FROM users WHERE username='" + userName + "'"));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(tempQuery.value("username").toString() == userName, qPrintable("ERROR in column userName: data not match! (Create user parent Admin test)"));
	QVERIFY2(tempQuery.value("firstName").toString() == firstName, qPrintable("ERROR in column firstName: data not match! (Create user parent Admin test)"));
	QVERIFY2(tempQuery.value("lastName").toString() == lastName, qPrintable("ERROR in column lastName: data not match! (Create user parent Admin test)"));
	QVERIFY2(tempQuery.value("administrator").toBool() == false, qPrintable("ERROR in column isAdmin: data not match! (Create user parent Admin test)"));
	QVERIFY2(tempQuery.value("readOnly").toBool() == false, qPrintable("ERROR in column isReadOnly: data not match! (Create user parent Admin test)"));
	QVERIFY2(tempQuery.value("disabled").toBool() == false, qPrintable("ERROR in column isDisabled: data not match! (Create user parent Admin test)"));

	// Create user what already exists
	//

	userName = "TestUser1";
	firstName = "Jack";
	lastName = "Toaster";
	password = "qwerty";
	readOnly = "false";
	disabled = "false";

	ok = query.exec(QString("SELECT user_api.create_user('%1', '%2', '%3', '%4', '%5', %6, %7);")
	                               .arg(session_key)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == false, qPrintable("User already exists error expected"));

	// Create readonly user
	//

	userName = "TestUser5";
	firstName = "Jack";
	lastName = "Toaster";
	password = "qwerty";
	readOnly = "true";
	disabled = "false";

	ok = query.exec(QString("SELECT user_api.create_user('%1', '%2', '%3', '%4', '%5', %6, %7);")
	                               .arg(session_key)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(("SELECT * FROM users WHERE username='" + userName + "'"));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(tempQuery.value("username").toString() == userName, qPrintable("ERROR in column userName: data not match! (Create readonly user test)"));
	QVERIFY2(tempQuery.value("firstName").toString() == firstName, qPrintable("ERROR in column firstName: data not match! (Create readonly user test)"));
	QVERIFY2(tempQuery.value("lastName").toString() == lastName, qPrintable("ERROR in column lastName: data not match! (Create readonly user test)"));
	QVERIFY2(tempQuery.value("administrator").toBool() == false, qPrintable("ERROR in column isAdmin: data not match! (Create readonly user test)"));
	QVERIFY2(tempQuery.value("readOnly").toBool() == true, qPrintable("ERROR in column isReadOnly: data not match! (Create readonly user test)"));
	QVERIFY2(tempQuery.value("disabled").toBool() == false, qPrintable("ERROR in column isDisabled: data not match! (Create readonly user test)"));

	// Create disabled user
	//

	userName = "TestUser6";
	firstName = "Jack";
	lastName = "Toaster";
	password = "qwerty";
	readOnly = "false";
	disabled = "true";

	ok = query.exec(QString("SELECT user_api.create_user('%1', '%2', '%3', '%4', '%5', %6, %7);")
	                               .arg(session_key)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(("SELECT * FROM users WHERE username='" + userName + "'"));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(tempQuery.value("username").toString() == userName, qPrintable("ERROR in column userName: data not match! (Create disabled user test)"));
	QVERIFY2(tempQuery.value("firstName").toString() == firstName, qPrintable("ERROR in column firstName: data not match! (Create disabled user test)"));
	QVERIFY2(tempQuery.value("lastName").toString() == lastName, qPrintable("ERROR in column lastName: data not match! (Create disabled user test)"));
	QVERIFY2(tempQuery.value("administrator").toBool() == false, qPrintable("ERROR in column isAdmin: data not match! (Create disabled user test)"));
	QVERIFY2(tempQuery.value("readOnly").toBool() == false, qPrintable("ERROR in column isReadOnly: data not match! (Create disabled user test)"));
	QVERIFY2(tempQuery.value("disabled").toBool() == true, qPrintable("ERROR in column isDisabled: data not match! (Create disabled user test)"));

	// Call small password error
	//

	userName = "TestUser7";
	firstName = "Jack";
	lastName = "Toaster";
	password = "small";
	readOnly = "false";
	disabled = "true";

	ok = query.exec(QString("SELECT user_api.create_user('%1', '%2', '%3', '%4', '%5', %6, %7);")
	                               .arg(session_key)
								   .arg(userName)
								   .arg(firstName)
								   .arg(lastName)
								   .arg(password)
								   .arg(readOnly)
								   .arg(disabled));

	QVERIFY2(ok == false, qPrintable("Small password error expected"));

	// Call wrong session_key error
	//

	userName = "TestUser3";
	firstName = "Jack";
	lastName = "Toaster";
	password = "small";
	readOnly = "false";
	disabled = "true";

	ok = query.exec(QString("SELECT user_api.create_user('wrong', '%1', '%2', '%3', '%4', %5, %6);")
	                               .arg(userName)
	                               .arg(firstName)
	                               .arg(lastName)
	                               .arg(password)
	                               .arg(readOnly)
	                               .arg(disabled));

	QVERIFY2(ok == false, qPrintable("Small password error expected"));

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
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

	QVERIFY2(query.value("firstName").toString() == newDataForTest, qPrintable("Error: firstName is not correct, or wrong userId has been returned!"));
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

	ok = query.exec(QString("SELECT * FROM update_user(%1, 'UpdateUserHackAdminTest', 'firstName', 'lastName', 'testPassword', 'newPassword', true, true, true)").arg(setAdminUpdateUserTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toInt() == setAdminUpdateUserTest, qPrintable ("Error: function returned wrong id"));

	ok = query.exec(QString("SELECT * FROM users WHERE userId = %1").arg(setAdminUpdateUserTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("administrator").toBool() == false, qPrintable ("Error: function changed value administrator by user"));
	QVERIFY2(query.value("readOnly").toBool() == true, qPrintable ("Error: function do not changed value readOnly"));
	QVERIFY2(query.value("disabled").toBool() == false, qPrintable ("Error: function changed value disabled y user"));

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

	/////////////////////////////////////////////////////////////////////
	// Another function without is_Admin testing
	//
	/////////////////////////////////////////////////////////////////////

	ok = query.exec(QString("SELECT * FROM update_user(1, '%1', '%2', '%2', '%1', '%2', true, true)").arg(oldDataForTest).arg(newDataForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM users WHERE userId = %1").arg(query.value(0).toInt()));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("firstName").toString() == newDataForTest, qPrintable("Error: firstName is not correct, or wrong userId has been returned!"));
	QVERIFY2(query.value("lastName").toString() == newDataForTest, qPrintable("Error: lastName is not correct!"));
	QVERIFY2(query.value("password").toString() == newDataForTest, qPrintable("Error: lastName is not correct!"));
	QVERIFY2(query.value("readonly").toBool() == true, qPrintable("Error: lastName is not correct!"));
	QVERIFY2(query.value("disabled").toBool() == true, qPrintable("Error: lastName is not correct!"));


	// Call too simple password error
	//

	ok = query.exec(QString("SELECT * FROM update_user(1, '%1', '%1', '%1', '%2', '%3', false, false)").arg(oldDataForTest).arg(newDataForTest).arg(simplePassword));
	QVERIFY2(ok == false, qPrintable("Too small password error expected!"));

	// Call wrong user error
	//

	ok = query.exec(QString("SELECT * FROM update_user(%2, '%1', 'TEST', 'TEST', 'testPass', '%1', false, false)").arg(oldDataForTest).arg(userForErrorTest));
	QVERIFY2(ok == false, qPrintable("Wrong user error expected!"));

	// Try to change one user by another
	//

	ok = query.exec(QString("SELECT * FROM update_user(%2, 'UpdateUserWrongUserChangePassErrorTest', 'firstName', 'lastName', 'testPassword', 'newPassword', false, false)").arg(userForErrorTest));
	QVERIFY2(ok == false, qPrintable("Error: user must not be able to change another user's password"));

	// Try to change user flags by user
	//

	ok = query.exec(QString("SELECT * FROM update_user(%1, 'UpdateUserHackAdminTest', 'firstName', 'lastName', 'newPassword', 'testPassword', true, true)").arg(setAdminUpdateUserTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toInt() == setAdminUpdateUserTest, qPrintable ("Error: function returned wrong id"));

	ok = query.exec(QString("SELECT * FROM users WHERE userId = %1").arg(setAdminUpdateUserTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("readOnly").toBool() == true, qPrintable ("Error: function do not changed value readOnly"));
	QVERIFY2(query.value("disabled").toBool() == false, qPrintable ("Error: function changed value disabled y user"));

	// Call wrong password error
	//

	ok = query.exec(QString("SELECT * FROM update_user(%1, 'UpdateUserWrongPasswordAdministrator', 'TEST', 'TEST', 'errorPassword', 'newPassword', false, false)").arg(AdministratorForErrorTest));
	QVERIFY2(ok == false, qPrintable("Wrong password error expected!"));

	// Call user is not exist error
	//

	ok = query.exec(QString("SELECT * FROM update_user(1, 'errorNameForUpdateUserTest', '%1', '%1', '%2', '%1', false, false)").arg(oldDataForTest).arg(newDataForTest));
	QVERIFY2(ok == false, qPrintable("Wrong userName error expected!"));

}
