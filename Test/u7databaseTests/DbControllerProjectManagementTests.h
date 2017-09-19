#pragma once
#include <QTest>
#include <memory>
#include "../../lib/DbController.h"

class DbControllerProjectTests : public QObject
{
	Q_OBJECT

public:
	DbControllerProjectTests();
	void setProjectVersion(int version);

private slots:
	void initTestCase();
	void createOpenUpgradeCloseDeleteProject();
	void getProjectListTest();
	void ProjectPropertyTest();
	void isProjectOpenedTest();
	void connectionInfoTest();
	void cleanupTestCase();

private:
	std::unique_ptr<DbController> m_db;
	int m_databaseVersion = -1;
	QString m_databaseHost = {"127.0.0.1"};
	QString m_databaseName = {"dbcontrollertesting"};
	QString m_databaseUser = {"u7"};
	QString m_adminPassword = {"P2ssw0rd"};
	int m_databasePort = 5432; // Current port by default. Not a magic number
};
