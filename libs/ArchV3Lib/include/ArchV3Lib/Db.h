#pragma once

#include <memory>

#include "Postgres.h"
#include "ArchSignal.h"
#include "ArchFile.h"

namespace ArchV3
{
	class Db : public LogWrapper
	{
	private:
		struct RegisteredSignalInfo
		{
			qint64 signalID;
			E::SignalType signalType;
			QString appSignalID;
			Hash hash;
			quint8 bucket;
			qint64 createdUTC;
		};

	public:
		static constexpr qint64 BAD_ARCHIVE_FILE_ID = -1;

	public:
		Db(	const QString& projectID, const QString& appDataSrvID, 
			const DbConnectionInfo& dbConnInfo,
			CircularLoggerShared logger);
		~Db();

		bool open();
		void close();

		bool isOpen() const;

		bool registerSignals(const std::vector<ArchSignal>& archSignals, std::vector<QString>* filesToDelete);
		bool getActiveArchiveFiles(std::unordered_map<Hash, ArchFileInfo>* activeFiles) const;
		bool createActiveArchFile(Hash hash, const QString& fileName, qint64 timeFromUtc, qint64 createdUtc, ArchFileInfo* afi);
		
	private:
		bool schemaCheckAndCreate();

		bool schemaCreate();
		bool schemaCleanup();

		bool typesCreate();
		bool functionsCreate();

		bool getRegisteredSignals(std::unordered_map<Hash, RegisteredSignalInfo>* registeredSignals) const;
		bool deleteSignals(const std::vector<QString>& ids, std::vector<QString>* filesToDelete) const;
		bool registerSignals(const std::vector<ArchSignal>& archSignals, const std::unordered_set<Hash>& signalsToRegister);
//		qint64 createArchiveFile(qint64 signalID, const QString& appSignalID, qint64 timeFromUtc, qint64 createdUtc) const;

		QString makeDatabaseName(const QString& projectId, const QString& appDataSrvId) const;

	private:
		QString m_projectID;
		QString m_appDataSrvID;
		DbConnectionInfo m_dbConnInfo;

		std::unique_ptr<Postgres> m_db;
	};
} // namespace ArchV3