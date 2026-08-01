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

		auto query = m_db->execQuery(QStringLiteral("SELECT * FROM fn_get_active_arch_files()"));

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
			bool res = afi.fromQuery(*query);

			if (res == false)
			{
				return false;
			}

			auto [it, inserted] = activeFiles->emplace(afi.hash, afi);

			if (inserted == false)
			{
				Q_ASSERT(false);
				return false;
			}
		}

		return true;
	}

	bool Db::createActiveArchFile(Hash hash, const QString& fileName, qint64 timeFromUtc, qint64 createdUtc, ArchFileInfo* afi)
	{
		TEST_PTR_RETURN_FALSE(afi);

		QSqlQuery query(m_db->db());

		bool res = query.prepare(QStringLiteral(R"(
					SELECT *
					FROM fn_create_active_arch_file
					(
						CAST(:hash AS BIGINT),
						CAST(:file_name AS TEXT),
						CAST(:time_from_utc AS BIGINT),
						CAST(:created_utc AS BIGINT)
					);		
				)"));

		if (res == false)
		{
			logErr(query.lastError().text());
			return false;
		}

		query.bindValue(":hash", QVariant::fromValue<qlonglong>(static_cast<qlonglong>(hash)));
		query.bindValue(":file_name", fileName);
		query.bindValue(":time_from_utc", timeFromUtc);
		query.bindValue(":created_utc", createdUtc);

		res = m_db->execQuery(query);

		if (!res)
		{
			return false;
		}

		if (!query.next())
		{
			logErr("fn_create_active_arch_file() returned no data");
			return false;
		}

		if (!afi->fromQuery(query))
		{
			logErr("Failed to parse arch_file_info");
			return false;
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
		result &= m_db->loadAndExecuteScript("Types/arch_file_info.sql");

		return result;
	}

	bool Db::functionsCreate()
	{
		TEST_PTR_RETURN_FALSE(m_db);

		bool result = true;

		result &= m_db->loadAndExecuteScript("Functions/fn_register_signals.sql");
		result &= m_db->loadAndExecuteScript("Functions/fn_get_registered_signals.sql");
		result &= m_db->loadAndExecuteScript("Functions/fn_delete_signals_by_hash.sql");
		result &= m_db->loadAndExecuteScript("Functions/fn_get_active_arch_files.sql");
		result &= m_db->loadAndExecuteScript("Functions/fn_create_active_arch_file.sql");

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

	QString Db::makeDatabaseName(const QString& projectID, const QString& appDataSrvID) const
	{ 
		return QString("u7arch_%1_%2").arg(sanitizeID(projectID)).arg(sanitizeID(appDataSrvID)).toLower();
	}
} // namespace ArchV3