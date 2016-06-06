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

	QVERIFY2(rowNumber == rowCount, qPrintable("Error: different nomber of rows"));
}

void OtherTests::build_startTest()
{
	QSqlQuery query;

	QString buildWorkstation = "testWorkstation";

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == ok, qPrintable (query.lastError().databaseText()));

	int signalId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST');").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == ok, qPrintable (query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE signalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == ok, qPrintable (query.lastError().databaseText()));

	int buildChangesetId = query.value("changeSetId").toInt();

	ok = query.exec(QString("SELECT * FROM build_start(1, '%2', false, %3)").arg(buildWorkstation).arg(buildChangesetId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT COUNT(*) FROM build WHERE userId = 1 AND workStation = '%1' AND release = false AND changeSetId = %2").arg(buildWorkstation).arg(buildChangesetId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toInt() == 1, qPrintable("Table not changed!"));

	buildChangesetId = 999999;

	ok = query.exec(QString("SELECT * FROM build_start(1, '%2', true, %3)").arg(buildWorkstation).arg(buildChangesetId));
	QVERIFY2(ok == false, qPrintable("Wrong changeSetId error expected"));

	buildChangesetId = -1;

	ok = query.exec(QString("SELECT * FROM build_start(1, '%2', true, %3)").arg(buildWorkstation).arg(buildChangesetId));
	QVERIFY2(ok == false, qPrintable("Wrong changeSetId error expected"));
}

void OtherTests::build_finishTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	const int errorsAmount = 1;
	const int warningsAmount = 2;
	const QString buildLog = "Test";
	const QString buildWorkstation = "testWorkstation";

	bool ok = query.exec("SELECT * FROM add_signal(1, 0, 0)");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == ok, qPrintable (query.lastError().databaseText()));

	int signalId = query.value("id").toInt();

	ok = query.exec(QString("SELECT * FROM checkin_signals(1, '{%1}', 'TEST');").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == ok, qPrintable (query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM signalInstance WHERE signalId = %1").arg(signalId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == ok, qPrintable (query.lastError().databaseText()));

	int buildChangesetId = query.value("changeSetId").toInt();

	ok = query.exec(QString("SELECT * FROM build_start(1, '%2', false, %3)").arg(buildWorkstation).arg(buildChangesetId));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	int buildId = query.value("build_start").toInt();

	ok = query.exec(QString("SElECT * FROM build_finish(%1, %2, %3, '%4')").arg(buildId).arg(errorsAmount).arg(warningsAmount).arg(buildLog));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT COUNT(*) FROM build WHERE buildId = %1 AND errors = %2 AND warnings = %3").arg(buildId).arg(errorsAmount).arg(warningsAmount));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	QVERIFY2(query.value(0).toInt() == 1, qPrintable("Error: table build not changed"));
}
