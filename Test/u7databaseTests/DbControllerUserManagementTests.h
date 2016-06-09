#pragma once
#include <QTest>
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
	DbController *m_dbController;
	QString m_databaseHost;
	QString m_databaseName;
	QString m_databaseUser;
	QString m_adminPassword;
};
