#include "MultiThreadSignalTest.h"
#include <QtSql>
#include <vector>
#include <QDebug>
#include <assert.h>

MultiThreadSignalTest::MultiThreadSignalTest(int number,
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
	m_amountOfSignalIds(amountOfFiles)
{
	assert(dbHost);
	assert(dbUser);
	assert(dbUserPassword);
	assert(name);
}

MultiThreadSignalTest::~MultiThreadSignalTest()
{
	//qDebug() << "MultiThreadTest::~MultiThreadTest() " << m_threadNumber;
}

void MultiThreadSignalTest::run()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "sgnalThread_" + QString::number(m_threadNumber));

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
						 .arg("SgnalThread_" + QString::number(m_threadNumber))
						 .arg("SgnalThread_" + QString::number(m_threadNumber))
						 .arg("SgnalThread_" + QString::number(m_threadNumber))
						 .arg("SgnalThread_" + QString::number(m_threadNumber)));
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
	int signalNumber = 0;						// Value, that will count amount of files
	int numberOfErrorsInWorkTest = 0;		// Number of errors, which can be occured in one test
	int totalErrors = 0;					// Total count of errors

	bool error = false;						// Value, that store info of error

	std::vector<int> signalIds;	// Array of fileIds of created files
	signalIds.reserve(m_amountOfSignalIds);

	for (signalNumber = 0; signalNumber < m_amountOfSignalIds; signalNumber++)
	{
		ok = query.exec(QString("SELECT * FROM add_signal(%1, 1, 3)").arg(userId));

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

		if (query.value("errCode").toInt() != 0)
		{
			qDebug() << "Error: errcode is " << query.value("errCode").toInt();
			error = true;
		}

		if (query.value("userId").toInt() != userId)
		{
			qDebug() << "Error: wrong user";
			error = true;
		}

		signalIds.push_back(query.value(0).toInt());

		// Show errors from test
		//

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " Add_signal " << signalNumber << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: add_signal() " << m_amountOfSignalIds << "signals";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: add_signal() " << m_amountOfSignalIds << "signals";
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	/*for (signalId : signalIds)
	{

	}*/

	db.close();
}
