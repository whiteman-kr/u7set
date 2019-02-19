#include "ProjectPropertyTests.h"
#include <QtSql>
#include <QString>
#include <QDebug>

ProjectPropertyTests::ProjectPropertyTests()
{

}

void ProjectPropertyTests::initTestCase()
{
	bool ok = createProjectDb();
	QVERIFY2(ok == true, "Cannot create projectdatabase");
}

void ProjectPropertyTests::cleanupTestCase()
{
	dropProjectDb();
}

void ProjectPropertyTests::set_project_property()
{
	// LogIn as Admin
	//
	QString session_key = logIn(m_projectAdministratorName, m_projectAdministratorPassword);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT api.set_project_property('%1', 'Name', 'TEST');").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toBool() == true, qPrintable("Error creating property with right parameters. True (sucsessful result) expected"));

	ok = query.exec("SELECT * FROM projectproperties WHERE name = 'Name' AND value = 'TEST';");

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT api.set_project_property('%1', 'Name', 'TEST2');").arg(session_key));

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

	ok = query.exec(QString("SELECT api.set_project_property('%1', '', '');").arg(session_key));
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toBool() == false, qPrintable("Error: false expected. Can not create property with NULL name"));

	// LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
}

void ProjectPropertyTests::get_project_property()
{
	// LogIn as Admin
	//
	QString session_key = logIn(m_projectAdministratorName, m_projectAdministratorPassword);
	QVERIFY2(session_key.isEmpty() == false, "Log in error");

	// --
	//
	QSqlQuery query;

	bool ok = query.exec(QString("SELECT api.set_project_property('%1', 'getProjectPropertyTest', 'justValue');").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM api.get_project_property('%1', 'getProjectPropertyTest');").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toString() == "justValue", qPrintable("Error: get_project_property() returned wrong value!"));

	ok = query.exec(QString("SELECT * FROM api.get_project_property('%1', 'testErrName');").arg(session_key));

	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.first() == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2(query.value(0).toString() == "", qPrintable("Error: no answer expected in case of wrong name"));

	// LogOut
	//
	ok = logOut();
	QVERIFY2(ok == true, "Log out error");

	return;
}
