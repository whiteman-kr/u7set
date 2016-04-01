#include "ProjectPropertyTests.h"
#include <QtSql>
#include <QString>
#include <QDebug>

ProjectPropertyTests::ProjectPropertyTests()
{

}

void ProjectPropertyTests::set_project_property()
{
	QSqlQuery query;

	bool ok = query.exec("SELECT set_project_property('Name', 'TEST');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error creating property with right parameters. True (sucsessful result) expected"));

	ok = query.exec("SELECT * FROM projectproperties WHERE name = 'Name' AND value = 'TEST';");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec("SELECT set_project_property('Name', 'TEST2');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error creating property with right parameters. True (sucsessful result) expected"));

	ok = query.exec("SELECT COUNT(*) FROM projectproperties WHERE name = 'Name' AND value = 'TEST';");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toInt() == 0, qPrintable("Error: Function set_project_property() do not updated (delete old) the record!"));

	ok = query.exec("SELECT COUNT(*) FROM projectproperties WHERE name = 'Name' AND value = 'TEST2';");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toInt() == 1, qPrintable("Error: Function set_project_property() do not updated (create new) the record!"));

	ok = query.exec("SELECT set_project_property('', '');");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: false expected. Can not create property with NULL name"));
}

void ProjectPropertyTests::get_project_property()
{
	QSqlQuery query;

	bool ok = query.exec("SELECT set_project_property('getProjectPropertyTest', 'justValue');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec("SELECT get_project_property('getProjectPropertyTest');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toString() == "justValue", qPrintable("Error: get_project_property() returned wrong value!"));

	ok = query.exec("SELECT get_project_property('testErrName');");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toString() == "", qPrintable("Error: no answer expected in case of wrong name"));
}
