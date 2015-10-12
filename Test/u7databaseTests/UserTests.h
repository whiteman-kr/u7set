#pragma once
#include <QString>
#include <QTest>

class UserTests : public QObject
{
	Q_OBJECT

public:
	UserTests();

private slots:
	void initTestCase();
	void cleanupTestCase();
	void createUserTest();
	void getUserIDTest();
	void isAdminTest();
	void check_user_passwordIntegerTextTest();
	void check_user_passwordTextTextTest();
	void update_userTest();
};
