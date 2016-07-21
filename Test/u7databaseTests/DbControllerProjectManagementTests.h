#pragma once
#include <QTest>
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
	DbController *m_dbController;
	int m_databaseVersion = -1;
	QString m_databaseHost;
	QString m_databaseName;
	QString m_databaseUser;
	QString m_adminPassword;
	int m_databasePort = 5432; // Current port by default. Not a magic number
};
