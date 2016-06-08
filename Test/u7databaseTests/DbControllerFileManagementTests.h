#pragma once
#include <QTest>
#include "../../lib/DbController.h"

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
	void deleteFileTest();
	void getFileInfo();
	void checkInTest();
	void checkOutTest();
	void fileHasChildrenTest();
	void getCheckedOutFilesTest();
	void getFileHistoryTest();
	void getLatestFileVersionTest();
	void getLatestTreeVersionTest();
	void getWorkcopyTest();
	void setWorkcopyTest();
	void getSpecificCopyTest();
	void checkInTreeTest();
	void cleanupTestCase();

private:
	DbController *m_dbController;
	QString m_databaseHost;
	QString m_databaseName;
	QString m_databaseUser;
	QString m_adminPassword;
};
