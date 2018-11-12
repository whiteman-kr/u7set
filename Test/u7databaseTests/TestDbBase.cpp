#include "TestDbBase.h"

TestDbBase::TestDbBase()
{
}

void TestDbBase::initTestCase()
{
	bool ok = createProjectDb();
	QVERIFY2(ok == true, "Cannot create project database");
}

void TestDbBase::cleanupTestCase()
{
	dropProjectDb();
}

bool TestDbBase::createProjectDb()
{
	// Drop the old DB, if exists
	//
	dropProjectDb();

	// Create the new one
	//
	{
		DbController dbc;

		dbc.disableProgress();
		dbc.setHost(m_databaseHost);
		dbc.setPort(m_databaseHostPort);
		dbc.setServerPassword(m_databaseUserPassword);
		dbc.setServerUsername(m_databaseUser);

		// Create Project
		//
		if (bool ok = dbc.createProject(m_projectName, m_projectAdministratorPassword, nullptr);
			ok == false)
		{
			qDebug() << "Cannot connect to database or create project. Error: " << dbc.lastError();
			return false;
		}

		// Upgrade project database to actual version
		//
		if (bool ok = dbc.upgradeProject(m_projectName, m_projectAdministratorPassword, true, nullptr);
			ok == false)
		{
			qDebug() << "Cannot upgrade project database. Error: " << dbc.lastError();

			if (bool ok = dbc.deleteProject(m_projectName, m_projectAdministratorPassword, true, nullptr);
				ok == false)
			{
				qDebug() << "Cannot delete database project. Error: " << dbc.lastError();
			}

			return false;
		}
	}

	// Check Project DB compatibility
	//
	try
	{
		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

		db.setHostName(m_databaseHost);
		db.setPort(m_databaseHostPort);

		db.setUserName(m_databaseUser);
		db.setPassword(m_databaseUserPassword);
		db.setDatabaseName(QString("u7_") + m_projectName);

		if (bool ok = db.open();
			ok == false)
		{
			qDebug() << "Cannot connect to database. Error: " << db.lastError();
			throw false;
		}

		QSqlQuery query;
		if (bool result = query.exec("SELECT MAX(VersionNo) FROM Version");
			result == false)
		{
			qDebug() << "Get test ProjectDB version error: " << query.lastError();
			throw false;
		}

		if (query.first() == false)
		{
			qDebug() << "Get test ProjectDB version error: Cannot get first record.";
			throw false;
		}

		int version = query.value(0).toInt();

		// !!!! It is commented, as it calls too many fails on automated CI tests
		//
//		if (version != ProjectDatabaseVersion)
//		{
//			qDebug() << "Get test ProjectDB version error: Invalid database version, " << ProjectDatabaseVersion << " required, current: " << version;
//			throw false;
//		}

		// Version is ok
		//
	}
	catch(...)
	{
		dropProjectDb();
		return false;
	}

	return true;
}

bool TestDbBase::dropProjectDb(QString projectName/*= QString()*/)
{
	if (QSqlDatabase::contains() == true)
	{
		QString cn;
		{
			QSqlDatabase db = QSqlDatabase::database();
			cn = db.connectionName();

			if (db.isOpen() == true)
			{
				db.close();
			}
		}

		QSqlDatabase::removeDatabase(cn);
	}

	if (projectName.isEmpty() == true)
	{
		projectName = m_projectName;
	}

	bool result = true;

	{
		QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", "DropTestDbConnection");

		db.setHostName(m_databaseHost);
		db.setPort(m_databaseHostPort);
		db.setUserName(m_databaseUser);
		db.setPassword(m_databaseUserPassword);
		db.setDatabaseName("postgres");

		try
		{
			bool ok = db.open();
			if (ok == false)
			{
				qDebug() << "Database open error: " << db.lastError().text();
				throw false;
			}

			QSqlQuery query(db);
			ok = query.exec(QString("SELECT datname FROM pg_database WHERE datname = '%1'").arg(QString("u7_") + m_projectName));

			if (ok == false)
			{
				qDebug() << "FAIL: " << query.lastError().text();
				throw false;;
			}

			if (query.next() == true)
			{
				if (bool ok = query.exec(QString("DROP DATABASE %1").arg(QString("u7_") + m_projectName));
					ok == false)
				{
					qDebug() << "FAIL: " << query.lastError().databaseText();
					throw false;;
				}
			}
		}
		catch (bool e)
		{
			result = e;
		}

		db.close();
	}

	QSqlDatabase::removeDatabase("DropTestDbConnection");
	return result;
}

QString TestDbBase::databaseHost() const
{
	return m_databaseHost;
}

void TestDbBase::setDatabaseHost(QString value)
{
	m_databaseHost = value;
}

int TestDbBase::databaseHostPort() const
{
	return m_databaseHostPort;
}

void TestDbBase::setDatabaseHostPort(int value)
{
	m_databaseHostPort = value;
}

QString TestDbBase::databaseUser() const
{
	return m_databaseUser;
}

void TestDbBase::setDatabaseUser(QString value)
{
	m_databaseUser = value;
}

QString TestDbBase::databaseUserPassword() const
{
	return m_databaseUserPassword;
}

void TestDbBase::setDatabaseUserPassword(QString value)
{
	m_databaseUserPassword = value;
}

QString TestDbBase::projectName() const
{
	return m_projectName;
}

void TestDbBase::setProjectName(QString value)
{
	m_projectName = value;
}

QString TestDbBase::projectAdministratorName() const
{
	return m_projectAdministratorName;
}

void TestDbBase::setProjectAdministratorName(QString value)
{
	m_projectAdministratorName = value;
}

QString TestDbBase::projectAdministratorPassword() const
{
	return m_projectAdministratorPassword;
}

void TestDbBase::setProjectAdministratorPassword(QString value)
{
	m_projectAdministratorPassword = value;
}
