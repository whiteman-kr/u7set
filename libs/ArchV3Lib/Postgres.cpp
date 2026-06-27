#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <QUuid>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <ArchV3Lib/Postgres.h>

#include <CommonLib/ConstStrings.h>

namespace ArchV3
{
	constexpr QLatin1StringView SHEMA_PUBLIC("public");

	Postgres::Postgres(const DbConnectionInfo& dbConnInfo, const QString& dbName, 
		CircularLoggerShared logger) :
		LogWrapper(logger, QString("Db '%1'").arg(dbName)),
		m_dbConnInfo(dbConnInfo),
		m_dbName(dbName)
	{ 
	}

	Postgres::~Postgres()
	{ 
		close();
	}

	bool Postgres::open()
	{
		if (isOpen() == true)
		{
			Q_ASSERT(false);
			close();
		}

		m_connectionName = m_dbName + "_" + QUuid::createUuid().toString(QUuid::WithoutBraces);

		m_db = QSqlDatabase::addDatabase("QPSQL", m_connectionName);

		m_db.setHostName(m_dbConnInfo.host);
		m_db.setPort(m_dbConnInfo.port);
		m_db.setDatabaseName(m_dbName);
		m_db.setUserName(m_dbConnInfo.user);
		m_db.setPassword(m_dbConnInfo.password);

		if (m_db.open() == false)
		{
			logErr(QString("failed to open: %1").arg(m_db.lastError().text()));
			close();
			return false;
		}

		QSqlQuery q(m_db);

		if (q.exec("SET client_encoding TO 'UTF8'") == false)
		{
			logErr(q.lastError().text());
			close();
			return false;
		}

		logMsg("opened");

		return true;
	}

	void Postgres::close()
	{
		const bool wasOpen = m_db.isOpen();

		if (wasOpen == true)
		{
			m_db.close();
		}

		m_db = QSqlDatabase();

		if (m_connectionName.isEmpty() == false)
		{
			QSqlDatabase::removeDatabase(m_connectionName);
		}

		if (wasOpen == true)
		{
			logMsg("closed");
		}

		m_connectionName.clear();
	}

	bool Postgres::isOpen() const
	{ 
		return m_db.isOpen(); 
	}

	bool Postgres::tableExists(const QString& schemaName, const QString& tableName)
	{
		if (isOpen() == false)
		{
			logErr("database not open!");
			return false;
		}

		QSqlQuery query(m_db);

		query.prepare("SELECT 1 "
					  "FROM information_schema.tables "
					  "WHERE table_schema = :shema_name "
					  "AND table_name = :table_name");

		query.bindValue(":schema_name", schemaName);
		query.bindValue(":table_name", tableName);

		if (query.exec() == false)
		{
			logErr(query.lastError().text());
			return false;
		}

		return query.next();
	}

	bool Postgres::tableExists(const QString& tableName)
	{ 
		return tableExists(SHEMA_PUBLIC, tableName);
	}

		QString Postgres::loadScript(const QString& scriptFileName) const
	{
		const QString resourcePath = QString(":/ArchV3Lib/Sql/%1").arg(scriptFileName);

		QFile file(resourcePath);

		if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
		{
			logErr(QString("failed to open SQL script '%1': %2").arg(resourcePath).arg(file.errorString()));
			return QString();
		}

		return QString::fromUtf8(file.readAll());
	}

	bool Postgres::executeScript(const QString& script) const
	{
		if (isOpen() == false)
		{
			logErr("database not open!");
			return false;
		}

		QSqlQuery query(m_db);

		if (query.exec(script) == false)
		{
			logErr(QString("failed to execute SQL script: %1").arg(query.lastError().text()));
			return false;
		}

		return true;
	}

	bool Postgres::loadAndExecuteScript(const QString& scriptFileName) const
	{ 
		QString script = loadScript(scriptFileName);

		if (script.isEmpty())
		{
			return false;
		}

		return executeScript(script);
	}

	bool Postgres::createDatabase(const QString& dbName)
	{
		if (isPostgresDatabase() == false)
		{
			return false;
		}

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
			logMsg(QString("database '%1' already exists").arg(dbName));
			return true;
		}

		const QString sql = QString("CREATE DATABASE \"%1\"").arg(dbName);

		if (query.exec(sql) == false)
		{
			logErr(QString("failed to create database '%1': %2").arg(dbName).arg(query.lastError().text()));
			return false;
		}

		logMsg(QString("database '%1' created").arg(dbName));

		return true;
	}

	bool Postgres::dropDatabases(const QString& databaseNamePattern)
	{
		if (isPostgresDatabase() == false)
		{
			return false;
		}

		QSqlQuery query(m_db);

		query.prepare("SELECT datname "
					  "FROM pg_database "
					  "WHERE datname LIKE :pattern");

		query.bindValue(":pattern", databaseNamePattern);

		if (query.exec() == false)
		{
			logErr(query.lastError().text());

			return false;
		}

		bool result = true;

		while (query.next())
		{
			const QString dbName = query.value(0).toString();

			QSqlQuery terminateQuery(m_db);

			terminateQuery.prepare("SELECT pg_terminate_backend(pid) "
								   "FROM pg_stat_activity "
								   "WHERE datname = :db_name "
								   "AND pid <> pg_backend_pid()");

			terminateQuery.bindValue(":db_name", dbName);

			if (terminateQuery.exec() == false)
			{
				logErr(QString("failed to terminate connections for database '%1': %2").
					arg(dbName).arg(terminateQuery.lastError().text()));
			}

			QSqlQuery dropQuery(m_db);

			const QString sql = QString("DROP DATABASE \"%1\"").arg(dbName);

			if (dropQuery.exec(sql) == false)
			{
				logErr(QString("failed to drop database '%1': %2").arg(dbName).arg(dropQuery.lastError().text()));
				result = false;
				continue;
			}

			logMsg(QString("database '%1' dropped").arg(dbName));
		}

		return result;
	}

	bool Postgres::isPostgresDatabase() const
	{
		if (m_dbName != DbName::POSTGRES)
		{
			Q_ASSERT(false);
			return false;
		}

		if (isOpen() == false)
		{
			logErr("database not open!");
			return false;
		}

		return true;
	}
}