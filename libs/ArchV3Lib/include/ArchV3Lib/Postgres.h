#pragma once

#include <QtSql/QSqlDatabase>

#include "../../OnlineLib/CircularLogger.h"

namespace ArchV3
{
	struct DbConnectionInfo
	{
		QString host;
		quint16 port = 0;
		QString user;
		QString password;
	};

	class Postgres : public LogWrapper
	{
	public:
		Postgres(const DbConnectionInfo& dbConnInfo, const QString& dbName,
				CircularLoggerShared logger);
		~Postgres();

		Postgres(const Postgres&) = delete;
		Postgres& operator=(const Postgres&) = delete;

		Postgres(Postgres&&) = delete;
		Postgres& operator=(Postgres&&) = delete;

		//

		bool open();
		void close();

		bool isOpen() const;

		bool execSql(const QString& sql) const;

		bool tableExists(const QString& schemaName, const QString& tableName) const;
		bool tableExists(const QString& tableName) const;

		QString loadScript(const QString& scriptFileName) const;
		bool executeScript(const QString& script) const;

		bool loadAndExecuteScript(const QString& scriptFileName) const;

		// for 'postgres' database only

		bool createDatabase(const QString& dbName) const;
		bool dropDatabases(const QString& databaseNamePattern) const;		// like "u7arch_test_%"

	private:

		bool isPostgresDatabase() const;

	private: 
		DbConnectionInfo m_dbConnInfo;
		QString m_dbName;

		QString m_connectionName;
		QSqlDatabase m_db;

		static constexpr QLatin1StringView SQL_RESOURCE_PREFIX{":/ArchV3Lib/Sql/"};
		static constexpr QLatin1StringView DEFAULT_SCHEMA{"public"};
	};
}