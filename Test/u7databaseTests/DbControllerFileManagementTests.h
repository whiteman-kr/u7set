#pragma once
#include <QTest>
#include <memory>
#include "../../lib/DbController.h"

class DbControllerFileTests : public QObject
{
	Q_OBJECT

public:
	DbControllerFileTests();

protected:
	QString logIn(QString username, QString password);		// returns session_key
	bool logOut();

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
	void undoChangestest();
	void systemFilesTest();
	void cleanupTestCase();

private:
	std::unique_ptr<DbController> m_db;
	QString m_databaseHost = "127.0.0.1";
	QString m_databaseName = "dbcontrollerfiletesting";
	QString m_databaseUser = "u7";
	QString m_adminPassword = "P2ssw0rd";
};
