#pragma once

#include <ArchV3Lib/Postgres.h>

namespace ArchV3
{
	class Db : public LogWrapper
	{
	public:
		Db(	const QString& projectID, const QString& appDataSrvID, 
			const DbConnectionInfo& dbConnInfo,
			CircularLoggerShared logger, const QString& className);
		~Db();

		bool open();
		void close();

	private:
		QString makeDatabaseName(const QString& projectId, const QString& appDataSrvId) const;

	private:
		QString m_projectID;
		QString m_appDataSrvID;
		DbConnectionInfo m_dbConnInfo;

		std::unique_ptr<Postgres> m_db;
		std::unique_ptr<Postgres> m_postgresDb;
	};
} // namespace ArchV3