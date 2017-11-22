#pragma once
#include <QTest>
#include <memory>
#include "../../lib/DbController.h"

class DbControllerUserTests : public QObject
{
	Q_OBJECT

public:
	DbControllerUserTests();

private slots:
	void initTestCase();
	void createUserTest();
	void updateUserTest();
	void getUserListTest();
	void cleanupTestCase();

private:
	std::unique_ptr<DbController> m_db;
	QString m_databaseHost = "127.0.0.1";
	QString m_databaseName = "dbcontrollerusertestsproject";
	QString m_databaseUser = "u7";
	QString m_adminPassword = "P2ssw0rd";
};
