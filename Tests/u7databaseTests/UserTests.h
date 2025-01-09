#pragma once
#include "TestDbBase.h"
#include <QTest>

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
};
