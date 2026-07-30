#pragma once

#include <unordered_map>

#include "../../OnlineLib/CircularLogger.h"

#include "ArchSignal.h"
#include "ArchFile.h"
#include "Db.h"

namespace ArchV3
{
	class Storage : public LogWrapper
	{
	public:
		Storage(const QString& archDir,
				const QString& projectName,
				const QString& clientID,
				const DbConnectionInfo& dbConnInfo,
				const std::vector<ArchSignal>& archSignals,
				CircularLoggerShared logger);
			
		virtual ~Storage();

		bool init();

		void processArchData(const char* archData, size_t archDataSize);

		static void deleteFiles(const QString& archDir,
								const QString& projectName,
								const QString& clientID,
								const std::vector<QString>& filesToDelete);

		static QString sanitizeString(QString str); // copy Ok
		static QString makeArchiveFileName(quint8 bucket, const QString& appSignalID, qint64 timeFromUtc, bool shortTermArchive);
		static QString makeBucketStr(quint8 bucket);

	private:
		QString makeArchiveBucketPath(quint8 bucket) const;

		void createArchFiles(const std::vector<ArchSignal>& archSignals);
		bool initArchFiles();

		bool createActiveArchFile(Hash hash, qint64 timeFromUTC, ArchFileInfo* afi);
		bool createNextActiveArchFile(Hash hash, qint64 timeFromUTC, ArchFileInfo* afi);
		bool updateActiveArchFile(const ArchFileInfo& afi);

		bool checkAndInitDirs();

	private:
		const QString m_archDir;
		const QString m_projectName;
		const QString m_clientID;
		const std::string m_stdClientID;

		QString m_archPath;

		Db m_db;

		static constexpr size_t BUCKET_COUNT = 256;

		std::unordered_map<Hash, ArchFileInfo> m_activeFiles;
		std::unordered_map<Hash, std::unique_ptr<ArchFileBase>> m_archFiles;
		std::vector<std::vector<Hash>> m_bucketSignals;
		
	};
} // namespace ArchV3