#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <QLatin1StringView>

#include <ArchV3Lib/Db.h>
#include <ArchV3Lib/Utils.h>
#include <ArchV3Lib/Storage.h>

#include <CommonLib/ConstStrings.h>

#include "../../UtilsLib/WUtils.h"

namespace ArchV3
{
	constexpr QLatin1StringView TABLE_ARCHIVE_INFO("archive_info");

	Db::Db(	const QString& projectID, const QString& appDataSrvID, 
			const DbConnectionInfo& dbConnInfo,
			CircularLoggerShared logger) :
		LogWrapper(logger, QString("Db '%1'").arg(makeDatabaseName(projectID, appDataSrvID))),
		m_projectID(projectID),  
		m_appDataSrvID(appDataSrvID),
		m_dbConnInfo(dbConnInfo)
	{
	}

	Db::~Db()
	{ 
		close();
	}	

	bool Db::open()
	{ 
		if (isOpen() == true)
		{
			Q_ASSERT(false);
			close();
		}

		QString dbName = makeDatabaseName(m_projectID, m_appDataSrvID);

		{
			Postgres postgresDb(m_dbConnInfo, DbName::POSTGRES, getLog());

			if (postgresDb.open() == false)
			{
				return false;
			}

			if (postgresDb.createDatabase(dbName) == false)
			{
				return false;
			}
		}

		m_db = std::make_unique<Postgres>(m_dbConnInfo, dbName, getLog());

		if (m_db->open() == false)
		{
			close();
			return false;
		}

		if (schemaCheckAndCreate() == false)
		{
			close();
			return false;
		}

		return true;
	}

	void Db::close() 
	{
		if (m_db != nullptr)
		{
			m_db->close();
			m_db.reset();
		}
	}

	bool Db::isOpen() const
	{
		return m_db != nullptr && m_db->isOpen(); 
	}

	bool Db::registerSignals(const std::vector<ArchSignal>& archSignals, std::vector<QString>* filesToDelete)
	{
		TEST_PTR_RETURN_FALSE(filesToDelete);

		std::unordered_map<Hash, RegisteredSignalInfo> registeredSignals;

		bool result = getRegisteredSignals(&registeredSignals);

		RETURN_IF_FALSE(result);

		std::unordered_set<Hash> signalsToRegister;
		std::vector<QString> signalsToDelete;

		for (const ArchSignal& signal : archSignals)
		{
			Hash hash = calcHash(signal.appSignalID);
			quint8 bucket = hash & 0xFF;

			auto it = registeredSignals.find(hash);

			if (it == registeredSignals.end())
			{
				signalsToRegister.insert(hash);
				continue;
			}

			// signal already restered, check if it needs to be updated

			RegisteredSignalInfo& rsi = it->second;
			
			if (rsi.signalType != signal.signalType ||
				rsi.hash != hash ||
				rsi.bucket != bucket)
			{
				signalsToDelete.push_back(signal.appSignalID);
				signalsToRegister.insert(hash);
			}
		}

		result = deleteSignals(signalsToDelete, filesToDelete);

		if (result == false)
		{
			filesToDelete->clear();
		}

		RETURN_IF_FALSE(result);

		result = registerSignals(archSignals, signalsToRegister);

		return result;
	}

		bool Db::getActiveArchiveFiles(std::unordered_map<Hash, ArchFileInfo>* activeFiles) const
	{
		TEST_PTR_RETURN_FALSE(activeFiles);

		activeFiles->clear();

		static constexpr int COL_ARCH_FILE_ID = 0;
		static constexpr int COL_SIGNAL_ID = 1;
		static constexpr int COL_SIGNAL_TYPE = 2;
		static constexpr int COL_HASH = 3;
		static constexpr int COL_BUCKET = 4;
		static constexpr int COL_FILE_NAME = 5;
		static constexpr int COL_TIME_FROM_UTC = 6;
		static constexpr int COL_TIME_TO_UTC = 7;
		static constexpr int COL_RECORD_COUNT = 8;
		static constexpr int COL_FILE_SIZE = 9;
		static constexpr int COL_CREATED_UTC = 10;

		auto query = m_db->execQuery(QStringLiteral(R"(
			SELECT
				archive_file_id,
				signal_id,
				signal_type,
				hash,
				bucket,
				file_name,
				time_from_utc,
				time_to_utc,
				record_count,
				file_size,
				created_utc
			FROM fn_get_active_archive_files();
		)"));

		if (!query)
		{
			return false;
		}

		if (query->size() > 0)
		{
			activeFiles->reserve(query->size());
		}

		ArchFileInfo afi;

		while (query->next())
		{
			afi.archFileID = query->value(COL_ARCH_FILE_ID).toLongLong();
			afi.signalID = query->value(COL_SIGNAL_ID).toLongLong();

			afi.hash = static_cast<Hash>(query->value(COL_HASH).toULongLong());
			afi.bucket = static_cast<quint8>(query->value(COL_BUCKET).toInt());
			afi.signalType = static_cast<E::SignalType>(query->value(COL_SIGNAL_TYPE).toInt());

			afi.fileName = query->value(COL_FILE_NAME).toString();

			afi.createdUTC = query->value(COL_CREATED_UTC).toLongLong();
			afi.timeFromUTC = query->value(COL_TIME_FROM_UTC).toLongLong();
			afi.timeToUTC = query->value(COL_TIME_TO_UTC).toLongLong();

			afi.recordCount = query->value(COL_RECORD_COUNT).toLongLong();
			afi.fileSize = query->value(COL_FILE_SIZE).toLongLong();

			auto [it, inserted] = activeFiles->emplace(afi.hash, afi);

			Q_ASSERT(inserted);
		}

		return true;
	}

	bool Db::schemaCheckAndCreate()
	{
		if (isOpen() == false)
		{
			logErr("database not open!");
			return false;
		}

		bool result = true;

		if (m_db->tableExists(TABLE_ARCHIVE_INFO) == false)
		{
			result &= schemaCreate();
			result &= typesCreate();
			result &= functionsCreate();

			if (result)
			{
				logMsg("schema created successfully");
			}
			else
			{
				logErr("schema creation ERROR!");
			}
		}
		else
		{
			logErr("schema already exists");
		}

		RETURN_IF_FALSE(result);

		return result;
	}

	bool Db::schemaCreate()
	{ 
		TEST_PTR_RETURN_FALSE(m_db);
		
		bool result = true;

		result &= m_db->loadAndExecuteScript("Schema/create.sql");
		result &= m_db->loadAndExecuteScript("Schema/create_partitions.sql");

		return result;
	}

	bool Db::schemaCleanup()
	{
		TEST_PTR_RETURN_FALSE(m_db);

		bool result = true;

		result &= m_db->loadAndExecuteScript("Schema/cleanup.sql");

		return result;
	}

	bool Db::typesCreate()
	{ 
		TEST_PTR_RETURN_FALSE(m_db);

		bool result = true;

		result &= m_db->loadAndExecuteScript("Types/signal_register_info.sql");

		return result;
	}

	bool Db::functionsCreate()
	{
		TEST_PTR_RETURN_FALSE(m_db);

		bool result = true;

		result &= m_db->loadAndExecuteScript("Functions/fn_register_signals.sql");
		result &= m_db->loadAndExecuteScript("Functions/fn_get_registered_signals.sql");
		result &= m_db->loadAndExecuteScript("Functions/fn_delete_signals_by_hash.sql");
		result &= m_db->loadAndExecuteScript("Functions/fn_create_archive_file.sql");
		result &= m_db->loadAndExecuteScript("Functions/fn_get_active_archive_files.sql");

		return result;
	}

	bool Db::getRegisteredSignals(std::unordered_map<Hash, RegisteredSignalInfo>* registeredSignals) const
	{ 
		TEST_PTR_RETURN_FALSE(registeredSignals);	

		registeredSignals->clear();

		static constexpr int COL_SIGNAL_ID = 0;
		static constexpr int COL_SIGNAL_TYPE = 1;
		static constexpr int COL_APP_SIGNAL_ID = 2;
		static constexpr int COL_HASH = 3;
		static constexpr int COL_BUCKET = 4;
		static constexpr int COL_CREATED_UTC = 5;

		auto query = m_db->execQuery(QStringLiteral(R"(
		   SELECT
			    signal_id,
				signal_type,
				app_signal_id,
				hash,
				bucket,
				created_utc
			FROM fn_get_registered_signals();
		)"));

		if (!query)
		{
			return false;
		}

		if (query->size() > 0)
		{
			registeredSignals->reserve(query->size());
		}

		RegisteredSignalInfo rsi;

		while (query->next())
		{
			rsi.signalID = query->value(COL_SIGNAL_ID).toLongLong();
			rsi.signalType = static_cast<E::SignalType>(query->value(COL_SIGNAL_TYPE).toInt());
			rsi.appSignalID = query->value(COL_APP_SIGNAL_ID).toString();
			rsi.hash = static_cast<Hash>(query->value(COL_HASH).toULongLong());
			rsi.bucket = static_cast<quint8>(query->value(COL_BUCKET).toInt());
			rsi.createdUTC = query->value(COL_CREATED_UTC).toLongLong();

			auto [it, inserted] = registeredSignals->emplace(rsi.hash, rsi);

			Q_ASSERT(inserted);
		}

		return true;
	}

	bool Db::deleteSignals(const std::vector<QString>& ids, std::vector<QString>* filesToDelete) const
	{
		TEST_PTR_RETURN_FALSE(filesToDelete);

		if (ids.empty())
		{
			return true;
		}

		filesToDelete->clear();

		QStringList values;

		for (const QString& id : ids)
		{
			values << QString::number(calcHash(id));
		}

		const QString sql = QString("SELECT * FROM fn_delete_signals_by_hash(ARRAY[%1]::BIGINT[])").arg(values.join(","));

		auto query = m_db->execQuery(sql);

		if (!query)
		{
			return false;
		}

		filesToDelete->reserve(query->size());

		while (query->next())
		{
			QString filePath = query->value(0).toString();

			if (filePath.isEmpty() == false)
			{
				filesToDelete->push_back(filePath);
			}
		}

		return true;
	}

	bool Db::registerSignals(const std::vector<ArchSignal>& archSignals, const std::unordered_set<Hash>& signalsToRegister)
	{
		if (signalsToRegister.empty())
		{
			return true;
		}

		bool result = true;

		QStringList values;

		quint32 ctr = 0;

		const QString query("SELECT fn_register_signals(ARRAY[%1]::signal_register_info[], %2::BIGINT)");

		qint64 utc = QDateTime::currentMSecsSinceEpoch();

		for (const ArchSignal& signal : archSignals)
		{
			Hash hash = calcHash(signal.appSignalID);

			if (signalsToRegister.find(hash) == signalsToRegister.end())
			{
				continue;
			}

			QString value = QString("('%1', %2, %3, %4)").
								arg(signal.appSignalID).
								arg(static_cast<qint64>(hash)).
								arg(static_cast<quint8>(signal.signalType)).
								arg(static_cast<quint8>(hash & 0xFF));
			values << value;

			ctr++;

			if (ctr >= 1000)
			{
				const QString sql = QString(query).arg(values.join(",")).arg(utc);

				if (m_db->execSql(sql) == false)
				{
					result = false;
				}

				values.clear();
				ctr = 0;
			}
		}

		if (values.isEmpty() == false)
		{
			const QString sql = QString(query).arg(values.join(",")).arg(utc);

			if (m_db->execSql(sql) == false)
			{
				result = false;
			}
		}

		return result;
	}

	qint64 Db::createArchiveFile(qint64 signalID, const QString& appSignalID, qint64 timeFromUtc, qint64 createdUtc) const
	{
		quint8 bucket = static_cast<quint8>(calcHash(appSignalID) & 0xFF);

		const QString fileName = Storage::makeArchiveFileName(bucket, appSignalID, timeFromUtc, true);
		const QString sql = QString("SELECT fn_create_archive_file(%1, %2, '%3', %4, %5)").
							arg(signalID).arg(bucket).arg(timeFromUtc).arg(createdUtc);

		auto query = m_db->execQuery(sql);

		if (!query || query->next() == false)
		{
			return BAD_ARCHIVE_FILE_ID;
		}

		return query->value(0).toLongLong();
	}

	QString Db::makeDatabaseName(const QString& projectID, const QString& appDataSrvID) const
	{ 
		return QString("u7arch_%1_%2").arg(sanitizeID(projectID)).arg(sanitizeID(appDataSrvID)).toLower();
	}
} // namespace ArchV3