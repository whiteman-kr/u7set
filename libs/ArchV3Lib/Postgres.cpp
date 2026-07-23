#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <mutex>

#include <QUuid>
#include <QtSql/QSqlError>
#include <QRegularExpression>
#include <QFile>

#include <ArchV3Lib/Postgres.h>

#include <CommonLib/ConstStrings.h>

namespace ArchV3
{
	Postgres::Postgres(const DbConnectionInfo& dbConnInfo, const QString& dbName, 
		CircularLoggerShared logger) :
		LogWrapper(logger, QString("Db '%1'").arg(dbName)),
		m_dbConnInfo(dbConnInfo),
		m_dbName(dbName)
	{
		static std::once_flag once;

		std::call_once(once,
					   []()
					   {
						   Q_INIT_RESOURCE(Sql);
					   });
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

	bool Postgres::execSql(const QString& sql) const
	{
		if (isOpen() == false)
		{
			logErr("database not open!");
			return false;
		}

		QSqlQuery query(m_db);

		if (query.exec(sql) == false)
		{
			logErr(QString("failed to execute SQL: %1").arg(query.lastError().text()));
			return false;
		}

		return true;
	}

	std::optional<QSqlQuery> Postgres::execQuery(const QString& sql) const
	{
		if (isOpen() == false)
		{
			logErr("database not open!");
			return std::nullopt;
		}

		QSqlQuery query(m_db);

		if (query.exec(sql) == false)
		{
			logErr(QString("failed to execute query %1: %2").arg(sql).arg(query.lastError().text()));
			return std::nullopt;
		}

		return query;
	}

	bool Postgres::tableExists(const QString& schemaName, const QString& tableName) const
	{
		if (isOpen() == false)
		{
			logErr("database not open!");
			return false;
		}

		QSqlQuery query(m_db);

		if (query.prepare("SELECT 1 "
			"FROM information_schema.tables "
			"WHERE table_schema = :schema_name "
			"AND table_name = :table_name") == false)
		{
			logErr(query.lastError().text());
			return false;
		}

		query.bindValue(":schema_name", schemaName);
		query.bindValue(":table_name", tableName);

		if (query.exec() == false)
		{
			logErr(query.lastError().text());
			return false;
		}

		return query.next();
	}

	bool Postgres::tableExists(const QString& tableName) const
	{ 
		return tableExists(DEFAULT_SCHEMA, tableName);
	}

	QString Postgres::loadScript(const QString& scriptFileName) const
	{
		const QString resourcePath = QString("%1/%2").arg(SQL_RESOURCE_PREFIX).arg(scriptFileName);

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

		static const QRegularExpression dollarQuotedRegex(R"(\$[A-Za-z0-9_]*\$)");

		bool noSplit = script.contains(dollarQuotedRegex);

		if (noSplit)
		{
			QSqlQuery query(m_db);

			if (query.exec(script) == false)
			{
				logErr(QString("failed to execute SQL script: %1").arg(query.lastError().text()));
				return false;
			}
		}
		else
		{
			QStringList statements = script.split(";", Qt::SkipEmptyParts);
			
			for (const QString& statement : statements)
			{
				QSqlQuery query(m_db);

				if (query.exec(statement.trimmed()) == false)
				{
					logErr(QString("failed to execute SQL statement %1: %2").
						arg(statement.trimmed()).arg(query.lastError().text()));
					return false;
				}
			}
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

		bool result = executeScript(script);

		if (result)
		{
			logMsg(QString("SQL script '%1' executed successfully").arg(scriptFileName));
		}
		else
		{
			logErr(QString("SQL script '%1' execution ERROR!").arg(scriptFileName));
		}

		return result;
	}

	bool Postgres::createDatabase(const QString& dbName) const
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

		//const QString sql2 = QString("ALTER DATABASE %1 SET lc_messages TO 'C'").arg(dbName);

		//if (query.exec(sql2) == false)
		//{
		//	logErr(QString("failed to alter database '%1': %2").arg(dbName).arg(query.lastError().text()));
		//	return false;
		//}

		logMsg(QString("database '%1' created").arg(dbName));

		return true;
	}

	bool Postgres::dropDatabases(const QString& databaseNamePattern) const
	{
		if (isPostgresDatabase() == false)
		{
			return false;
		}

		QSqlQuery query(m_db);

		query.prepare("SELECT datname "
					  "FROM pg_database "
					  "WHERE datname ILIKE :pattern");

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