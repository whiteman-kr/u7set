#pragma once
#include <QTest>
#include "../../lib/DbController.h"

class DbControllerHardwareConfigurationTests : public QObject
{
	Q_OBJECT

public:
	DbControllerHardwareConfigurationTests();

private slots:
	void initTestCase();
	void addAndRemoveDeviceObjectTest();
	void cleanupTestCase();

private:
	DbController *m_dbController;
	QString m_databaseHost;
	QString m_databaseName;
	QString m_databaseUser;
	QString m_adminPassword;
};
