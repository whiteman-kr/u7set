#pragma once
#include <QString>
#include <QTest>
#include <QtSql>
#include <assert.h>

class UserTests : public QObject
{
	Q_OBJECT

public:
	UserTests();

	void setDatabaseHost(QString host);
	void setDatabaseUser(QString userName);
	void setDatabaseUserPassword(QString password);
	void setProjectName(QString projectName);

	void setAdminPassword(QString password);

private slots:
	void initTestCase();
	void cleanupTestCase();
	void logInOutTest();
	void createUserTest();
	void getUserIDTest();
	void isAdminTest();
	void check_user_passwordIntegerTextTest();
	void check_user_passwordTextTextTest();
	void update_userTest();

private:
	QString m_dbHost = "127.0.0.1";
	QString m_dbUser = "u7";
	QString m_dbUserPassword = "P2ssw0rd";
	QString m_adminPassword = "123412341234";
	QString m_projectName = "testproject";
};
