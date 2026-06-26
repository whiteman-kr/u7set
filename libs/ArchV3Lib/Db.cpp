#ifndef ARCH_V3_LIB_DOMAIN
	#error Do not include this file in the project! Link ArchV3Lib instead.
#endif

#include <ArchV3Lib/Db.h>
#include <ArchV3Lib/Utils.h>

#include <CommonLib/ConstStrings.h>

namespace ArchV3
{
	Db::Db(	const QString& projectID, const QString& appDataSrvID, 
			const DbConnectionInfo& dbConnInfo,
			CircularLoggerShared logger, const QString& className) :
			LogWrapper(logger, className),
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
		if (m_postgresDb == nullptr)
		{
			m_postgresDb = std::make_unique<Postgres>(m_dbConnInfo, DbName::POSTGRES, getLog());

			if (m_postgresDb->open() == false)
			{
				m_postgresDb.reset();
				return false;
			}
		}

		QString dbName = makeDatabaseName(m_projectID, m_appDataSrvID);

		m_db = std::make_unique<Postgres>(m_dbConnInfo, dbName, getLog());
	}

	void Db::close() 
	{
		if (m_postgresDb != nullptr)
		{
			m_postgresDb->close();
			m_postgresDb.reset();
		}

		if (m_db != nullptr)
		{
			m_db->close();
			m_db.reset();
		}
	}

	QString Db::makeDatabaseName(const QString& projectID, const QString& appDataSrvID) const
	{ 
		return QString("u7arch_%1_%2").arg(sanitizeID(projectID)).arg(sanitizeID(appDataSrvID)).toLower();
	}
} // namespace ArchV3