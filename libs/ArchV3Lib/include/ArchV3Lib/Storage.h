#pragma once

namespace ArchV3
{
	class Storage
	{
	public:
		Storage(const QString& archDir, const QString& projectName, const QString& clientID);
		virtual ~Storage();

		void deleteFiles(const std::vector<QString>& filesToDelete);

		static QString makeArchiveFileName(const QString& appSignalID, quint8 bucket, qint64 timeFromUtc, bool shortTermArchive);
		static QString sanitizeString(QString str);		// copy Ok

	private:
		QString m_archDir;
		QString m_projectName;
		QString m_clientID;
	};
} // namespace ArchV3