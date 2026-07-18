#pragma once

#include <memory>

#include "Postgres.h"
#include "ArchSignal.h"

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

		bool registerSignals(const std::vector<ArchSignal>& archSignals);

	private:
		bool schemaCheckAndCreate();

		bool schemaCreate();
		bool schemaCleanup();

		bool typesCreate();
		bool functionsCreate();

		QString makeDatabaseName(const QString& projectId, const QString& appDataSrvId) const;

	private:
		QString m_projectID;
		QString m_appDataSrvID;
		DbConnectionInfo m_dbConnInfo;

		std::unique_ptr<Postgres> m_db;
	};
} // namespace ArchV3