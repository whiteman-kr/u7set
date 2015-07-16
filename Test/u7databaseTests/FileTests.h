#pragma once
#include <QString>
#include <QTest>
#include <QtSql>

struct ObjectState
{
	int newFileId;
	bool deleted;
	bool checkedOut;
	int action;
	int userId;
	int errCode;
};

class FileTests : public QObject
{
	Q_OBJECT

public:
	FileTests();

private slots:
	void initTestCase();
	void cleanupTestCase();
	void fileExistsTest_data();
	void fileExistsTest();
	void filesExistTest_data();
	void filesExistTest();
	void is_any_checked_outEmptyTableTest();
	void filesAddTest_data();
	void filesAddTest();
	void is_file_checkedoutTest_data();
	void is_file_checkedoutTest();
	void is_any_checked_outRecordsExistTest();
	/*void file_has_childrenTest_data();
	void file_has_childrenTest();*/
	/*void get_file_stateTest_data();
	void get_file_stateTest();*/
	/*void delete_fileTest_data();
	void delete_fileTest();*/

public:
	static bool fileExists(int fileID);
	static QString filesExist(QString fileID);
	static bool add_file(int userId, QString fileName, int parentId, QString fileData);
	static bool is_file_checkedout(int fileId);
	static bool is_any_checked_out();
	static int file_has_children(int user_id, int fileId);
	static QString delete_file(int user_id, int fileId);
	static QString get_file_state(int fileId);

private:
	static void getObjectState(QSqlQuery& q, ObjectState &os);
};
