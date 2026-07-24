#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <ArchV3Lib/Storage.h>

#include <QDir.h>
#include <QRegularExpression.h>

namespace ArchV3
{
	Storage::Storage(const QString& archDir, const QString& projectName, const QString& clientID) :
		m_archDir(archDir),
		m_projectName(projectName),
		m_clientID(clientID)
	{
	}

	Storage::~Storage()
	{
	}

	void Storage::deleteFiles(const std::vector<QString>& filesToDelete)
	{
		for (const QString& filePath : filesToDelete)
		{
			QString fullPath =	m_archDir + QDir::separator() + 
								m_projectName + QDir::separator() +
								m_clientID + QDir::separator() + filePath;

			if (QFile::exists(fullPath) == true)
			{
				QFile::remove(fullPath);
			}
		}
	}

	QString Storage::makeArchiveFileName(const QString& appSignalID, quint8 bucket, qint64 timeFromUtc, bool shortTermArchive)
	{
		static const QString shortTermExtension = "sta";
		static const QString longTermExtension = "lta";

		const QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(timeFromUtc, Qt::UTC);

		QString fileName = QString("%1%2%3%4%5.%6")
							   .arg(bucket, 3, 10, QLatin1Char('0'))
							   .arg(QDir::separator())
							   .arg(sanitizeString(appSignalID))
							   .arg(QDir::separator())
							   .arg(dateTime.toString(QStringLiteral("yyyyMMdd_HHmmss_zzz.arch")))
							   .arg(shortTermArchive ? shortTermExtension : longTermExtension);
		return fileName;
	}

	QString Storage::sanitizeString(QString str)
	{
		str.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
		str.replace(QRegularExpression("_+"), "_");
		str.remove(QRegularExpression("^_+"));
		str.remove(QRegularExpression("_+$"));

		return str;
	}



} // namespace ArchV3
