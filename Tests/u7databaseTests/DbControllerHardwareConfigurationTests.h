#pragma once
#include <QTest>

class DbControllerHardwareConfigurationTests : public QObject
{
	Q_OBJECT

public:
	DbControllerHardwareConfigurationTests(const QString& projectName);

private slots:
	void initTestCase();
	void addAndRemoveDeviceObjectTest();
	void getDeviceTreeLatestVersionTest();
	void cleanupTestCase();

private:
	DbController m_db;
	QString m_projectName;
	QString m_databaseHost;
	int m_databasePort = 5432;
	QString m_databaseUser;
	QString m_databasePassword;
	QString m_adminPassword;
};
