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
	void createUserTest_data();
	void createUserTest();
	void getUserIDTest_data();
	void getUserIDTest();
	void isAdminTest_data();
	void isAdminTest();

public:

	static bool createUserTest(const QString& parentUser, const QString& userName, const QString& firstName, const QString& lastName, const QString& password, bool isAdmin, bool isReadOnly, bool isDisabled);
	static bool isAdmin(int userID);
	static int getUserIdTest(const QString& login, const QString& password);
};
