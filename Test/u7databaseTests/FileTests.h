#pragma once
#include <QString>
#include <QTest>
#include <QtSql>
#include "TestDbBase.h"

struct ObjectState;

struct User
{
	QString username;
	QString password;
	int userId;
};

class FileTests : public TestDbBase
{
	Q_OBJECT

public:
	FileTests();

protected:
	QString logIn(User user);								// returns session_key
	QString logIn(QString username, QString password);		// returns session_key
	bool logOut();

private slots:
	void initTestCase();
	void cleanupTestCase();

	void api_is_file_exists();
	void api_set_file_attributes();
	void api_add_file();
	void api_move_file();
	void api_undo_changes_after_move_file();
	void api_rename_file();
	void api_undo_changes_after_rename_file();

	void fileExistsTest();
	void filesExistTest_data();
	void filesExistTest();
	void api_is_any_checked_out();
	void filesAddTest_data();
	void filesAddTest();
	void is_file_checkedout();
	void file_has_children();
	void delete_fileTest_data();
	void delete_fileTest();
	void check_inTest();
	void check_outTest();
	void set_workcopyTest();
	void get_workcopyTest();
	void get_file_historyTest();
	void get_file_stateTest();
	void get_last_changesetTest();
	void get_file_IdIntegerTextTest();
	void get_file_IdIntegerIntegerTextTest();
	void get_file_infoTest();
	void get_latest_file_versionTest();
	void get_file_listIntegerIntegerTextTest();
	void get_file_listIntegerIntegerTest();
	void get_file_list_tree_test();
	void get_latest_file_tree_versionTest();
	void undo_changesTest();
	void add_or_update_fileTest();
	void get_specific_copyTest();
	void add_deviceTest();
	void get_checked_out_filesTest();
	void check_in_treeTest();

public:
	static QString filesExist(QString fileID);
	static bool add_file(int userId, QString fileName, int parentId, QString fileData, QString details);
	static bool delete_file(int userId, int fileId);

	User m_user1;
	User m_user2;

	static const int maxValueId = 9999999;

private:
	static void getObjectState(QSqlQuery& q, ObjectState& os);
};
