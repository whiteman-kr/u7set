#include <QString>
#include <QtTest>
#include <QtSql>
#include "UserTests.h"
#include "FileTests.h"
#include "OtherTests.h"


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
	bool result;
	result = query.exec("SELECT \"VersionNo\" FROM \"Version\" ORDER BY \"VersionNo\" desc limit 1");
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
	QString version = query.value("VersionNo").toString();
	if (version != "39")
	{
		qDebug() << "Invalid database version, 39 required, current: " << version;
		return 1;
	}


	UserTests userTests;
	FileTests fileTests;
	OtherTests otherTests;

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
		qDebug() << testResult << " file test(s) has been interrupted by error(s)";
		return testResult;
	}

	testResult = QTest::qExec(&fileTests, argc, argv);
	if (testResult != 0)
	{
		qDebug() << testResult << " file test(s) has been interrupted by error(s)";
		return testResult;
	}

	//FileTests::file_add(1, "test", 1, "test");

	return 0;
}
