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
			m_db.reset();
			return false;
		}

		if (schemaCheckAndCreate() == false)
		{
			m_db->close();
			m_db.reset();
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

		return result;
	}

	QString Db::makeDatabaseName(const QString& projectID, const QString& appDataSrvID) const
	{ 
		return QString("u7arch_%1_%2").arg(sanitizeID(projectID)).arg(sanitizeID(appDataSrvID)).toLower();
	}
} // namespace ArchV3