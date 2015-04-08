#include "ConfigurationBuilder.h"

#include "../../include/DbController.h"
#include "../../include/OutputLog.h"
#include "../../include/DeviceObject.h"

namespace Builder
{

	ConfigurationBuilder::ConfigurationBuilder(DbController* db, OutputLog* log,
		int changesetId, bool debug) :
		m_db(db),
		m_log(log),
		m_changesetId(changesetId),
		m_debug(debug)
	{
		assert(m_db);
		assert(m_log);

		return;
	}

	ConfigurationBuilder::~ConfigurationBuilder()
	{
	}

	bool ConfigurationBuilder::build()
	{


		return true;
	}

	DbController* ConfigurationBuilder::db()
	{
		return m_db;
	}

	OutputLog* ConfigurationBuilder::log() const
	{
		return m_log;
	}

	int ConfigurationBuilder::changesetId() const
	{
		return m_changesetId;
	}

	bool ConfigurationBuilder::debug() const
	{
		return m_debug;
	}

	bool ConfigurationBuilder::release() const
	{
		return !debug();
	}

}
