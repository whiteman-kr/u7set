#pragma once
#include <QTest>
#include <memory>
#include "../../lib/DbController.h"

class DbControllerVersionControlTests : public QObject
{
	Q_OBJECT

public:
	DbControllerVersionControlTests();

protected:
	QString logIn(QString username, QString password);		// returns session_key
	bool logOut();

private slots:
	void initTestCase();
	void isAnyCheckedOutTest();
	void lastChangesetIdTest();
	void get_changeset_details();
	void cleanupTestCase();

private:
	std::unique_ptr<DbController> m_db;
	QString m_databaseHost = "127.0.0.1";
	QString m_databaseName = "dbcontrollerversiontesting";
	QString m_databaseUser = "u7";
	QString m_adminPassword = "P2ssw0rd";
};
