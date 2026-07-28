#pragma once

#include <unordered_map>

#include "../../OnlineLib/CircularLogger.h"

#include "ArchSignal.h"
#include "ArchFile.h"

namespace ArchV3
{
	class Storage : public LogWrapper
	{
	public:
		Storage(const QString& archDir,
				const QString& projectName,
				const QString& clientID,
				const std::vector<ArchSignal>& archSignals,
				CircularLoggerShared logger);
			
		virtual ~Storage();

		bool init();

		static void deleteFiles(const QString& archDir,
								const QString& projectName,
								const QString& clientID,
								const std::vector<QString>& filesToDelete);

		static QString sanitizeString(QString str); // copy Ok
		static QString makeArchiveFileName(quint8 bucket, const QString& appSignalID, qint64 timeFromUtc, bool shortTermArchive);
		static QString makeBucketStr(quint8 bucket);

	private:
		QString makeArchiveBucketPath(quint8 bucket) const;

		void initArchFiles(const std::vector<ArchSignal>& archSignals);

		bool checkAndInitDirs();

	private:
		const QString m_archDir;
		const QString m_projectName;
		const QString m_clientID;

		QString m_archPath;

		static constexpr size_t BUCKET_COUNT = 256;

		std::unordered_map<Hash, std::unique_ptr<ArchFileBase>> m_archFiles;
		std::vector<std::vector<Hash>> m_bucketSignals;
		
	};
} // namespace ArchV3