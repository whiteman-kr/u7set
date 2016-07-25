#pragma once
#include <QTest>
#include "../../lib/DbController.h"

class DbControllerVersionControlTests : public QObject
{
	Q_OBJECT

public:
	DbControllerVersionControlTests();

private slots:
	void initTestCase();
	void isAnyCheckedOutTest();
	void lastChangesetIdTest();
	void cleanupTestCase();

private:
	DbController *m_dbController;
	QString m_databaseHost;
	QString m_databaseName;
	QString m_databaseUser;
	QString m_adminPassword;
};
