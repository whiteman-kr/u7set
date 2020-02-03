#include "DbControllerVersionControlTests.h"
#include <QSql>
#include <QSqlError>
#include <QDebug>

DbControllerVersionControlTests::DbControllerVersionControlTests() :
	m_db(new DbController())
{
}

QString DbControllerVersionControlTests::logIn(QString username, QString password)
{
	QSqlQuery query;
	bool ok = query.exec(QString("SELECT * FROM user_api.log_in('%1', '%2')")
							.arg(username)
							.arg(password));

	if (ok == false)
	{
		return QString("");
	}

	ok = query.next();
	if (ok == false)
	{
		return QString("");
	}

	QString session_key = query.value(0).toString();
	return session_key;
}

bool DbControllerVersionControlTests::logOut()
{
	QSqlQuery query;
	bool ok = query.exec("SELECT * FROM user_api.log_out()");
	return ok;
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

void DbControllerVersionControlTests::get_changeset_details()
{
	int changesetId = -1;
	int fileId = -1;

	{
		QSqlDatabase db = QSqlDatabase::database();

		db.setHostName(m_databaseHost);
		db.setUserName(m_databaseUser);
		db.setPassword(m_adminPassword);
		db.setDatabaseName("u7_" + m_databaseName);

		QVERIFY2 (db.open() == true, qPrintable(QString("Error: Can not connect to %1 database! ").arg("u7_" + m_databaseName) + db.lastError().databaseText()));

		QSqlQuery query(db);

		// 1. LogIn as User1
		//
		QString session_key = logIn("Administrator", m_adminPassword);
		QVERIFY2(session_key.isEmpty() == false, "Log in error");

		bool ok = true;

		// 2. Create and CheckInt file
		//
		QString request = QString("SELECT * FROM api.add_or_update_file('%1', '$root$/HP', 'TestFile.tmp', 'Test function get_changeset_details', '', '{}');")
						  .arg(session_key);

		ok = query.exec(request);
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.size() == 1, "api.add_or_update_file, expected 1 record result");

		query.next();
		fileId = query.value(0).toInt();

		// 3. Get file history, shoud be 1 records
		//
		request = QString("SELECT * FROM api.get_file_history('%1', %2);")
							.arg(session_key)
							.arg(fileId);

		ok = query.exec(request);
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.size() == 1, "api.get_file_history, expected 1 record result");

		query.next();

		DbChangeset cs;
		DbWorker::db_dbChangeset(query, &cs);

		QVERIFY(cs.userId() == 1);
		QVERIFY(cs.username() == "Administrator");
		QVERIFY(cs.comment() == "Test function get_changeset_details");
		QVERIFY(cs.action() == VcsItemAction::Added);

		changesetId = cs.changeset();

		// By chagesetid get it's info
		//
		request = QString("SELECT * FROM api.get_changeset_details('%1', %2);")
							.arg(session_key)
							.arg(changesetId);

		ok = query.exec(request);
		QVERIFY2(ok == true, qPrintable(query.lastError().databaseText()));
		QVERIFY2(query.size() == 1, "api.get_changeset_details, expected 1 record result");

		query.next();

		DbChangesetDetails csd;
		DbWorker::db_dbChangesetObject(query, &csd);

		const std::vector<DbChangesetObject>& objects = csd.objects();

		QVERIFY(csd.changeset() == changesetId);
		QVERIFY(csd.username() == "Administrator");
		QVERIFY(csd.comment() == "Test function get_changeset_details");
		QVERIFY(objects.size() == 1);

		if (objects.size() == 1)
		{
			const DbChangesetObject& cso = objects[0];

			QVERIFY(cso.type() == DbChangesetObject::Type::File);
			QVERIFY(cso.isFile() == true);
			QVERIFY(cso.isSignal() == false);

			QVERIFY(cso.id() == fileId);
			QVERIFY(cso.name() == "TestFile.tmp");
			QVERIFY(cso.action() == VcsItemAction::Added);
		}

		// 7. LogOut
		//
		ok = logOut();
		QVERIFY2(ok == true, "Log out error");
	}

	{
		// changesetId
		//
		DbChangesetDetails csd;

		bool ok = m_db->getChangesetDetails(changesetId, &csd, nullptr);
		QVERIFY2(ok == true, qPrintable(m_db->lastError()));

		QVERIFY(csd.changeset() == changesetId);
		QVERIFY(csd.userId() == 1);
		QVERIFY(csd.username() == "Administrator");
		QVERIFY(csd.comment() == "Test function get_changeset_details");


		const std::vector<DbChangesetObject>& objects = csd.objects();
		QVERIFY(objects.size() == 1);

		if (objects.size() == 1)
		{
			const DbChangesetObject& cso = objects[0];

			QVERIFY(cso.type() == DbChangesetObject::Type::File);
			QVERIFY(cso.isFile() == true);
			QVERIFY(cso.isSignal() == false);

			QVERIFY(cso.id() == fileId);
			QVERIFY(cso.name() == "TestFile.tmp");
			QVERIFY(cso.action() == VcsItemAction::Added);
		}
	}

	return;
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
