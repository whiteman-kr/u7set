#pragma once
#include <QTest>
#include "../../include/DbController.h"

class DbControllerProjectTests : public QObject
{
	Q_OBJECT

public:
	DbControllerProjectTests();

private slots:
	void initTestCase();
	void createOpenUpgradeCloseDeleteProject();
	void getProjectListTest();
	void cleanupTestCase();

private:
	DbController *m_dbController;
	QString m_databaseHost;
	QString m_databaseName;
	QString m_databaseUser;
	QString m_adminPassword;
};
