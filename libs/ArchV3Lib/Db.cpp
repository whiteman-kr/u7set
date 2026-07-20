#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <QLatin1StringView>

#include <ArchV3Lib/Db.h>
#include <ArchV3Lib/Utils.h>

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

	bool Db::registerSignals(const std::vector<ArchSignal>& archSignals)
	{
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

		result = deleteSignals(signalsToDelete);

		Q_ASSERT(false);

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

		return result;
	}

	bool Db::getRegisteredSignals(std::unordered_map<Hash, RegisteredSignalInfo>* registeredSignals) const
	{ 
		TEST_PTR_RETURN_FALSE(registeredSignals);	

		auto query = m_db->execQuery("SELECT * FROM fn_get_registered_signals()");

		if (!query)
		{
			return false;
		}

		static constexpr int COL_SIGNAL_ID = 0;
		static constexpr int COL_SIGNAL_TYPE = 1;
		static constexpr int COL_APP_SIGNAL_ID = 2;
		static constexpr int COL_HASH = 3;
		static constexpr int COL_BUCKET = 4;
		static constexpr int COL_CREATED_UTC = 5;

		registeredSignals->clear();
		registeredSignals->reserve(query->size());

		RegisteredSignalInfo rsi;

		while (query->next())
		{
			rsi.signalID = query->value(COL_SIGNAL_ID).toULongLong();
			rsi.signalType = static_cast<E::SignalType>(query->value(COL_SIGNAL_TYPE).toInt());
			rsi.appSignalID = query->value(COL_APP_SIGNAL_ID).toString();
			rsi.hash = static_cast<Hash>(query->value(COL_HASH).toULongLong());
			rsi.bucket = static_cast<quint8>(query->value(COL_BUCKET).toInt());

			registeredSignals->emplace(rsi.hash, rsi);
		}

		return true;
	}

	bool Db::deleteSignals(const std::vector<QString>& ids) const
	{
		QStringList values;

		for (const QString& id : ids)
		{
			values << QString::number(calcHash(id));
		}

		const QString sql = QString("SELECT fn_delete_signals_by_hash(ARRAY[%1]::BIGINT[])").arg(values.join(","));

		bool res = m_db->execSql(sql);

		return res;
	}

	QString Db::makeDatabaseName(const QString& projectID, const QString& appDataSrvID) const
	{ 
		return QString("u7arch_%1_%2").arg(sanitizeID(projectID)).arg(sanitizeID(appDataSrvID)).toLower();
	}
} // namespace ArchV3