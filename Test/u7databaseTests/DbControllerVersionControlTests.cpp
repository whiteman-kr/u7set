#include "DbControllerVersionControlTests.h"
#include <QSql>
#include <QSqlError>
#include <QDebug>

DbControllerVersionControlTests::DbControllerVersionControlTests() :
	m_db(new DbController())
{
}

void DbControllerVersionControlTests::initTestCase()
{
	m_db->setServerUsername(m_databaseUser);
	m_db->setServerPassword(m_adminPassword);

	QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("postgres");

	QVERIFY2 (db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

	QSqlQuery query, tempQuery;
	bool ok = query.exec("SELECT datname FROM pg_database WHERE datname LIKE 'u7_%' AND NOT datname LIKE 'u7u%'");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	while (query.next() == true)
	{
		if (query.value(0).toString() == "u7_" + m_databaseName)
		{
			ok = tempQuery.exec(QString("DROP DATABASE %1").arg(query.value(0).toString()));
			QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
			qDebug() << "Project " << query.value(0).toString() << "dropped!";
		}
	}

	db.close();

	ok = m_db->createProject(m_databaseName, m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not create project: " + m_db->lastError()));

	ok = m_db->upgradeProject(m_databaseName, m_adminPassword, true, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not upgrade project: " + m_db->lastError()));

	ok = m_db->openProject(m_databaseName, "Administrator", m_adminPassword, 0);
	QVERIFY2 (ok == true, qPrintable ("Error: can not open project: " + m_db->lastError()));
}

void DbControllerVersionControlTests::isAnyCheckedOutTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2(db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

	QSqlQuery query;

	bool ok = query.exec("SELECT * FROM add_file(1, 'isAnycheckedOutTest', 1, 'testtesttest', '{}')");
	QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));

	int result = 0;

	ok = m_db->isAnyCheckedOut(&result);
	QVERIFY2(ok == true, qPrintable(m_db->lastError()));
	QVERIFY2(result == 1, qPrintable("Error: there are some checked out files, but function isAnyCheckedOut ignored it"));

	db.close();
}

void DbControllerVersionControlTests::lastChangesetIdTest()
{
	QSqlDatabase db = QSqlDatabase::database();

	db.setHostName(m_databaseHost);
	db.setUserName(m_databaseUser);
	db.setPassword(m_adminPassword);
	db.setDatabaseName("u7_" + m_databaseName);

	QVERIFY2 (db.open() == true, qPrintable("Error: Can not connect to postgres database! " + db.lastError().databaseText()));

	QSqlQuery query;

	bool ok = query.exec("SELECT * FROM add_file(1, 'lstChangesetIdTest', 1, 'testtesttest', '{}')");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec(QString("SELECT * FROM check_in(1, '{%1}', 'TEST');").arg(query.value("id").toInt()));
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));

	ok = query.exec("SELECT MAX(changesetId) FROM changeset");
	QVERIFY2 (ok == true, qPrintable(query.lastError().databaseText()));
	QVERIFY2 (query.next() == true, qPrintable(query.lastError().databaseText()));

	int result = 0;

	ok = m_db->lastChangesetId(&result);
	QVERIFY2 (ok == true, qPrintable(m_db->lastError()));

	assert(result != 0);

	QVERIFY2 (query.value(0).toInt() == result, qPrintable("Error: wrong changesetId returned by function"));

	db.close();
}

void DbControllerVersionControlTests::cleanupTestCase()
{
	for (QString connection : QSqlDatabase::connectionNames())
	{
		QSqlDatabase::removeDatabase(connection);
	}

	bool ok = m_db->deleteProject(m_databaseName, m_adminPassword, true, nullptr);
	QVERIFY2(ok == true, qPrintable(m_db->lastError()));

	return;
}
