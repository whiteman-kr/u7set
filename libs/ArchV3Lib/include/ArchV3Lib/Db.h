#pragma once

#include <memory>

#include "Postgres.h"
#include "ArchSignal.h"

namespace ArchV3
{
	class Db : public LogWrapper
	{
	private:
		struct RegisteredSignalInfo
		{
			quint64 signalID;
			E::SignalType signalType;
			QString appSignalID;
			Hash hash;
			quint8 bucket;
			qint64 createdUtc;
		};

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

		bool getRegisteredSignals(std::unordered_map<Hash, RegisteredSignalInfo>* registeredSignals) const;
		bool deleteSignals(const std::vector<QString>& ids) const;
		bool registerSignals(const std::vector<ArchSignal>& archSignals, const std::unordered_set<Hash>& signalsToRegister);

		QString makeDatabaseName(const QString& projectId, const QString& appDataSrvId) const;

	private:
		QString m_projectID;
		QString m_appDataSrvID;
		DbConnectionInfo m_dbConnInfo;

		std::unique_ptr<Postgres> m_db;
	};
} // namespace ArchV3