#include "MultiThreadTest.h"
#include <QtSql>
#include <vector>
#include <QDebug>
#include <assert.h>

MultiThreadTest::MultiThreadTest(int number,
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
	Q_ASSERT(dbUser != "");
	Q_ASSERT(dbUserPassword != "");
	Q_ASSERT(name != "");
}

MultiThreadTest::~MultiThreadTest()
{
	//qDebug() << "MultiThreadTest::~MultiThreadTest() " << m_threadNumber;
}

void MultiThreadTest::run()
{
	// Create new database connection for thread to work with
	//

	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "Thread_" + QString::number(m_threadNumber));

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

	bool ok = query.exec(QString("SELECT * FROM create_user(1, '%1', '%2', '%3', '%4', false, false, false)")
						 .arg("Thread_" + QString::number(m_threadNumber))
						 .arg("Thread_" + QString::number(m_threadNumber))
						 .arg("Thread_" + QString::number(m_threadNumber))
						 .arg("Thread_" + QString::number(m_threadNumber)));
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
	}
	ok = query.next();
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
	}

	// Remember id of created user
	//

	int userId = query.value(0).toInt();

	int fileNumber = 0;						// Value, that will count amount of files
	bool error = false;						// Value, that store info of error
	int numberOfErrorsInAddTest = 0;		// Number of errors, which can be occured

	std::vector<int> fileIds;	// Array of fileIds of created files
	fileIds.reserve(m_amountOfFileIds);

	// Start cycle, where function will create files
	//

	qDebug() << "Thread " << m_threadNumber << " started add_file() procedure for " << m_amountOfFileIds << "times";

	for (fileNumber = 0; fileNumber < m_amountOfFileIds; fileNumber++)
	{
		ok = query.exec(QString("SELECT * FROM add_file(%1, '%2', 1, '%3', '{}')")
						.arg(userId).arg("thread" + QString::number(m_threadNumber) + "_" + "number" + QString::number(fileNumber))
						.arg("thread" + QString::number(m_threadNumber) + "_" + "number" + QString::number(fileNumber)));

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

		// Remember fileId to array
		//

		fileIds.push_back(query.value(0).toInt());

		// Show result of function
		//

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " Add_file" << fileNumber << ": ERROR";
			numberOfErrorsInAddTest++;
		}
		error = false;
	}

	qDebug() << "Thread " << m_threadNumber << " ended add_file() procedure with " << numberOfErrorsInAddTest << "errors";

	// Erase all error data for new test
	//

	int numberOfErrorsInWorkTest = 0;

	qDebug() << "Thread " << m_threadNumber << " started work simulation procedure with " << m_amountOfFileIds << "files";

	for (int fileId : fileIds)
	{
		// Check in and check out file to make history
		//

		ok = query.exec(QString("SElECT * FROM check_in (%1, '{%2}', 'TEST');")
						.arg(userId)
						.arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

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

		ok = query.exec(QString("SElECT * FROM check_out (%1, '{%2}');").arg(userId).arg(fileId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

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

		// Try set_workcopy() and get_workcopy functions()
		//

		ok = query.exec(QString("SELECT * FROM set_workcopy(%1, %2, '%3', '{}')").arg(userId).arg(fileId).arg(QString("Thread_%1_fileId_%2 testingData").arg(m_threadNumber).arg(fileId)));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		ok = query.exec(QString("SELECT * FROM get_workcopy(%1, %2)").arg(userId).arg(fileId));

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

		if (query.value("data").toString() != QString("Thread_%1_fileId_%2 testingData").arg(m_threadNumber).arg(fileId))
		{
			qDebug() << "Error: fileId is not match after function get_workcopy()\nThread: " << m_threadNumber << "FileId" << fileId;
			error = true;
		}

		// Delete file, and check result from table fileInstance
		//

		ok = query.exec(QString("SELECT * FROM delete_file(%1, %2)").arg(userId).arg(fileId));

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

	//qDebug() << "Thread_" << threadNumber << ": has been ended testing work simulation with " << numberOfErrorsInWorkTest << " errors";
	if (numberOfErrorsInAddTest == 0 && numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread # " << m_threadNumber;
	}
	else
	{
		qDebug() << "FAIL   : Thread # " << m_threadNumber << ": File creation errors - " << numberOfErrorsInAddTest << ", File processing errors - " << numberOfErrorsInWorkTest;
	}

	db.close();
}
