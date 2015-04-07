#include "BuildResultWriter.h"


namespace Builder
{
	BuildResultWriter::BuildResultWriter(DbController *db, OutputLog *log, bool release, QObject *parent) :
		m_dbController(db),
		m_log(log),
		m_release(release),
		QObject(parent)
	{
		assert(m_log != nullptr);
	}

	bool BuildResultWriter::start()
	{
		//QString appDataDir = QDesktopServices()::
		return true;
	}

	bool BuildResultWriter::finish()
	{
		return true;
	}
}
