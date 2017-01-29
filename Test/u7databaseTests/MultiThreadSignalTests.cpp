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
						 .arg("SgnalThread_" + QString::number(m_threadNumber))
						 .arg("SgnalThread_" + QString::number(m_threadNumber))
						 .arg("SgnalThread_" + QString::number(m_threadNumber))
						 .arg("SgnalThread_" + QString::number(m_threadNumber)));

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
	int signalNumber = 0;						// Value, that will count amount of signals
	int numberOfErrorsInWorkTest = 0;		// Number of errors, which can be occured in one test
	int totalErrors = 0;					// Total count of errors

	bool error = false;						// Value, that store info of error

	ok = query.exec("SELECT * FROM user_api.log_out()");

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		this->terminate();
	}

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
		sd.appSignalID = "thread_" + QString::number(m_threadNumber) + " signalId_" + QString::number(signalId);
		sd.customAppSignalID = "thread_" + QString::number(m_threadNumber) + " signalId_" + QString::number(signalId);
		sd.caption = "thread_" + QString::number(m_threadNumber) + " signalId_" + QString::number(signalId);
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
		sd.equipmentID = "dviceStrId";
		sd.outputRangeMode = 24;
		sd.filteringTime = 25;
		sd.maxDifference = 26;
		sd.byteOrder = 27;
		sd.enableTuning = "true";
		sd.tuningDefaultValue = 28;

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
						 .arg(sd.appSignalID)
						 .arg(sd.customAppSignalID)
						 .arg(sd.caption)
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

		arguments.append(QString("'%1', %2, %3, %4, %5, '%6', %7")
						 .arg(sd.equipmentID)
						 .arg(sd.outputRangeMode)
						 .arg(sd.filteringTime)
						 .arg(sd.maxDifference)
						 .arg(sd.byteOrder)
						 .arg(sd.enableTuning)
						 .arg(sd.tuningDefaultValue));

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

		if (query.value("appSignalID") != QString("thread_" + QString::number(m_threadNumber) + " signalId_" + QString::number(signalId)))
		{
			qDebug() << "Error in data read-write: wrong strId\nThread: " << m_threadNumber << "\nSigal: " << signalId;
			error = true;
		}

		if (query.value("customAppSignalID") != QString("thread_" + QString::number(m_threadNumber) + " signalId_" + QString::number(signalId)))
		{
			qDebug() << "Error in data read-write: wrong extStrId\nThread: " << m_threadNumber << "\nSigal: " << signalId;
			error = true;
		}

		if (query.value("caption") != QString("thread_" + QString::number(m_threadNumber) + " signalId_" + QString::number(signalId)))
		{
			qDebug() << "Error in data read-write: wrong caption\nThread: " << m_threadNumber << "\nSigal: " << signalId;
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

int MultiThreadSignalTest::create_user(const char* dbHost,
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

	bool ok = query.exec("SELECT salt FROM users WHERE username = 'Administrator'");

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return -1;
	}

	if (query.next() == false)
	{
		qDebug() << query.lastError().databaseText();
		return -1;
	}

	QString passwordHashQuery = QString("user_api.password_hash('%1', '%2')").arg(query.value(0).toString()).arg(dbUserPassword);

	ok = query.exec(QString("UPDATE users SET passwordhash = %1 WHERE username = 'Administrator'").arg(passwordHashQuery));

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return -1;
	}

	ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(dbUserPassword));

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return -1;
	}

	if (query.next() == false)
	{
		qDebug() << query.lastError().databaseText();
		return -1;
	}

	QString session_key = query.value(0).toString();

	ok = query.exec(QString("SELECT * FROM user_api.create_user('%1', 'MultiThreadSignalStressTestUser', 'MultiThreadSignalStressTestUser', 'MultiThreadSignalStressTestUser', 'MultiThreadSignalStressTestUser', false, false)").arg(session_key));

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

	int userId = query.value(0).toInt();

	ok = query.exec("SELECT * FROM user_api.log_out()");

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return -1;
	}

	db.close();
	return userId;
}

int MultiThreadSignalTest::fillSignalIdsVector(std::vector<int>& signalIds,
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

MultiThreadSignalTest::~MultiThreadSignalTest()
{
}

MultiThreadGetSignalTest :: MultiThreadGetSignalTest(const char* dbHost,
													 const char* dbUser,
													 const char* dbUserPassword,
													 const char* name,
													 std::vector<int>& signalIds) :
	m_signalIds(signalIds),
	m_databaseHost(dbHost),
	m_databaseUser(dbUser),
	m_databaseUserPassword(dbUserPassword),
	m_projectName(name)
{
	assert(dbHost);
	assert(dbUser);
	assert(dbUserPassword);
	assert(name);
}

void MultiThreadGetSignalTest::run()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "GetSignalTestMode");

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

	// Create random user, to start get_latest_file() funtion on signal owned by another user
	//

	bool ok = query.exec("SELECT salt FROM users WHERE username = 'Administrator'");

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		qDebug() << "FAIL   : MultiSignalStressTest :: get_latest_signal() ";
		this->terminate();
	}

	if (query.next() == false)
	{
		qDebug() << query.lastError().databaseText();
		qDebug() << "FAIL   : MultiSignalStressTest :: get_latest_signal() ";
		this->terminate();
	}

	QString passwordHashQuery = QString("user_api.password_hash('%1', '%2')").arg(query.value(0).toString()).arg(m_databaseUserPassword);

	ok = query.exec(QString("UPDATE users SET passwordhash = %1 WHERE username = 'Administrator'").arg(passwordHashQuery));

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		qDebug() << "FAIL   : MultiSignalStressTest :: get_latest_signal() ";
		this->terminate();
	}

	ok = query.exec(QString("SELECT * FROM user_api.log_in('Administrator', '%1')").arg(m_databaseUserPassword));

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		qDebug() << "FAIL   : MultiSignalStressTest :: get_latest_signal() ";
		this->terminate();
	}

	if (query.next() == false)
	{
		qDebug() << query.lastError().databaseText();
		qDebug() << "FAIL   : MultiSignalStressTest :: get_latest_signal() ";
		this->terminate();
	}

	QString session_key = query.value(0).toString();

	ok = query.exec(QString("SELECT * FROM user_api.create_user('%1', 'StressTestRandomUser', 'StressTestRandomUser', 'StressTestRandomUser', 'StressTestRandomUser', false, false);").arg(session_key));

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		qDebug() << "FAIL   : MultiSignalStressTest :: get_latest_signal() ";
		this->terminate();
	}

	if (query.next() == false)
	{
		qDebug() << query.lastError().databaseText();
		qDebug() << "FAIL   : MultiSignalStressTest :: get_latest_signal() ";
		this->terminate();
	}

	int randomUserId = query.value(0).toInt();

	ok = query.exec("SELECT * FROM user_api.log_out()");

	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		qDebug() << "FAIL   : MultiSignalStressTest :: get_latest_signal() ";
		this->terminate();
	}

	while (m_currentSignalId != m_signalIds.size())
	{

		// Work with signals, that were send by checkInOut thread
		//

		ok = query.exec(QString("SELECT * FROM get_latest_signal(%1, %2)").arg(randomUserId).arg(m_signalIds[m_currentSignalId]));

		if (ok == false)
		{
			qDebug() << query.lastError().databaseText();
			error = true;
		}

		if (error)
		{
			qDebug() << "get_latest_signal " << m_currentSignalId << ": ERROR";
			numberOfErrors++;
		}
		error = false;
	}
	if (numberOfErrors == 0)
	{
		qDebug() << "PASS   : MultiSignalStressTest :: get_latest_signal() ";
	}
	else
	{
		qDebug() << "FAIL   : MultiSignalStressTest :: get_latest_signal() ";
	}
	db.close();
}

MultiThreadGetSignalTest::~MultiThreadGetSignalTest()
{
}

MultiThreadSignalCheckInTest :: MultiThreadSignalCheckInTest(const char* dbHost,
															 const char* dbUser,
															 const char* dbUserPassword,
															 const char* name,
															 const int userIdSignalCreator,
															 std::vector<int>& signalIds,
															 MultiThreadGetSignalTest* getSignalThread) :
	m_signalIds(signalIds),
	m_databaseHost(dbHost),
	m_databaseUser(dbUser),
	m_databaseUserPassword(dbUserPassword),
	m_projectName(name),
	m_userIdSignalCreator(userIdSignalCreator),
	m_getSignalThread(getSignalThread)
{
	assert(dbHost);
	assert(dbUser);
	assert(dbUserPassword);
	assert(name);
}

void MultiThreadSignalCheckInTest::run()
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "SignalCheckInTestMode");

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

	for (int signalId : m_signalIds)
	{
		m_getSignalThread->m_currentSignalId = signalNumber;
		for (int numberOfFunctionStart = 0; numberOfFunctionStart < 5; numberOfFunctionStart++)
		{
			// Check every signal from vector for 5 times
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
				qDebug() << "get_latest_signal " << m_signalIds[signalNumber] << ": ERROR";
				numberOfErrors++;
			}
			error = false;
		}
		signalNumber++;
	}

	// When all signals were checked, send non-existed signal number
	// for get_latest_signal thread to stop

	m_getSignalThread->m_currentSignalId = signalNumber;

	if (numberOfErrors == 0)
	{
		qDebug() << "PASS   : MultiSignalStressTest :: checkIn - checkOut() ";
	}
	else
	{
		qDebug() << "FAIL   : MultiSignalStressTest :: checkIn - checkOut() ";
	}
	db.close();
}

MultiThreadSignalCheckInTest::~MultiThreadSignalCheckInTest()
{
}
