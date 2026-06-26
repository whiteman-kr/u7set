#pragma once

#include "../../OnlineLib/CircularLogger.h"

#include <QtSql/QSqlDatabase>

namespace ArchV3
{
	class Db : public LogWrapper
	{
	public:
		Db(	const QString& projectID, const QString& appDataSrvID, 
			const QString& host, quint16 port,
			CircularLoggerShared logger, const QString& className);
		~Db();

		Db(const Db&) = delete;
		Db& operator=(const Db&) = delete;

		Db(Db&&) = delete;
		Db& operator=(Db&&) = delete;

		bool open();
		void close();

		bool isOpen() const;

/* bool createDatabaseIfNeeded();
		bool createSchemaIfNeeded();
		bool checkSchemaVersion();

		bool registerSignal(const QString& signalId, quint32 signalType);

		bool openArchiveFile(const QString& signalId,
							 const QString& relativePath,
							 const QString& fileName,
							 qint64 timeFromUTC,
							 quint64& fileDbId);

		bool closeArchiveFile(quint64 fileDbId, qint64 timeToUTC, qint64 recordCount, qint64 fileSize);

		QVector<ArchiveFileInfo> findArchiveFiles(const QString& signalId, qint64 timeFromUTC, qint64 timeToUTC);

		bool markCompressed(quint64 fileDbId, const QString& compressedFileName, qint64 compressedSize);

		bool markDeleted(quint64 fileDbId);*/

	private:
		bool openDatabase(const QString& dbName);
		bool createDatabaseIfNeeded(const QString& dbName);
		bool createSchemaIfNeeded();

		QString makeDatabaseName(const QString& projectId, const QString& appDataSrvId) const;

	private:
		QString m_projectID;
		QString m_appDataSrvID;
		QString m_host;
		quint16 m_port;
		QString m_user;
		QString m_password;
		
		QString m_dbName;
		QString m_connectionName;
		QSqlDatabase m_db;
	};
} // namespace ArchV3