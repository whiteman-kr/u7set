#include <QString>
#include <QtTest>
#include <QtSql>
#include "UserTests.h"
#include "FileTests.h"
#include "OtherTests.h"
#include "SignalTests.h"
#include "MultiThreadTest.h"
#include "../../include/DbController.h"

const int DatabaseProjectVersion = 40;

const char* DatabaseHost = "127.0.0.1";
const char* DatabaseUser = "u7";
const char* DatabaseUserPassword = "P2ssw0rd";

const char* ProjectName = "testproject";
const char* ProjectAdministratorName = "Administrator";
const char* ProjectAdministratorPassword = "P2ssw0rd";

const int AmountOfThreadsInMultiThreadTest = 10;
const int AmountOfFilesInMultiThreadTest = 500;

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
		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

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

		db.close();
	}
	catch (int retval)
	{
		returnCode = retval;
	}

	// Multi-thread testing
	//

	qDebug() << "********* Start testing of Multi-thread tests *********";

	std::vector<MultiThreadTest*> multiThreadTest;

	for (int numberOfThread = 0; numberOfThread < AmountOfThreadsInMultiThreadTest; numberOfThread++)
	{
		MultiThreadTest* thread = new MultiThreadTest(numberOfThread, DatabaseHost, DatabaseUser, DatabaseUserPassword, ProjectName, AmountOfFilesInMultiThreadTest);

		thread->start();

		multiThreadTest.push_back(thread);
	}

	for (MultiThreadTest* thread : multiThreadTest)
	{
		while (thread->isFinished() == false)
		{
		}

		delete thread;
	}

	qDebug() << "********* Finished testing of Multi-thread tests *********";

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
