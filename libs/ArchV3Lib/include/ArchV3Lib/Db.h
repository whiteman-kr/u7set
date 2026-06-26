#pragma once

#include <memory>

#include <ArchV3Lib/Postgres.h>

namespace ArchV3
{
	class Db : public LogWrapper
	{
	public:
		Db(	const QString& projectID, const QString& appDataSrvID, 
			const DbConnectionInfo& dbConnInfo,
			CircularLoggerShared logger);
		~Db();

		bool open();
		void close();

		bool isOpen() const;

	private:
		bool createSchema();
		QString loadScript(const QString& scriptFileName) const;
		QString makeDatabaseName(const QString& projectId, const QString& appDataSrvId) const;

	private:
		QString m_projectID;
		QString m_appDataSrvID;
		DbConnectionInfo m_dbConnInfo;

		std::unique_ptr<Postgres> m_db;
	};
} // namespace ArchV3