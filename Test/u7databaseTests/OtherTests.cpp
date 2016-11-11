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

void OtherTests::add_unitTest()
{
	QSqlQuery query;
	QSqlQuery tempQuery;

	QString unit_en = "test_unit_en";
	QString unit_ru = "тест_юнит_ру";

	int unit_id = -1;

	bool ok = query.exec(QString("SELECT * FROM add_unit('%1','%2')").arg(unit_en).arg(unit_ru));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = tempQuery.exec(QString("SELECT * FROM unit WHERE unit_en = '%1' AND unit_ru = '%2'").arg(unit_en).arg(unit_ru));
	QVERIFY2(ok == true, qPrintable(tempQuery.lastError().databaseText()));
	QVERIFY2(tempQuery.next() == true, qPrintable(tempQuery.lastError().databaseText()));

	unit_id = tempQuery.value("unitid").toInt();

	QVERIFY2(query.value("add_unit").toInt() == unit_id, qPrintable("Error: wrong unit_id returned!"));

	ok = query.exec(QString("SELECT * FROM add_unit('%1','test')").arg(unit_en));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("add_unit").toInt() == -1, qPrintable("Error: expecting existing unit_en error!"));

	ok = query.exec(QString("SELECT * FROM add_unit('test','%1')").arg(unit_ru));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("add_unit").toInt() == -2, qPrintable("Error: expecting existing unit_ru error!"));
}

void OtherTests::update_unitTest()
{
	QSqlQuery query;

	QString unit_en = "test_unit_en_1";
	QString unit_ru = "тест_юнит_ру_1";

	QString new_unit_en = "test1";
	QString new_unit_ru = "test2";

	int unit_id = -1;

	bool ok = query.exec(QString("SELECT * FROM add_unit('%1','%2')").arg(unit_en).arg(unit_ru));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM unit WHERE unit_en = '%1' AND unit_ru = '%2'").arg(unit_en).arg(unit_ru));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	unit_id = query.value("unitid").toInt();

	ok = query.exec(QString("SELECT * FROM update_unit(%1, '%2','test')").arg(unit_id).arg(unit_en));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("update_unit").toInt() == -1, qPrintable("Error: expecting existing unit_en error!"));

	ok = query.exec(QString("SELECT * FROM update_unit(%1, 'test','%2')").arg(unit_id).arg(unit_ru));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value("update_unit").toInt() == -2, qPrintable("Error: expecting existing unit_ru error!"));

	ok = query.exec(QString("SELECT * FROM update_unit(%1, '%2', '%3')").arg(unit_id).arg(new_unit_en).arg(new_unit_ru));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM unit WHERE unit_en = '%1' AND unit_ru = '%2'").arg(new_unit_en).arg(new_unit_ru));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
}

void OtherTests::add_log_recordTest()
{
	QSqlQuery query;

	QString host = "localhost";
	QString message = "Just simple log. Nothing interesting";

	int processId = 667;
	int userId = 1;

	bool ok = query.exec(QString("SELECT * FROM add_log_record(%1, '%2', %3, '%4')").arg(userId).arg(host).arg(processId).arg(message));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM Log WHERE UserID=%1 AND Host='%2' AND ProcessID=%3 AND Text='%4'").arg(userId).arg(host).arg(processId).arg(message));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_log_record(%1, '%2')").arg(userId).arg(message));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM Log WHERE UserID=%1 AND Text='%2'").arg(userId).arg(message));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM add_log_record(%1, '%2', '%3')").arg(userId).arg(host).arg(message));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM Log WHERE UserID=%1 AND Host='%2' AND Text='%3'").arg(userId).arg(host).arg(message));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.next() == true, qPrintable(query.lastError().databaseText()));
}
