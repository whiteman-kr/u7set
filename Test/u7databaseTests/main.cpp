#include <QString>
#include <QtTest>
#include <QtSql>
#include "UserTests.h"


int main(int argc, char *argv[])
{
	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
	db.setHostName("localhost");
	db.setUserName("fabler");
	db.setPassword("qwerty15");
	db.setDatabaseName("u7_test1");

	bool ok = db.open();
	if (ok == false)
	{
		qDebug() << "Cannot connect to database";
		return 1;
	}

	UserTests userTests;
	if (QTest::qExec(&userTests, argc, argv) != 0)
	{
		qDebug() << "User Tests has been interrupted by error(s)";
		return 1;
	}

	return 0;
}

//#include "main.moc"
