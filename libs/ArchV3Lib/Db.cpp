#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <QUuid>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <ArchV3Lib/Db.h>
#include <ArchV3Lib/Utils.h>

namespace ArchV3
{
	Db::Db(	const QString& projectID, const QString& appDataSrvID, 
			const QString& host, quint16 port,
			CircularLoggerShared logger, const QString& className) :
			LogWrapper(logger, className),
			m_projectID(projectID),
			m_appDataSrvID(appDataSrvID),
			m_host(host),
			m_port(port)
	{
		m_user ="u7arch";
		m_password = "P2ssw0rd";
	}

	Db::~Db()
	{ 
		close(); 
	}	

	bool Db::open()
	{ 
		if (isOpen() == true)
		{
			Q_ASSERT(false);
			close();
		}

		if (!openDatabase("postgres") == false)
		{
			return false;
		}

		m_dbName = makeDatabaseName(m_projectID, m_appDataSrvID);

		if (createDatabaseIfNeeded(m_dbName) == false)
		{
			close();
			return false;
		}

		close();

		if (openDatabase(m_dbName) == false)
		{
			return false;
		}

		return true;
	}

	void Db::close()
	{
		if (m_db.isOpen())
		{
			m_db.close();
		}

		m_db = QSqlDatabase();

		if (m_connectionName.isEmpty() == false)
		{
			QSqlDatabase::removeDatabase(m_connectionName);
		}

		logMsg(QString("Database '%1' is closed").arg(m_dbName));

		m_dbName.clear();
		m_connectionName.clear();
	}

	bool Db::isOpen() const
	{ 
		return m_db.isOpen();
	}

	bool Db::openDatabase(const QString& dbName)
	{
		if (isOpen() == true)
		{
			Q_ASSERT(false);
			close();
		}

		m_connectionName = dbName + "_" + QUuid::createUuid().toString(QUuid::WithoutBraces);

		m_db = QSqlDatabase::addDatabase("QPSQL", m_connectionName);

		m_db.setHostName(m_host);
		m_db.setPort(m_port);
		m_db.setDatabaseName(m_dbName);
		m_db.setUserName(m_user);
		m_db.setPassword(m_password);

		if (m_db.open() == false)
		{
			logErr(QString("Failed to open database '%1': %2").arg(dbName).arg(m_db.lastError().text()));

			close();

			return false;
		}

		logMsg(QString("Database '%1' is opened").arg(dbName));

		return true;

	}

	bool Db::createDatabaseIfNeeded(const QString& dbName)
	{
		QSqlQuery query(m_db);

		query.prepare("SELECT 1 FROM pg_database WHERE datname = :db_name");
		query.bindValue(":db_name", dbName);

		if (query.exec() == false)
		{
			logErr(query.lastError().text());
			return false;
		}

		if (query.next() == true)
		{
			logMsg(QString("Database '%1' already exists").arg(dbName));
			return true;
		}

		const QString sql = QString("CREATE DATABASE %1").arg(dbName);

		if (query.exec(sql) == false)
		{
			logErr(QString("Failed to create database '%1': %2").arg(dbName).arg(query.lastError().text()));
			return false;
		}

		return true;
	}

	bool Db::createSchemaIfNeeded()
	{ 
		return true;
	}

	QString Db::makeDatabaseName(const QString& projectID, const QString& appDataSrvID) const
	{ 
		return QString("u7arch_%1_%2").arg(sanitizeID(projectID)).arg(sanitizeID(appDataSrvID));
	}
} // namespace ArchV3