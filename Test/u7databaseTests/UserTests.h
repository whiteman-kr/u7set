#pragma once
#include <QString>
#include <QtSql>
#include "TestDbBase.h"

class UserTests : public TestDbBase
{
	Q_OBJECT

public:
	UserTests();

private slots:
	virtual void initTestCase() override;
	virtual void cleanupTestCase() override;

	void logInOutTest();
	void checkSessionKeyTest();
	void createUserTest();
	void currentUserIdTest();
	void isCurrentUserAdminTest();
	void getUserDataTest();
	void check_user_passwordTest();
	void update_userTest();

//private:
//	QString m_dbHost = "127.0.0.1";
//	QString m_dbUser = "u7";
//	QString m_dbUserPassword = "P2ssw0rd";
//	QString m_adminPassword = "P2ssw0rd";
//	QString m_projectName = "testproject";
};
