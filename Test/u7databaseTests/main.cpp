#include <QString>
#include <QtTest>
#include <QtSql>
#include "UserTests.h"
#include "FileTests.h"
#include "OtherTests.h"
#include "SignalTests.h"
#include "../../include/DbController.h"

const int DatabaseProjectVersion = 40;

const char* DatabaseHost = "127.0.0.1";
const char* DatabaseUser = "u7";
const char* DatabaseUserPassword = "P2ssw0rd";

const char* ProjectName = "testproject";
const char* ProjectAdministratorName = "Administrator";
const char* ProjectAdministratorPassword = "P2ssw0rd";

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	{
		DbController dbc;

		dbc.disableProgress();
		dbc.setHost(DatabaseHost);
		dbc.setServerPassword(DatabaseUserPassword);
		dbc.setServerUsername(DatabaseUser);

		// Create Project
		//
		bool ok = dbc.createProject(ProjectName, ProjectAdministratorPassword, nullptr);

		if (ok == false)
		{
			qDebug() << "Cannot connect to database or create project. Error: " << dbc.lastError();
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
			throw 1;
		}

		// TODO: Get project version from DbController
		//
		QSqlQuery query;
		bool result = query.exec("SELECT MAX(VersionNo) FROM Version");
		if (result == false)
		{
			qDebug() << "Error executting query";
			throw 1;
		}

		result = query.first();
		if (result == false)
		{
			qDebug() << "Cannot get first record";
			throw 1;
		}

		int version = query.value(0).toInt();
		if (version != DatabaseProjectVersion)
		{
			qDebug() << "Invalid database version, " << DatabaseProjectVersion << " required, current: " << version;
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
			throw testResult;
		}

		testResult = QTest::qExec(&otherTests, argc, argv);
		if (testResult != 0)
		{
			qDebug() << testResult << " other test(s) has been interrupted by error(s)";
			throw testResult;
		}

		testResult = QTest::qExec(&signalTests, argc, argv);
		if (testResult != 0)
		{
			qDebug() << testResult << " file test(s) has been interrupted by error(s)";
			throw testResult;
		}

		testResult = QTest::qExec(&fileTests, argc, argv);
		if (testResult != 0)
		{
			qDebug() << testResult << " file test(s) has been interrupted by error(s)";
			throw testResult;
		}

		db.close();
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

	return returnCode;
}
