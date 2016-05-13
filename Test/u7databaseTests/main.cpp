#include <QString>
#include <QtTest>
#include <QtSql>
#include <QObject>
#include "UserTests.h"
#include "FileTests.h"
#include "OtherTests.h"
#include "SignalTests.h"
#include "PropertyObjectTests.h"
#include "MultiThreadFileTest.h"
#include "MultiThreadSignalTests.h"
#include "ProjectPropertyTests.h"
#include "DbControllerProjectManagementTests.h"
#include "DbControllerUserManagementTests.h"
#include "DbControllerFileManagementTests.h"
#include "../../include/DbController.h"

const int DatabaseProjectVersion = 61;

const char* DatabaseHost = "127.0.0.1";
const char* DatabaseUser = "u7";
const char* DatabaseUserPassword = "P2ssw0rd";

const char* ProjectName = "testproject";
const char* ProjectAdministratorName = "Administrator";
const char* ProjectAdministratorPassword = "P2ssw0rd";

const int AmountOfThreadsInMultiThreadTest = 10;
const int AmountOfItemsInMultiThreadTest = 500;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	Hardware::Init();

	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
	bool matchedForDeletion = false;

	db.setHostName(DatabaseHost);
	db.setUserName(DatabaseUser);
	db.setPassword(DatabaseUserPassword);
	db.setDatabaseName(QString("u7_") + ProjectName);

	if (db.open() == true)
	{
		matchedForDeletion = true;
	}

	db.close();

	{
		DbController dbc;

		dbc.disableProgress();
		dbc.setHost(DatabaseHost);
		dbc.setServerPassword(DatabaseUserPassword);
		dbc.setServerUsername(DatabaseUser);

		// Check project need to be deleted
		//

		if (matchedForDeletion)
		{
			bool ok = dbc.deleteProject(ProjectName, ProjectAdministratorPassword, true, nullptr);

			if (ok == false)
			{
				qDebug() << "Cannot delete database project. Error: " << dbc.lastError();
				return 1;
			}
		}

		// Create Project
		//

		bool ok = dbc.createProject(ProjectName, ProjectAdministratorPassword, nullptr);

		if (ok == false)
		{
			qDebug() << "Cannot connect to database or create project. Error: " << dbc.lastError();

			Hardware::Shutdwon();
			return 1;
		}

		// Upgrade project database to actual version
		//
		ok = dbc.upgradeProject(ProjectName, ProjectAdministratorPassword, true, nullptr);

		if (ok == false)
		{
			qDebug() << "Cannot upgrade project database. Error: " << dbc.lastError();

			// TODO: Drop database project
			//

			Hardware::Shutdwon();

			return 1;
		}

		// Open project
		//
		//	ok = db.openProject(ProjectName, ProjectAdministratorName, ProjectAdministratorPassword, nullptr);
		//	if (ok == false)
		//	{
		//		qDebug() << "Cannot open project database. Error: " << db.lastError();

		//		// TODO: Drop database project
		//		//
		//		return 1;
		//	}
	}

	int returnCode = 0;

	try
	{
		// Block is to release QSqlDatabase
		//
		QSqlDatabase db = QSqlDatabase::database();

		db.setHostName(DatabaseHost);
		db.setUserName(DatabaseUser);
		db.setPassword(DatabaseUserPassword);
		db.setDatabaseName(QString("u7_") + ProjectName);

		bool ok = db.open();
		if (ok == false)
		{
			qDebug() << "Cannot connect to database. Error: " << db.lastError();
			db.close();
			throw 1;
		}

		// TODO: Get project version from DbController
		//
		QSqlQuery query;
		bool result = query.exec("SELECT MAX(VersionNo) FROM Version");
		if (result == false)
		{
			qDebug() << "Error executting query";
			db.close();
			throw 1;
		}

		result = query.first();
		if (result == false)
		{
			qDebug() << "Cannot get first record";
			db.close();
			throw 1;
		}

		int version = query.value(0).toInt();
		if (version != DatabaseProjectVersion)
		{
			qDebug() << "Invalid database version, " << DatabaseProjectVersion << " required, current: " << version;
			db.close();
			throw 1;
		}


		UserTests userTests;
		FileTests fileTests;
		OtherTests otherTests;
		SignalTests signalTests;
		ProjectPropertyTests projectPropertyTests;
		PropertyObjectTests propertyObjectTests;
		DbControllerProjectTests dbControllerProjectTests;
		DbControllerUserTests dbControllerUserTests;
		DbControllerFileTests dbControllerFileTests;

		int testResult;
		testResult = QTest::qExec(&userTests, argc, argv);
		if (testResult != 0)
		{
			qDebug() << testResult << " user test(s) has been interrupted by error(s)";
			db.close();
			throw testResult;
		}

		testResult = QTest::qExec(&fileTests, argc, argv);
		if (testResult != 0)
		{
			qDebug() << testResult << " file test(s) has been interrupted by error(s)";
			db.close();
			throw testResult;
		}

		testResult = QTest::qExec(&signalTests, argc, argv);
		if (testResult != 0)
		{
			qDebug() << testResult << " signal test(s) has been interrupted by error(s)";
			db.close();
			throw testResult;
		}

		testResult = QTest::qExec(&otherTests, argc, argv);
		if (testResult != 0)
		{
			qDebug() << testResult << " other test(s) has been interrupted by error(s)";
			db.close();
			throw testResult;
		}

		testResult = QTest::qExec(&projectPropertyTests, argc, argv);
		if (testResult != 0)
		{
			qDebug() << testResult << " project property test(s) has been interrupted by error(s)";
			db.close();
			throw testResult;
		}

		testResult = QTest::qExec(&propertyObjectTests, argc, argv);
		if (testResult != 0)
		{
			qDebug() << testResult << " propertyObject test(s) has been interrupted by error(s)";
			db.close();
			throw testResult;
		}

		db.close();

		testResult = QTest::qExec(&dbControllerProjectTests, argc, argv);
		if (testResult != 0)
		{
			qDebug() << testResult << " dbControllerProject test(s) has been interrupted by error(s)";
			db.close();
			throw testResult;
		}

		testResult = QTest::qExec(&dbControllerUserTests, argc, argv);
		if (testResult != 0)
		{
			qDebug() << testResult << " dbControllerUser test(s) has been interrupted by error(s)";
			db.close();
			throw testResult;
		}

		testResult = QTest::qExec(&dbControllerFileTests, argc, argv);
		if (testResult != 0)
		{
			qDebug() << testResult << " dbControllerFile test(s) has been interrupted by error(s)";
			db.close();
			throw testResult;
		}

		// Multi-thread testing
		//

		qDebug() << "********* Start testing of MultiThreadFile tests *********";

		std::vector<MultiThreadFileTest*> multiThreadFileTest;

		for (int numberOfThread = 0; numberOfThread < AmountOfThreadsInMultiThreadTest; numberOfThread++)
		{
			MultiThreadFileTest* thread = new MultiThreadFileTest(numberOfThread, DatabaseHost, DatabaseUser, DatabaseUserPassword, ProjectName, AmountOfItemsInMultiThreadTest);

			thread->start();

			multiThreadFileTest.push_back(thread);
		}

		for (MultiThreadFileTest* thread : multiThreadFileTest)
		{
			while (thread->isFinished() == false)
			{
			}

			delete thread;
		}

		qDebug() << "********* Finished testing of MultiThreadFile tests *********";
		qDebug() << "********* Started testing of MultiThreadSignal tests *********";

		std::vector<MultiThreadSignalTest*> multiThreadSignalTest;

		for (int numberOfThread = 0; numberOfThread < AmountOfThreadsInMultiThreadTest; numberOfThread++)
		{
			MultiThreadSignalTest* thread = new MultiThreadSignalTest(numberOfThread, DatabaseHost, DatabaseUser, DatabaseUserPassword, ProjectName, AmountOfItemsInMultiThreadTest);

			thread->start();

			multiThreadSignalTest.push_back(thread);
		}

		for (MultiThreadSignalTest* thread : multiThreadSignalTest)
		{
			while (thread->isFinished() == false)
			{
			}

			delete thread;
		}

		qDebug() << "********* Finished testing of MultiThreadSignal tests *********";
		qDebug() << "********* Started testing of MultiThreadStressSignal tests *********";

		std::vector<int> signalIds;

		int userIdForSignalStressTest = MultiThreadSignalTest::create_user(DatabaseHost,
																		   DatabaseUser,
																		   DatabaseUserPassword,
																		   ProjectName);

		int errCode = MultiThreadSignalTest::fillSignalIdsVector(signalIds,
																 userIdForSignalStressTest,
																 AmountOfItemsInMultiThreadTest,
																 DatabaseHost,
																 DatabaseUser,
																 DatabaseUserPassword,
																 ProjectName);

		if (errCode == 0)
		{
			MultiThreadGetSignalTest* threadGetLatestSignal = new MultiThreadGetSignalTest(DatabaseHost,
																						   DatabaseUser,
																						   DatabaseUserPassword,
																						   ProjectName,
																						   signalIds);

			MultiThreadSignalCheckInTest* threadCheckInCheckOut = new MultiThreadSignalCheckInTest(DatabaseHost,
																								   DatabaseUser,
																								   DatabaseUserPassword,
																								   ProjectName,
																								   userIdForSignalStressTest,
																								   signalIds,
																								   threadGetLatestSignal);



			threadGetLatestSignal->start();
			threadCheckInCheckOut->start();

			while (threadCheckInCheckOut->isFinished() == false)
			{
			}

			while (threadGetLatestSignal->isFinished() == false)
			{
			}

			delete threadCheckInCheckOut;
			delete threadGetLatestSignal;
		}
		else
			qDebug() << "FAIL: errCode is " << errCode << ": can not fill vector with fileIds or create user";

		qDebug() << "********* Finished testing of MultiThreadStressSignal tests *********";
	}
	catch (int retval)
	{
		returnCode = retval;
	}

	// Drop database project
	//
	{
		DbController dbc;

		dbc.disableProgress();
		dbc.setHost(DatabaseHost);
		dbc.setServerPassword(DatabaseUserPassword);
		dbc.setServerUsername(DatabaseUser);

		bool ok = dbc.deleteProject(ProjectName, ProjectAdministratorPassword, true, nullptr);

		if (ok == false)
		{
			qDebug() << "Cannot delete database project. Error: " << dbc.lastError();
			returnCode = 1;
		}
	}

	Hardware::Shutdwon();

	return returnCode;
}
