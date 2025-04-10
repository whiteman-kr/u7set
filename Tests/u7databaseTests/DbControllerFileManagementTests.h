#pragma once

class DbController;

class DbControllerFileTests : public QObject
{
	Q_OBJECT

public:
	DbControllerFileTests(const QString& projectName);

protected:
	QString logIn(QString username, QString password); // returns session_key
	bool logOut();

private slots:
	void initTestCase();
	void init();

	void getFileListTest();
	void getFileListTreeTest();
	void addFileTest();
	void addFilesTest();
	void deleteFileTest();
	void getFileInfo();
	void getFileInfo1();
	void getFileInfo2(); // It is basically getFileInfo1, but ARRAY version
	void getFullPathFileInfo();
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
	QString m_projectName;
	QString m_databaseHost;
	int m_databasePort = 5432;
	QString m_databaseUser;
	QString m_adminPassword;

	QString m_user11 = "User11";
	QString m_user22 = "User22";
};
