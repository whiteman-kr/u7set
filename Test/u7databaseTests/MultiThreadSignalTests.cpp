#include "MultiThreadSignalTests.h"
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
	int signalNumber = 0;						// Value, that will count amount of signals
	int numberOfErrorsInWorkTest = 0;		// Number of errors, which can be occured in one test
	int totalErrors = 0;					// Total count of errors

	bool error = false;						// Value, that store info of error

	std::vector<int> signalIds;	// Array of fileIds of created signals
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

	for (int signalId : signalIds)
	{
		ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(userId).arg(signalId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.first() == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.value("signalId").toInt() != signalId)
		{
			qDebug() << "Error: signalId not match\nThread: " << m_threadNumber << "\nSignalId: " << signalId;
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " get_latest_signal() " << signalNumber << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: get_latest_signal() " << m_amountOfSignalIds << "signals";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: get_latest_signal() " << m_amountOfSignalIds << "signals";
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	// Write data to signal
	//

	signalData sd;
	QString arguments;

	for (int signalId : signalIds)
	{
		arguments = "";

		ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2);").arg(userId).arg(signalId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (query.first() == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		int checkedOutInstanceId = query.value("signalInstanceId").toInt();

		// Values, that not used in function, but required
		//

		sd.signalGroupId = 0;
		sd.changeSetId = 0;
		sd.checkedOut = "false";
		sd.userId = 0;
		sd.channel = 0;
		sd.type =0;
		sd.created = "2015-09-15 00:00:00.811+03";
		sd.deleted = "false";
		sd.instanceCreated = "2015-09-15 00:00:00.811+03";
		sd.action = 0;

		// Fill values that function use
		//

		sd.signalId = signalId;
		sd.signalInstanceId = checkedOutInstanceId;
		sd.strId = "thread_" + QString::number(m_threadNumber) + " signalId_" + QString::number(signalId);
		sd.extStrId = "thread_" + QString::number(m_threadNumber) + " signalId_" + QString::number(signalId);
		sd.name = "thread_" + QString::number(m_threadNumber) + " signalId_" + QString::number(signalId);
		sd.dataFormatId = 1;
		sd.dataSize = 2;
		sd.lowAdc = 3;
		sd.highAdc = 4;
		sd.lowLimit = 5;
		sd.highLimit = 6;
		sd.unitId = 7;
		sd.adjustment = 8;
		sd.dropLimit = 9;
		sd.excessLimit = 10;
		sd.unbalanceLimit = 11;
		sd.inputLowLimit = 12;
		sd.inputHighLimit = 13;
		sd.inputUnitId = 14;
		sd.inputSensorId = 15;
		sd.outputLowLimit = 16;
		sd.outputHighLimit = 17;
		sd.outputUnitId = 18;
		sd.outputSensorId = 19;
		sd.acquire = "true";
		sd.calculated = "true";
		sd.normalState = 20;
		sd.decimalPlaces = 21;
		sd.aperture = 22;
		sd.inOutType = 23;
		sd.deviceStrId = "dviceStrId";
		sd.outputRangeMode = 24;
		sd.filteringTime = 25;
		sd.maxDifference = 26;
		sd.byteOrder = 27;

		QString arguments = QString("%1, %2, %3, %4, %5, %6, %7, %8, '%9', %10, ")
				.arg(sd.signalId)
				.arg(sd.signalGroupId)
				.arg(sd.changeSetId)
				.arg(sd.signalInstanceId)
				.arg(sd.checkedOut)
				.arg(sd.userId)
				.arg(sd.channel)
				.arg(sd.type)
				.arg(sd.created)
				.arg(sd.deleted);

		arguments.append(QString("'%1', %2, '%3', '%4', '%5', %6, %7, %8, %9, %10, ")
						 .arg(sd.instanceCreated)
						 .arg(sd.action)
						 .arg(sd.strId)
						 .arg(sd.extStrId)
						 .arg(sd.name)
						 .arg(sd.dataFormatId)
						 .arg(sd.dataSize)
						 .arg(sd.lowAdc)
						 .arg(sd.highAdc)
						 .arg(sd.lowLimit));

		arguments.append(QString("%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, ")
						 .arg(sd.highLimit)
						 .arg(sd.unitId)
						 .arg(sd.adjustment)
						 .arg(sd.dropLimit)
						 .arg(sd.excessLimit)
						 .arg(sd.unbalanceLimit)
						 .arg(sd.inputLowLimit)
						 .arg(sd.inputHighLimit)
						 .arg(sd.inputUnitId)
						 .arg(sd.inputSensorId));

		arguments.append(QString("%1, %2, %3, %4, %5, %6, %7, %8, %9, %10, ")
						 .arg(sd.outputLowLimit)
						 .arg(sd.outputHighLimit)
						 .arg(sd.outputUnitId)
						 .arg(sd.outputSensorId)
						 .arg(sd.acquire)
						 .arg(sd.calculated)
						 .arg(sd.normalState)
						 .arg(sd.decimalPlaces)
						 .arg(sd.aperture)
						 .arg(sd.inOutType));

		arguments.append(QString("'%1', %2, %3, %4, %5")
						 .arg(sd.deviceStrId)
						 .arg(sd.outputRangeMode)
						 .arg(sd.filteringTime)
						 .arg(sd.maxDifference)
						 .arg(sd.byteOrder));

		// Start function
		//

		ok = query.exec(QString("SELECT * FROM set_signal_workcopy(1, ROW(%1))").arg(arguments));

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

		if (query.value("ErrCode").toInt() != 0)
		{
			qDebug() << "Error code is " << query.value("errCode").toInt();
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " set_signal_workcopy() " << signalNumber << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: set_signal_workcopy() " << m_amountOfSignalIds << "signals";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: set_signal_workcopy() " << m_amountOfSignalIds << "signals";
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	for (int signalId : signalIds)
	{
		ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2)").arg(userId).arg(signalId));

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

		if (query.value("strId") != QString("thread_" + QString::number(m_threadNumber) + " signalId_" + QString::number(signalId)))
		{
			qDebug() << "Error in data read-write: wrong strId\nThread: " << m_threadNumber << "\nSigal: " << signalId;
			error = true;
		}

		if (query.value("extStrId") != QString("thread_" + QString::number(m_threadNumber) + " signalId_" + QString::number(signalId)))
		{
			qDebug() << "Error in data read-write: wrong extStrId\nThread: " << m_threadNumber << "\nSigal: " << signalId;
			error = true;
		}

		if (query.value("name") != QString("thread_" + QString::number(m_threadNumber) + " signalId_" + QString::number(signalId)))
		{
			qDebug() << "Error in data read-write: wrong name\nThread: " << m_threadNumber << "\nSigal: " << signalId;
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " get_latest_signal " << signalNumber << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: get_latest_signal() " << m_amountOfSignalIds << "signals";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: get_latest_signal() " << m_amountOfSignalIds << "signals";
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	// Check_in test
	//

	for (int signalId : signalIds)
	{
		ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'Thread %3, signal %2');").arg(userId).arg(signalId).arg(m_threadNumber));

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
			qDebug() << "Error: errCode is " << query.value("errCode").toInt();
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " get_latest_signal " << signalNumber << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: checkin_signals() " << m_amountOfSignalIds << "signals";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: checkin_signals() " << m_amountOfSignalIds << "signals";
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	for (int signalId : signalIds)
	{
		ok = query.exec(QString("SELECT * FROM checkout_signals(%1, '{%2}');").arg(userId).arg(signalId));

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
			qDebug() << "Error: errCode is " << query.value("errCode").toInt();
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " get_latest_signal " << signalNumber << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: checkout_signals() " << m_amountOfSignalIds << "signals";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: checkout_signals() " << m_amountOfSignalIds << "signals";
	}

	totalErrors += numberOfErrorsInWorkTest;
	numberOfErrorsInWorkTest = 0;

	for (int signalId : signalIds)
	{
		ok = query.exec(QString("SELECT * FROM delete_signal(%1, %2);").arg(userId).arg(signalId));

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
			qDebug() << "Error: errCode is " << query.value("errCode").toInt();
			error = true;
		}

		if (error)
		{
			qDebug() << "Thread_" << m_threadNumber << " delete_signal() " << signalNumber << ": ERROR";
			numberOfErrorsInWorkTest++;
		}
		error = false;
	}

	if (numberOfErrorsInWorkTest == 0)
	{
		qDebug() << "PASS   : Thread " << m_threadNumber << ":: delete_signal() " << m_amountOfSignalIds << "signals";
	}
	else
	{
		qDebug() << "FAIL   : Thread " << m_threadNumber << ":: delete_signal() " << m_amountOfSignalIds << "signals";
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

MultiThreadSignalStressTest :: MultiThreadSignalStressTest(const char* dbHost,
														   const char* dbUser,
														   const char* dbUserPassword,
														   const char* name,
														   const int mode,
														   const int userIdSignalCreator,
														   std::vector<int>& signalIds) :
	m_mode(mode),
	m_signalIds(signalIds),
	m_databaseHost(dbHost),
	m_databaseUser(dbUser),
	m_databaseUserPassword(dbUserPassword),
	m_projectName(name),
	m_userIdSignalCreator(userIdSignalCreator)
{
	assert(dbHost);
	assert(dbUser);
	assert(dbUserPassword);
	assert(name);
}

int MultiThreadSignalStressTest::create_user(const char* dbHost,
											 const char* dbUser,
											 const char* dbUserPassword,
											 const char* name)
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "sgnalStressCreateUser");

	db.setHostName(dbHost);
	db.setUserName(dbUser);
	db.setPassword(dbUserPassword);
	db.setDatabaseName(QString("u7_") + name);

	if (db.open() == false)
	{
		qDebug() << "Error: Database not open. " << db.lastError().databaseText();
		return -1;
	}

	QSqlQuery query(db);

	bool ok = query.exec("SELECT * FROM create_user(1, 'MultiThreadSignalStressTestUser', 'MultiThreadSignalStressTestUser', 'MultiThreadSignalStressTestUser', 'MultiThreadSignalStressTestUser', false, false, false)");
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		db.close();
		return -1;
	}
	if (query.next() == false)
	{
		qDebug() << query.lastError().databaseText();
		db.close();
		return -1;
	}

	db.close();
	return query.value(0).toInt();
}

int MultiThreadSignalStressTest::fillSignalIdsVector(std::vector<int>& signalIds,
													 int userId,
													 int signalAmount,
													 const char* dbHost,
													 const char* dbUser,
													 const char* dbUserPassword,
													 const char* name)
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "sgnalStressfillSignalIdsVector");

	db.setHostName(dbHost);
	db.setUserName(dbUser);
	db.setPassword(dbUserPassword);
	db.setDatabaseName(QString("u7_") + name);

	if (db.open() == false)
	{
		qDebug() << "Error: Database not open. " << db.lastError().databaseText();
		return -1;
	}

	QSqlQuery query(db);
	signalIds.reserve(signalAmount);

	for (int numberOfSignalId = 0; numberOfSignalId < signalAmount; numberOfSignalId ++)
	{

		bool ok = query.exec(QString("SELECT * FROM add_signal (%1, 1, 4)").arg(userId));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			db.close();
			return -1;
		}

		if (query.next() == false)
		{
			qDebug() << query.lastError().databaseText();
			db.close();
			return -1;
		}

		signalIds.push_back(query.value("id").toInt());
	}

	db.close();
	return 0;
}

void MultiThreadSignalStressTest::run()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "SignalStressTestMode_" + QString::number(m_mode));

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
	bool error = false;
	int numberOfErrors = 0;
	int signalNumber = 0;

	if (m_mode == 0)						// Start checkin_checkout test
	{
		for (int signalId : m_signalIds)	// Try every signal
		{
			for (int numberOfCycleToMakeStressTest = 0; numberOfCycleToMakeStressTest < 5; numberOfCycleToMakeStressTest++)
			{
				// To make sure, that signal has been checked by function get_latest_signal() at least once,
				// we need check_in - check_out function 5 times
				//

				bool ok = query.exec(QString("SELECT * FROM checkin_signals(%1, '{%2}', 'TEST');").arg(m_userIdSignalCreator).arg(signalId));

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

				ok = query.exec(QString("SELECT * FROM checkout_signals(%1, '{%2}');").arg(m_userIdSignalCreator).arg(signalId));

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

				if (error)
				{
					qDebug() << "get_latest_signal " << signalNumber << ": ERROR";
					numberOfErrors++;
				}
				error = false;
			}
			signalNumber++;
		}
		if (numberOfErrors == 0)
		{
			qDebug() << "PASS   : MultiSignalStressTest :: checkIn - checkOut() ";
		}
		else
		{
			qDebug() << "FAIL   : MultiSignalStressTest :: checkIn - checkOut() ";
		}
	}
	else	// Start get_latest_signal Test
	{
		// Create random user, to start get_latest_file() funtion on signal owned by another user
		//

		bool ok = query.exec("SELECT * FROM create_user(1, 'StressTestRandomUser', 'StressTestRandomUser', 'StressTestRandomUser', 'StressTestRandomUser', false, false, false);");

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

		int randomUserId = query.value(0).toInt();

		for (int signalId : m_signalIds)
		{
			for (int numberOfCycleToMakeStressTest = 0; numberOfCycleToMakeStressTest < 5; numberOfCycleToMakeStressTest++)
			{
				ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2)").arg(randomUserId).arg(signalId));

				if (ok == false)
				{
					qDebug() << query.lastError().databaseText();
					error = true;
				}

				/*if (query.next() == false)
				{
					qDebug() << query.lastError().databaseText();
					error = true;
				}*/

				if (error)
				{
					qDebug() << "get_latest_signal " << signalNumber << ": ERROR";
					numberOfErrors++;
				}
				error = false;
			}
			signalNumber++;
		}
		if (numberOfErrors == 0)
		{
			qDebug() << "PASS   : MultiSignalStressTest :: get_latest_signal() ";
		}
		else
		{
			qDebug() << "FAIL   : MultiSignalStressTest :: get_latest_signal() ";
		}
	}
	db.close();
}

MultiThreadSignalStressTest::~MultiThreadSignalStressTest()
{
	//qDebug() << "MultiThreadTest::~MultiThreadTest() " << m_threadNumber;
}
