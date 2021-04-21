#ifndef TESTDBBASE_H
#define TESTDBBASE_H

#include <QObject>
#include <QTest>
#include <QtSql>
#include "../../DbLib/DbController.h"


struct User
{
	QString username;
	QString password;
	int userId;
};


class TestDbBase : public QObject
{
	Q_OBJECT

protected:
	TestDbBase();

protected slots:
	virtual void initTestCase();
	virtual void cleanupTestCase();

protected:
	bool createProjectDb();
	bool dropProjectDb(QString projectName = QString());

	int createUser(QString sessionKey, QString username, QString password);		// returns user_id

	QString logIn(User user);									// returns session_key
	QString logIn(QString username, QString password);			// returns session_key
	bool logOut();

public:
	QString databaseHost() const;
	void setDatabaseHost(QString value);

	int databaseHostPort() const;
	void setDatabaseHostPort(int value);

	QString databaseUser() const;
	void setDatabaseUser(QString value);

	QString databaseUserPassword() const;
	void setDatabaseUserPassword(QString value);

	QString projectName() const;
	void setProjectName(QString value);

	QString projectAdministratorName() const;
	void setProjectAdministratorName(QString value);

	QString projectAdministratorPassword() const;
	void setProjectAdministratorPassword(QString value);

protected:
	static const int ProjectDatabaseVersion = 230;			// Supported Project DB version

	QString m_databaseHost = "127.0.0.1";
	int m_databaseHostPort = 5432;
	QString m_databaseUser = "u7";
	QString m_databaseUserPassword = "P2ssw0rd";

	QString m_projectName = "testproject";
	QString m_projectAdministratorName = "Administrator";
	QString m_projectAdministratorPassword = "P2ssw0rd";
};

#endif // TESTDBBASE_H
