#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <ArchV3Lib/Storage.h>

#include <QDir.h>
#include <QRegularExpression.h>

#include "../../UtilsLib/WUtils.h"

#include <CommonStdLib/TimesStd.h>

namespace ArchV3
{
	Storage::Storage(const QString& archDir,
					 const QString& projectName,
					 const QString& clientID,
					 const DbConnectionInfo& dbConnInfo,
					 const std::vector<ArchSignal>& archSignals,
					 CircularLoggerShared logger) :
		LogWrapper(logger, QString("Storage(%1)").arg(clientID)),
		m_archDir(archDir),
		m_projectName(sanitizeString(projectName)),
		m_clientID(sanitizeString(clientID)),
		m_db(projectName, clientID, dbConnInfo, logger),
		m_stdClientID(m_clientID.toStdString())
	{ 
		if (ArchFileBase::clusterSize() == 0)
		{
			ArchFileBase::readClusterSize(archDir);
			logMsg(QString("disk cluster size %1 bytes").arg(ArchFileBase::clusterSize()));
		}

		m_archPath = m_archDir + QDir::separator() + m_projectName + QDir::separator() + m_clientID;

		createArchFiles(archSignals);
	}

	Storage::~Storage()
	{
	}

	bool Storage::init()
	{ 
		bool result = true;

		result = m_db.open();

		RETURN_IF_FALSE(result);

		result &= checkAndInitDirs();
		result &= initArchFiles();

		return result;
	}

	void Storage::processArchData(const char* archData, size_t archDataSize)
	{ 
		static Network::SaveAppSignalsStatesToArchiveRequest request;

		bool result = request.ParseFromArray(archData, TO_INT(archDataSize));

		if (result == false)
		{
			return;
		}

		if (request.clientequipmentid() != m_stdClientID)
		{
			Q_ASSERT(false);
			return;
		}

		ArchFileInfo afi;

		qint64 nowUTC = currentMSecsUTC();

		for (const Proto::AppSignalState& state : request.appsignalstates())
		{
			Hash hash = state.hash();

			auto it = m_archFiles.find(hash);

			if (it == m_archFiles.end())
			{
				continue;
			}

			std::unique_ptr<ArchFileBase>& archFile = it->second;

			if (archFile->hasActiveFile() == false)
			{
				auto it2 = m_activeFiles.find(hash);

				if (it2 == m_activeFiles.end())
				{
					bool res = createActiveArchFile(archFile, state.systemtime(), &afi);

					if (res == false)
					{
						continue;
					}

					archFile->setActiveFile(afi);
				}
				else
				{
					const ArchFileInfo& registeredAfi = it2->second;
					ArchFileInfo checkedAfi;

					bool res = ArchFileBase::checkFile(registeredAfi, &checkedAfi);

					if (res == true)
					{
						archFile->setActiveFile(registeredAfi);
					}
					else
					{
						res = updateActiveArchFile(checkedAfi);

						if (res == true)
						{
							archFile->setActiveFile(checkedAfi);
						}
						else
						{
							continue;
						}
					}
				}
			}

			bool res2 = archFile->prepareForNextState(&afi);		// check cluster margin here

			if (res2 == false)
			{
				bool res3 = updateActiveArchFile(afi);

				if (res3 == false)
				{
					continue;
				}

				res3 = createNextActiveArchFile(hash, state.systemtime(), &afi);

				if (res3 == true)
				{
					archFile->setActiveFile(afi);
				}
			}

			bool writtenToDisk = archFile->pushState(state, nowUTC, &afi);

			if (writtenToDisk == true)
			{
				updateActiveArchFile(afi);
			}
		}
	}

	void Storage::deleteFiles(const QString& archDir,
							  const QString& projectName,
							  const QString& clientID,
							  const std::vector<QString>& filesToDelete)
	{
		QString path = archDir + QDir::separator() + projectName + QDir::separator() + clientID + QDir::separator();

		for (const QString& filePath : filesToDelete)
		{
			QString fullPath =	path + filePath;

			if (QFile::exists(fullPath) == true)
			{
				QFile::remove(fullPath);
			}
		}
	}

	QString Storage::sanitizeString(QString str)
	{
		str.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
		str.replace(QRegularExpression("_+"), "_");
		str.remove(QRegularExpression("^_+"));
		str.remove(QRegularExpression("_+$"));

		return str;
	}

	QString Storage::makeArchiveFileName(quint8 bucket, const QString& appSignalID, qint64 timeFromUtc, bool shortTermArchive)
	{
		//
		// ShortTermArchiveFileName format: <bucket>/<appSignalID>/<timeFromUtc>.sta
		// LongTermArchiveFileName format: <bucket>/<appSignalID>/<timeFromUtc>.lta
		//

		static const QString shortTermExtension = "sta";
		static const QString longTermExtension = "lta";

		const QDateTime dateTime = QDateTime::fromMSecsSinceEpoch(timeFromUtc, QTimeZone::utc());

		QString fileName = QString("/%1/%2/%3.%4").
								arg(makeBucketStr(bucket)).
								arg(sanitizeString(appSignalID)).
								arg(dateTime.toString(QStringLiteral("yyyyMMdd_HHmmss_zzz.arch"))).
								arg(shortTermArchive ? shortTermExtension : longTermExtension);
		return fileName;
	}

	QString Storage::makeBucketStr(quint8 bucket)
	{ 
		return QString("%1").arg(bucket, 3, 10, QLatin1Char('0')); 
	}

	QString Storage::makeArchiveBucketPath(quint8 bucket) const 
	{ 
		return QString("%1/%2").arg(m_archPath).arg(makeBucketStr(bucket));
	}

	void Storage::createArchFiles(const std::vector<ArchSignal>& archSignals)
	{
		size_t analogsCount = 0;
		size_t discretesCount = 0;

		std::vector<size_t> signalInBucketCount(BUCKET_COUNT, 0);

		for (const ArchSignal& s : archSignals)
		{
			switch(s.signalType)
			{
			case E::SignalType::Analog:
				analogsCount++;
				signalInBucketCount[s.bucket]++;
				break;

			case E::SignalType::Discrete:
				discretesCount++;
				signalInBucketCount[s.bucket]++;
				break;
			}
		}

		m_archFiles.reserve(analogsCount + discretesCount);

		m_bucketSignals.resize(BUCKET_COUNT);

		for (size_t n = 0; n < BUCKET_COUNT; n++)
		{
			if (signalInBucketCount[n] > 0)
			{
				m_bucketSignals[n].reserve(signalInBucketCount[n]);
			}
		}

		//std::unordered_map<quint8, QStringList> signlsInGroups;

		//for (quint32 n = 0; n < 256; n++)
		//{
		//	signlsInGroups[static_cast<quint8>(n)] = QStringList();
		//}

		for (const ArchSignal& s : archSignals)
		{
			switch (s.signalType)
			{
			case E::SignalType::Analog:
				{
					std::unique_ptr<AnalogArchFile> file = std::make_unique<AnalogArchFile>(s.appSignalID);
					m_archFiles.emplace(s.hash, std::move(file));
					m_bucketSignals[s.bucket].push_back(s.hash);
				}
				break;

			case E::SignalType::Discrete:
				{
					std::unique_ptr<DiscreteArchFile> file = std::make_unique<DiscreteArchFile>(s.appSignalID);
					m_archFiles.emplace(s.hash, std::move(file));
					m_bucketSignals[s.bucket].push_back(s.hash);
				}
				break;
			}
		}

//		writeSignalInGropsFile(signlsInGroups);
	}

	bool Storage::initArchFiles()
	{ 
		bool result = m_db.getActiveArchiveFiles(&m_activeFiles);

		//for (const auto& [hash, afi] : activeFiles)
		//{
		//	auto it = m_archFiles.find(hash);

		//	if (it == m_archFiles.end())
		//	{
		//		continue;
		//	}

		//	std::unique_ptr<ArchFileBase>& archFile = it->second;

		//	TEST_PTR_CONTINUE(archFile);

		//	Q_ASSERT(archFile->hasActiveFile() == false);

		//	archFile->setActiveFile(afi);
		//}

		return result;
	}

	bool Storage::createActiveArchFile(std::unique_ptr<ArchFileBase>& archFile, qint64 timeFromUTC, ArchFileInfo* afi)
	{ 
		TEST_PTR_RETURN_FALSE(archFile);

		qint64 createdUTC = currentMSecsUTC();

		Hash hash = calcHash(archFile->appSignalID());
		
		QString fileName = makeArchiveFileName(static_cast<quint8>(hash & 0xFF), archFile->appSignalID(), timeFromUTC, true);

		bool result = m_db.createActiveArchFile(hash, fileName, timeFromUTC, createdUTC, afi);

		return result;
	}

	bool Storage::createNextActiveArchFile(Hash hash, qint64 timeFromUTC, ArchFileInfo* afi)
	{
		Q_ASSERT(false);    // TO DO
		return true;
	}

	bool Storage::updateActiveArchFile(const ArchFileInfo& afi)
	{
		Q_ASSERT(false);    // TO DO
		return true;
	}

	bool Storage::checkAndInitDirs()
	{
		bool result = QDir().mkpath(m_archPath);

		if (result == false)
		{
			logErr(QString("Failed to create archive directory: %1").arg(m_archPath));
			return false;
		}

		QString path;

		for (quint32 n = 0; n < 256; n++)
		{
			path = makeArchiveBucketPath(static_cast<quint8>(n));

			bool res = QDir().mkpath(path);

			if (res == false)
			{
				logErr(QString("Failed to create archive subdirectory: %1").arg(path));
				result = false;
			}
		}

		return result;
	}


} // namespace ArchV3
