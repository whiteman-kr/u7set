#include <QString>
#include <QtTest>
#include <QtSql>
#include "UserTests.h"
#include "FileTests.h"
#include "OtherTests.h"
#include "SignalTests.h"

const int DatabaseProjectVersion = 40;

int main(int argc, char *argv[])
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
	db.setHostName("localhost");
	db.setUserName("fabler");
	db.setPassword("qwerty15");
	db.setDatabaseName("u7_test");

	bool ok = db.open();
	if (ok == false)
	{
		qDebug() << "Cannot connect to database";
		return 1;
	}

	QSqlQuery query;
	bool result = query.exec("SELECT MAX(VersionNo) FROM Version");
	if (result == false)
	{
		qDebug() << "Error executting query";
		return 1;
	}

	result = query.first();
	if (result == false)
	{
		qDebug() << "Cannot get first record";
		return 1;
	}

	int version = query.value(0).toInt();
	if (version != DatabaseProjectVersion)
	{
		qDebug() << "Invalid database version, " << DatabaseProjectVersion << " required, current: " << version;
		return 1;
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
		return testResult;
	}

	testResult = QTest::qExec(&otherTests, argc, argv);
	if (testResult != 0)
	{
		qDebug() << testResult << " other test(s) has been interrupted by error(s)";
		return testResult;
	}

	testResult = QTest::qExec(&signalTests, argc, argv);
	if (testResult != 0)
	{
		qDebug() << testResult << " file test(s) has been interrupted by error(s)";
		return testResult;
	}

	testResult = QTest::qExec(&fileTests, argc, argv);
	if (testResult != 0)
	{
		qDebug() << testResult << " file test(s) has been interrupted by error(s)";
		return testResult;
	}

	return 0;
}
