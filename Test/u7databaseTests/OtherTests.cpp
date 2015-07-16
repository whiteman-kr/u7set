#include <QtSql>
#include <QTest>
#include "OtherTests.h"


OtherTests::OtherTests()
{
}

void OtherTests::get_project_versionTest()
{
	QCOMPARE(OtherTests::get_project_version(), true);
}

bool OtherTests::get_project_version()
{
	QSqlQuery query;
	bool ok = query.exec("SELECT get_project_version();");
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	ok = query.first();
	if (ok == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	int result = query.value(0).toInt();

	if (query.exec("SELECT MAX(\"VersionNo\") FROM \"Version\"") == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (query.first() == false)
	{
		qDebug() << query.lastError().databaseText();
		return false;
	}

	if (result != query.value(0))
	{
		return false;
	}

	return true;
}
