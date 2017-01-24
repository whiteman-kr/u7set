#include "MultiThreadFileTest.h"
#include <QtSql>
#include <vector>
#include <QDebug>
#include <assert.h>

MultiThreadFileTest::MultiThreadFileTest(int number,
								 const char* dbHost,
								 const char* dbUser,
								 const char* dbUserPassword,
								 const char* name,
								 int amountOfFiles) :
	m_threadNumber(number),
	m_databaseHost(dbHost),
	m_databaseUser(dbUser),
	m_databaseUserPassword(dbUserPassword),
	m_projectName(name),
	m_amountOfFileIds(amountOfFiles)
{
	assert(dbHost);
	assert(dbUser);
	assert(dbUserPassword);
	assert(name);
}

MultiThreadFileTest::~MultiThreadFileTest()
{
	//qDebug() << "MultiThreadTest::~MultiThreadTest() " << m_threadNumber;
}

void MultiThreadFileTest::run()
{
	// Create new database connection for thread to work with
	//

	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "fileThread_" + QString::number(m_threadNumber));

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_databaseUserPassword);
	db.setDatabaseName(QString("u7_") + m_projectName);

	if (db.open() == false)
	{
		qDebug() << "Error: Database not open. " << db.lastError().databaseText();
		this->terminate();
	}

	QSqlQuery query(db);

	// Create new user for each thread
	//

	bool ok = query.exec("SELECT salt FROM users WHERE username = 'Administrator'");

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		this->terminate();
	}

	if (query.next() == false)
	{
		qDebug() << query.lastError().databaseText();
		this->terminate();
	}

	QString passwordHashQuery = QString("user_api.password_hash('%1', '%2')").arg(query.value(0).toString()).arg(m_databaseUserPassword);

	ok = query.exec(QString("UPDATE users SET passwordhash = %1 WHERE username = 'Administrator'").arg(passwordHashQuery));

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		this->terminate();
	}

	ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_databaseUserPassword));

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		this->terminate();
	}

	if (query.next() == false)
	{
		qDebug() << query.lastError().databaseText();
		this->terminate();
	}

	QString session_key = query.value(0).toString();

	ok = query.exec(QString("SELECT * FROM user_api.create_user('%1', '%2', '%3', '%4', '%5', false, false)")
	                     .arg(session_key)
						 .arg("fileThread_" + QString::number(m_threadNumber))
						 .arg("fileThread_" + QString::number(m_threadNumber))
						 .arg("fileThread_" + QString::number(m_threadNumber))
						 .arg("fileThread_" + QString::number(m_threadNumber)));

	ok = query.exec("SELECT * FROM user_api.log_out()");

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		this->terminate();
	}

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		this->terminate();
	}

	ok = query.next();

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		this->terminate();
	}

	// Remember id of created user
	//

	int userId = query.value(0).toInt();
	int fileNumber = 0;						// Value, that will count amount of files
	int numberOfErrorsInWorkTest = 0;		// Number of errors, which can be occured in one test
	int totalErrors = 0;					// Total count of errors

	bool error = false;						// Value, that store info of error

	std::vector<int> fileIds;	// Array of fileIds of created files
	fileIds.reserve(m_amountOfFileIds);

	// Start add_file function testing
	//

	for (fileNumber = 0; fileNumber < m_amountOfFileIds; fileNumber++)
	{
		ok = query.exec(QString("SELECT * FROM add_file(%1, '%2', 1, '%3', '{}')")
						.arg(userId)
						.arg("fileThread_" + QString::number(m_threadNumber) + "_" + "number" + QString::number(fileNumber))
						.arg("fileThread_" + QString::number(m_threadNumber) + "_" + "number" + QString::number(fileNumber)));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.next() == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		// Check file, which was created
		//

		ok = query.exec(QString("SELECT * FROM file WHERE fileId = %1")
						.arg(query.value(0).toInt()));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.next() == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		// Write fileId to array
		//

		fileIds.push_back(query.value(0).toInt());

		// Show errors from test
		//

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " Add_file " << fileNumber << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: add_file() " << m_amountOfFileIds << "files";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: add_file() " << m_amountOfFileIds << "files";
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	// Start check_in function test
	//

	for (int fileId : fileIds)
	{
		ok = query.exec(QString("SElECT * FROM check_in (%1, '{%2}', 'TEST');")
						.arg(userId)
						.arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			qDebug() << "Thread_" << m_threadNumber << " check_in(): " << fileId << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	// Check that files were correctly checked in
	//

	for (int fileId : fileIds)
	{
		ok = query.exec(QString("SELECT * FROM get_file_state(%1)")
						.arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.next() == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.value("id").toInt() != fileId)
		{
			qDebug() << "Error: wrong fileId after check_in() function\nThread: " << m_threadNumber << "FileId: " << fileId;
			error = true;
		}

		if (query.value("checkedOut").toBool() == true)
		{
			qDebug() << "Error: File was not checked in after check_in() function\nThread: " << m_threadNumber << "FileId: " << fileId;
			error = true;
		}

		if (query.value("deleted").toBool() == true)
		{
			qDebug() << "Error: File deleted after check_in() function\nThread: " << m_threadNumber << "FileId: " << fileId;
			error = true;
		}

		if (query.value("action").toInt() != 1)
		{
			qDebug() << "Error: Wrong action after check_in() function\nThread: " << m_threadNumber << "FileId: " << fileId;
			error = true;
		}

		if (query.value("userId").toInt() != userId)
		{
			qDebug() << "Error: Wrong user_id after check_in() function\nThread: " << m_threadNumber << "FileId: " << fileId;
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " get_file_info() after check_in(): " << fileId << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: check_in() " << m_amountOfFileIds << "files";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: check_in() " << m_amountOfFileIds << "files";
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	// Start check_out files test
	//

	for (int fileId : fileIds)
	{
		bool ok = query.exec(QString("SElECT * FROM check_out (%1, '{%2}');")
							 .arg(userId)
							 .arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			qDebug() << "Thread_" << m_threadNumber << " check_out() : " << fileId << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
	}

	// Check files were correctly checked out
	//

	for (int fileId : fileIds)
	{
		ok = query.exec(QString("SELECT * FROM get_file_state(%1)")
						.arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.next() == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.value("id").toInt() != fileId)
		{
			qDebug() << "Error: wrong fileId after check_out() function\nThread: " << m_threadNumber << "FileId: " << fileId;
			error = true;
		}

		if (query.value("checkedOut").toBool() == false)
		{
			qDebug() << "Error: File was checked in after check_out() function\nThread: " << m_threadNumber << "FileId: " << fileId;
			error = true;
		}

		if (query.value("deleted").toBool() == true)
		{
			qDebug() << "Error: File deleted after check_out() function\nThread: " << m_threadNumber << "FileId: " << fileId;
			error = true;
		}

		if (query.value("action").toInt() != 2)
		{
			qDebug() << "Error: Wrong action after check_out() function\nThread: " << m_threadNumber << "FileId: " << fileId;
			error = true;
		}

		if (query.value("userId").toInt() != userId)
		{
			qDebug() << "Error: Wrong user_id after check_out() function\nThread: " << m_threadNumber << "FileId: " << fileId;
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " get_file_info() after check_out(): " << fileId << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: check_out() " << m_amountOfFileIds << "files";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: check_out() " << m_amountOfFileIds << "files";
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	// Start set_workcopy() test (write data to files)
	//

	for (int fileId : fileIds)
	{
		ok = query.exec(QString("SELECT * FROM set_workcopy(%1, %2, '%3', '{}')")
						.arg(userId)
						.arg(fileId)
						.arg(QString("fileThread_%1_fileId_%2 testingData")
						.arg(m_threadNumber)
						.arg(fileId)));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << "  set_workcopy() : " << fileId << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: set_workcopy() " << m_amountOfFileIds << "files";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: set_workcopy() " << m_amountOfFileIds << "files";
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	// Start get_workcopy() test (read data from files)
	//

	for (int fileId : fileIds)
	{
		bool ok = query.exec(QString("SELECT * FROM get_workcopy(%1, %2)")
							 .arg(userId)
							 .arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.next() == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.value("fileId").toInt() != fileId)
		{
			qDebug() << "Error: fileId is not match after function get_workcopy()\nThread: " << m_threadNumber << "FileId" << fileId;
			error = true;
		}

		if (query.value("data").toString() != QString("fileThread_%1_fileId_%2 testingData").arg(m_threadNumber).arg(fileId))
		{
			qDebug() << "Error: fileId is not match after function get_workcopy()\nThread: " << m_threadNumber << "FileId" << fileId;
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " get_workcopy() : " << fileId << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: get_workcopy() " << m_amountOfFileIds << "files";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: get_workcopy() " << m_amountOfFileIds << "files";
	}

	// Start get_file_info() test
	//

	for (int fileId : fileIds)
	{
		ok = query.exec(QString("SELECT * FROM get_file_info(%1, '{%2}')")
						.arg(userId)
						.arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.next() == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.value("fileId").toInt() != fileId)
		{
			qDebug() << "Error: wrong fileId in get_file_info() function\nThread: " << m_threadNumber << "\nFileId: " << fileId;
			error = true;
		}
	}

	//Start get_latest_file_version() test
	//

	fileNumber = 0;

	for (int fileId : fileIds)
	{
		ok = query.exec(QString("SELECT * FROM get_latest_file_version(%1, %2);")
						.arg(userId)
						.arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.next() == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.value("fileId").toInt() != fileId)
		{
			qDebug() << "Error: wrong fileId in get_latest_file_version() function\nThread: " << m_threadNumber << "\nFileId: " << fileId;
			error = true;
		}

		if (query.value("deleted").toBool() == true)
		{
			qDebug() << "Error: File matched as deleted in get_latest_file_version() function\nThread: " << m_threadNumber << "\nFileId: " << fileId;
			error = true;
		}

		if (query.value("name").toString() != QString("fileThread_" + QString::number(m_threadNumber) + "_" + "number" + QString::number(fileNumber)))
		{
			qDebug() << "Error: wrong name in get_latest_file_version() function\nThread: " << m_threadNumber << "\nFileId: " << fileId;
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " get_latest_file_version() : " << fileId << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
		fileNumber++;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: get_latest_file_version() " << m_amountOfFileIds << "files";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: get_latest_file_version() " << m_amountOfFileIds << "files";
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	// Start is_file_checkedout() test
	//

	for (int fileId: fileIds)
	{
		ok = query.exec(QString("SELECT * FROM is_file_checkedOut(%1)").arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.next() == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.value(0).toBool() == false)
		{
			qDebug() << "Error: file must be checked out\nThread: " << m_threadNumber << "FileId" << fileId;
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " Result of work simulation: " << fileId << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: is_file_checkedout() " << m_amountOfFileIds << "files";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: is_file_checkedout() " << m_amountOfFileIds << "files";
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	// Start delete_file() test
	//

	for (int fileId : fileIds)
	{
		ok = query.exec(QString("SELECT * FROM delete_file(%1, %2)")
						.arg(userId)
						.arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			qDebug() << "Thread_" << m_threadNumber << " Result of work simulation: " << fileId << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
	}

	// Check result of delete_file() test
	//

	for (int fileId : fileIds)
	{
		ok = query.exec(QString("SELECT * FROM get_file_state(%1)")
						.arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.next() == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.value("action").toInt() != 3)
		{
			qDebug() << "Error: wrong action after delete_file() function()\nThread: " << m_threadNumber << "FileId" << fileId;
			error = true;
		}

		// Check in file, to check result in table 'file'
		//

		ok = query.exec(QString("SElECT * FROM check_in (%1, '{%2}', 'TEST');").arg(userId).arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		ok = query.exec(QString("SELECT * FROM get_file_state(%1)").arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.next() == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.value("deleted").toBool() == false)
		{
			qDebug() << "Error: file was not deleted after check_in() function()\nThread: " << m_threadNumber << "FileId" << fileId;
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " Result of work simulation: " << fileId << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	if (totalErrors == 0)
	{
		qDebug() << "PASS   : Thread # " << m_threadNumber;
	}
	else
	{
		qDebug() << "FAIL   : Thread # " << m_threadNumber << ": Total " << totalErrors;
	}

	db.close();
}
