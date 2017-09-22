#pragma once
#include <QTest>
#include <memory>
#include "../../lib/DbController.h"

class DbControllerHardwareConfigurationTests : public QObject
{
	Q_OBJECT

public:
	DbControllerHardwareConfigurationTests();

private slots:
	void initTestCase();
	void addAndRemoveDeviceObjectTest();
	void getDeviceTreeLatestVersionTest();
	void cleanupTestCase();

private:
	std::unique_ptr<DbController> m_db;
	QString m_databaseHost = "127.0.0.1";
	QString m_databaseName = "dbcontrollerbuildtesting";
	QString m_databaseUser = "u7";
	QString m_databasePassword = "P2ssw0rd";
	QString m_adminPassword = "P2ssw0rd";
};
