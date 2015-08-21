#pragma once
#include <QString>
#include <QTest>
#include <QtSql>

class ObjectState;

class FileTests : public QObject
{
	Q_OBJECT

public:
	FileTests();

private slots:
	void initTestCase();
	void cleanupTestCase();
	void fileExistsTest();
	void filesExistTest_data();
	void filesExistTest();
	void is_any_checked_outTest();
	void filesAddTest_data();
	void filesAddTest();
	void is_file_checkedoutTest_data();
	void is_file_checkedoutTest();
	void file_has_childrenTest_data();
	void file_has_childrenTest();
	void delete_fileTest_data();
	void delete_fileTest();
	void check_inTest();
	void check_outTest();
	void set_workcopyTest();
	void get_workcopyTest();
	void get_file_historyTest();
	void get_file_stateTest();
	void get_last_changesetTest();
	void get_file_IdIntegetTextTest();
	void get_file_IdIntegetrIntegerTextTest();
	void get_file_infoTest();
	void get_latest_file_versionTest();
	void get_file_listTest();
	void get_latest_file_tree_versionTest();
	void undo_changesTest();
	void add_or_update_fileTest();
	void get_specific_copyTest();
	void add_deviceTest();

public:
	static bool fileExists(int fileID);
	static QString filesExist(QString fileID);
	static bool add_file(int userId, QString fileName, int parentId, QString fileData);
	static bool is_file_checkedout(int fileId);
	static int file_has_children(int user_id, int fileId);
	static bool delete_file(int userId, int fileId);

	int m_firstUserForTest = -1;
	int m_secondUserForTest = -1;
	static const int maxValueId = 9999999;

private:
	static void getObjectState(QSqlQuery& q, ObjectState& os);
};
