#pragma once
#include <QTest>

class DbController;

class DbControllerVersionControlTests : public QObject
{
	Q_OBJECT

public:
	DbControllerVersionControlTests(const QString& projectName);

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
	QString m_projectName;
	QString m_databaseHost;
	int m_databasePort = 5432;
	QString m_databaseUser;
	QString m_adminPassword;
};
