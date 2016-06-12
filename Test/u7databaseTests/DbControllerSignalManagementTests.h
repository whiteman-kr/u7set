#pragma once
#include <QTest>
#include "../../lib/DbController.h"

class DbControllerSignalTests : public QObject
{
	Q_OBJECT

public:
	DbControllerSignalTests();

private slots:
	void initTestCase();
	void addSignalTest();
	void getSignalIdsTest();
	void checkInCheckOutSignalsTest();
	void cleanupTestCase();

private:
	DbController *m_dbController;
	QString m_databaseHost;
	QString m_databaseName;
	QString m_databaseUser;
	QString m_adminPassword;
};
