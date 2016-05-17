#pragma once
#include <QTest>
#include "../../include/DbController.h"

class DbControllerFileTests : public QObject
{
	Q_OBJECT

public:
	DbControllerFileTests();

private slots:
	void initTestCase();
	void getFileListTest();
	void addFileTest();
	void addFilesTest();
	void cleanupTestCase();

private:
	DbController *m_dbController;
	QString m_databaseHost;
	QString m_databaseName;
	QString m_databaseUser;
	QString m_adminPassword;
};
