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
