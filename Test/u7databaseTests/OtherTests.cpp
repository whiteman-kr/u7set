#include <QtSql>
#include <QTest>
#include "OtherTests.h"


OtherTests::OtherTests()
{
}

void OtherTests::get_project_versionTest()
{
	QSqlQuery query;

	bool ok = query.exec("SELECT get_project_version();");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	int result = query.value(0).toInt();

	ok = query.exec("SELECT MAX(versionNo) FROM version");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(result == query.value(0).toInt(), qPrintable("Error: wrong project version"));
}

void OtherTests::get_unitsTest()
{
	QSqlQuery functionQuery, tableQuery, query;
	int rowNumber=0;

	bool ok = query.exec("SELECT COUNT(*) FROM unit");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	int rowCount = query.value(0).toInt();

	ok = functionQuery.exec("SELECT * FROM unit ORDER BY unitid");
	QVERIFY2(ok == true, qPrintable(functionQuery.lastError().databaseText()));

	ok = tableQuery.exec("SELECT * FROM get_units()");
	QVERIFY2(ok == true, qPrintable(tableQuery.lastError().databaseText()));

	while (functionQuery.next() && tableQuery.next())
	{
		rowNumber++;
		QVERIFY2(functionQuery.value("unitId").toInt() == tableQuery.value("unitId").toInt(),
				 qPrintable(QString("Error: values \"unitId\" not match at row %1").arg(rowNumber)));
		QVERIFY2(functionQuery.value("unit_en").toString() == tableQuery.value("unit_en").toString(),
				 qPrintable(QString("Error: values \"unit_en\" not match at row %1").arg(rowNumber)));
		QVERIFY2(functionQuery.value("unit_ru").toString() == tableQuery.value("unit_ru").toString(),
				 qPrintable(QString("Error: values \"unit_ru\" not match at row %1").arg(rowNumber)));
	}

	QVERIFY2(rowNumber = rowCount, qPrintable("Error: different nomber of rows"));
}
