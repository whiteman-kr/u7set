#pragma once
#include <QTest>
#include "../../lib/DbController.h"

class DbControllerBuildTests : public QObject
{
	Q_OBJECT

public:
	DbControllerBuildTests();

private slots:
	void initTestCase();
	void buidProcessTest();
	void cleanupTestCase();

private:
	DbController *m_dbController;
	QString m_databaseHost;
	QString m_databaseName;
	QString m_databaseUser;
	QString m_adminPassword;
};
