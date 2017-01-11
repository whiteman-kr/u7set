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
	QSqlQuery query;

	// Alter Administrator user. Set administrator password "123412341234"
	//

	bool ok = query.exec("SELECT salt FROM users WHERE username = 'Administrator'");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QString passwordHashQuery = QString("user_api.password_hash('%1', '%2')").arg(query.value(0).toString()).arg(m_adminPassword);

	ok = query.exec(QString("UPDATE users SET passwordhash = %1 WHERE username = 'Administrator'").arg(passwordHashQuery));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
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

void UserTests::checkSessionKeyTest()
{
	QSqlQuery query;
	QString session_key;

	// Log in as Administrator
	//

	bool ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_adminPassword));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	session_key = query.value(0).toString();

	// Check with enabled error messages
	//

	ok = query.exec(QString("SELECT * FROM user_api.check_session_key('%1', true)").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error: function returned false with valid session_key (error messages enabled)"));

	// Check with disabled error messages
	//

	ok = query.exec(QString("SELECT * FROM user_api.check_session_key('%1', false)").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error: function returned false with valid session_key (error messages disabled)"));

	// Log out and check again
	//

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM user_api.check_session_key('%1', true)").arg(session_key));

	QVERIFY2(ok == false, qPrintable("Expected error: user is not logged in"));

	ok = query.exec(QString("SELECT * FROM user_api.check_session_key('%1', false)").arg(session_key));

	QVERIFY2(ok == false, qPrintable("Expected error: user is not logged in"));

	// Check invalid session_key
	//

	ok = query.exec("SELECT * FROM user_api.check_session_key('1234',true)");

	QVERIFY2(ok == false, qPrintable("Expected error: Invalid session_key"));

	ok = query.exec("SELECT * FROM user_api.check_session_key('1234',false)");

	QVERIFY2(ok == false, qPrintable("Expected error: Invalid session_key"));

	// Check empty session_key
	//

	ok = query.exec("SELECT * FROM user_api.check_session_key('',true)");

	QVERIFY2(ok == false, qPrintable("Expected error: Invalid session_key"));

	ok = query.exec("SELECT * FROM user_api.check_session_key('',false)");

	QVERIFY2(ok == false, qPrintable("Expected error: Invalid session_key"));
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

	// Create user which already exists
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
}

void UserTests::currentUserIdTest()
{
	QSqlQuery query;
	QString session_key;

	// Log in as Administrator to create new user for test
	//

	bool ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_adminPassword));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	session_key = query.value(0).toString();

	// Create user to test
	//

	ok = query.exec(QString("SELECT user_api.create_user ('%1', 'currentUserIDTest', 'currentUserIDTest', 'currentUserIDTest', 'currentUserIDTest', false, false);").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int userId = query.value("create_user").toInt();

	// Log out as Administrator
	//

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Try use function with logged off session_key
	//

	ok = query.exec(QString("SELECT user_api.current_user_id('%1');").arg(session_key));

	QVERIFY2(ok == false, qPrintable("Expected error: user already logged off"));

	// Log in as new user and try again
	//

	ok = query.exec("SELECT * FROM user_api.log_in('currentUserIDTest', 'currentUserIDTest')");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	session_key = query.value(0).toString();

	ok = query.exec(QString("SELECT user_api.current_user_id('%1');").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toInt() == userId, qPrintable("Error: function current_user_id returned wrong userId"));

	// Log out
	//

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Try with wrong session key
	//

	ok = query.exec("SELECT user_api.current_user_id('1234');");

	QVERIFY2(ok == false, qPrintable("Expected error: wrong session_key"));
}

void UserTests::isCurrentUserAdminTest()
{
	QSqlQuery query;
	QString session_key;

	// Log in as Administrator to create new user for test
	//

	bool ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_adminPassword));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	session_key = query.value(0).toString();

	// Create user to test
	//

	ok = query.exec(QString("SELECT user_api.create_user ('%1', 'isCurrentUserAdmin', 'isCurrentUserAdmin', 'isCurrentUserAdmin', 'isCurrentUserAdmin', false, false);").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	// Check administrator user
	//

	ok = query.exec(QString("SELECT * FROM user_api.is_current_user_admin('%1')").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error: function is_current_user_admin with session key of administrator returned false"));

	// Force make administrator disabled, and check again
	//

	ok = query.exec("UPDATE users SET Disabled = true WHERE userId = 1");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM user_api.is_current_user_admin('%1')").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: function is_current_user_admin with session key of disabled administrator returned true"));

	ok = query.exec("UPDATE users SET Disabled = false WHERE userId = 1");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	// Log out as Administrator, and check function again
	//

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM user_api.is_current_user_admin('%1')").arg(session_key));

	QVERIFY2(ok == false, qPrintable("Expected error: user is not logged in"));

	// Log in as ordinary user
	//

	ok = query.exec("SELECT * FROM user_api.log_in('isCurrentUserAdmin', 'isCurrentUserAdmin')");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	session_key = query.value(0).toString();

	ok = query.exec(QString("SELECT * FROM user_api.is_current_user_admin('%1')").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: function is_current_user_admin with session key of ordinary user returned true"));

	// Log out
	//

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
}

void UserTests::getUserDataTest()
{
	QSqlQuery query;

	QString session_key;
	QString userName = "getUserDataName";
	QString firstName = "getUserDataFirstName";
	QString lastName = "getUserDataLastName";
	QString password = "getUserDataPassword";

	int userId = -1;

	// Log in as Administrator to create new user for test
	//

	bool ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_adminPassword));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	session_key = query.value(0).toString();

	ok = query.exec(QString("SELECT user_api.create_user ('%1', '%2', '%3', '%4', '%5', false, false);")
	                .arg(session_key)
	                .arg(userName)
	                .arg(firstName)
	                .arg(lastName)
	                .arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	userId = query.value(0).toInt();

	ok = query.exec(QString("SELECT * FROM user_api.get_user_data('%1', %2)").arg(session_key).arg(userId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("userId").toInt() == userId, qPrintable("Error: wrong userId returned"));
	QVERIFY2(query.value("userName").toString() == userName, qPrintable("Error: wrong userName returned"));
	QVERIFY2(query.value("firstName").toString() == firstName, qPrintable("Error: wrong firstName returned"));
	QVERIFY2(query.value("lastName").toString() == lastName, qPrintable("Error: wrong lastName returned"));
	QVERIFY2(query.value("administrator").toBool() == false, qPrintable("Error: wrong administrator value returned"));
	QVERIFY2(query.value("readOnly").toBool() == false, qPrintable("Error: wrong readOnly value returned"));
	QVERIFY2(query.value("disabled").toBool() == false, qPrintable("Error: wrong disabled value returned"));

	// Try use function with invalid session_key
	//

	ok = query.exec(QString("SELECT * FROM user_api.get_user_data('wrong', %1)").arg(userId));
	QVERIFY2(ok == false, qPrintable("Expected error: wrong session_key"));

	// Try use function with empty session_key
	//

	ok = query.exec(QString("SELECT * FROM user_api.get_user_data('', %1)").arg(userId));
	QVERIFY2(ok == false, qPrintable("Expected error: wrong session_key"));

	// Try use function with wrong user
	//

	ok = query.exec(QString("SELECT * FROM user_api.get_user_data('%1', -1)").arg(session_key));
	QVERIFY2(ok == false, qPrintable("Expected error: invalid user"));

	// Log out as Administrator
	//

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
}

void UserTests::check_user_passwordTest()
{
	QString login = "checkUserPasswordTest";
	QString password = "TestPassword";

	QSqlQuery query;
	QString session_key;

	// Log in as Administrator to create new user for test
	//

	bool ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_adminPassword));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	session_key = query.value(0).toString();

	// Create user to test
	//

	ok = query.exec(QString("SELECT user_api.create_user ('%1', '%2', 'Richard', 'Stallman', '%3', false, false);").arg(session_key).arg(login).arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	// Log out as Administrator
	//

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM user_api.check_user_password('%1', '%2');").arg(login).arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == true, qPrintable("Error: password or username is wrong!"));

	ok = query.exec(QString("SELECT * FROM user_api.check_user_password('%1', '%2');").arg(login).arg("password"));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == false, qPrintable("Wrong username or password error expected"));

	ok = query.exec(QString("SELECT * FROM user_api.check_user_password('%1', '%2');").arg("name").arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == false, qPrintable("Wrong username or password error expected"));

	ok = query.exec(QString("SELECT * FROM user_api.check_user_password('%1', '%2');").arg(login).arg(""));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == false, qPrintable("Wrong username or password error expected"));

	ok = query.exec(QString("SELECT * FROM user_api.check_user_password('%1', '%2');").arg("").arg(password));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0) == false, qPrintable("Wrong username or password error expected"));
}

void UserTests::update_userTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;
	QString oldDataForTest = "updateUserTest";
	QString newDataForTest = "updateUserTestUpdated";
	QString simplePassword = "1234";

	// Log in as Administrator to create new user for test
	//

	bool ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_adminPassword));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QString session_key = query.value(0).toString();

	ok = query.exec(QString("SELECT * FROM user_api.create_user('%1', '%2', '%2', '%2', '%2', false, false)").arg(session_key).arg(oldDataForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	// Try update user with wrong session_key
	//

	ok = query.exec(QString("SELECT * FROM user_api.update_user('wrong', '%1', '%2', '%2', '%1', '%2', true, true)").arg(oldDataForTest).arg(newDataForTest));

	QVERIFY2(ok == false, qPrintable("Wrong session key error expected!"));

	// Update user with valid data
	//

	ok = query.exec(QString("SELECT * FROM user_api.update_user('%1', '%2', '%3', '%3', '%2', '%3', true, true)").arg(session_key).arg(oldDataForTest).arg(newDataForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM users WHERE userId = %1").arg(query.value(0).toInt()));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("firstName").toString() == newDataForTest, qPrintable("Error: firstName is not correct, or wrong userId has been returned!"));
	QVERIFY2(query.value("lastName").toString() == newDataForTest, qPrintable("Error: lastName is not correct!"));
	QVERIFY2(query.value("administrator").toBool() == false, qPrintable("Error: administrator value is not correct!"));
	QVERIFY2(query.value("readonly").toBool() == true, qPrintable("Error: readonly value is not correct!"));
	QVERIFY2(query.value("disabled").toBool() == true, qPrintable("Error: disabled value is not correct!"));

	ok = tempQuery.exec(QString("SELECT * FROM user_api.password_hash('%1', '%2')").arg(query.value("salt").toString(), newDataForTest));

	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.first() == true, qPrintable(tempQuery.lastError().databaseText()));

	QVERIFY2(query.value("passwordhash").toString() == tempQuery.value(0).toString(), qPrintable("Error: wrong password hash after user update"));

	// Call too simple password error
	//

	ok = query.exec(QString("SELECT * FROM user_api.update_user('%1', '%2', '%2', '%2', '%3', '%4', false, false)").arg(session_key).arg(oldDataForTest).arg(newDataForTest).arg(simplePassword));
	QVERIFY2(ok == false, qPrintable("Too small password error expected!"));

	// Call wrong user error
	//

	ok = query.exec(QString("SELECT * FROM user_api.update_user('%1', 'WrongUser', 'TEST', 'TEST', 'testPass', '%1', false, false)").arg(session_key));
	QVERIFY2(ok == false, qPrintable("Error expected: user not exist error!"));

	// Try to change one user by another
	//

	QString passSecondUserForTest = "testPass";
	QString userNameSecondUserForTest = "testUser";
	int secondUserId = -1;

	ok = query.exec(QString("SELECT * FROM user_api.create_user('%1', '%2', 'TEST', 'TEST', '%3', false, false)").arg(session_key).arg(userNameSecondUserForTest).arg(passSecondUserForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	secondUserId = query.value(0).toInt();

	// Log out as Administrator and log in as new user
	//

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM user_api.log_in('%1', '%2')").arg(userNameSecondUserForTest).arg(passSecondUserForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	session_key = query.value(0).toString();

	ok = query.exec(QString("SELECT * FROM user_api.update_user('%1', '%2', 'TEST', 'TEST', '%3', 'TESTTEST', false, false)").arg(session_key).arg(oldDataForTest).arg(newDataForTest));
	QVERIFY2(ok == false, qPrintable("Error expected: user must has not rights to update another user!"));

	//	Try to change user flags by user
	//

	ok = query.exec(QString("SELECT * FROM user_api.update_user('%1', '%2', 'TEST', 'TEST', '%3', '%3', true, true)").arg(session_key).arg(userNameSecondUserForTest).arg(passSecondUserForTest));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toInt() == secondUserId, qPrintable ("Error: function returned wrong id"));

	ok = query.exec(QString("SELECT * FROM users WHERE userId = %1").arg(secondUserId));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value("administrator").toBool() == false, qPrintable ("Error: function changed value administrator by user"));
	QVERIFY2(query.value("readOnly").toBool() == true, qPrintable ("Error: function do not changed value readOnly"));
	QVERIFY2(query.value("disabled").toBool() == false, qPrintable ("Error: function changed value disabled y user"));

	// Call wrong password error
	//

	ok = query.exec(QString("SELECT * FROM user_api.update_user('%1', '%2', 'TEST', 'TEST', 'TESTTEST', 'TESTTEST', true, true)").arg(session_key).arg(userNameSecondUserForTest));

	QVERIFY2(ok == false, qPrintable("Error expected: wrong password error!"));

	ok = query.exec("SELECT * FROM user_api.log_out()");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
}
