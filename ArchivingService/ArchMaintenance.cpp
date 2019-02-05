#include "ArchMaintenance.h"
#include "Archive.h"

ArchMaintenanceThread::ArchMaintenanceThread(Archive& archive, CircularLoggerShared logger) :
	m_archive(archive),
	m_log(logger)
{
}

void ArchMaintenanceThread::run()
{
	while(isQuitRequested() == false)
	{
		if (m_archive.isMaintenanceRequired() == true)
		{
			maintenance();
		}
		else
		{
			msleep(500);
		}
	}
}

void ArchMaintenanceThread::maintenance()
{
	qint64 startTime = QDateTime::currentSecsSinceEpoch();

	DEBUG_LOG_MSG(m_log, QString("Archive maintenance is starting"));

	int deletedCount = 0;
	int packedCount = 0;

	m_archive.startMaintenance();

	while(isQuitRequested() == false)
	{
		bool res = m_archive.continueMaintenance(&deletedCount, &packedCount);

		if (res == false)
		{
			DEBUG_LOG_MSG(m_log, QString("Archive maintenance is finished. Deleted %1. Packed %2. Time elapsed %3 s").
							arg(deletedCount).
							arg(packedCount).
							arg(QDateTime::currentSecsSinceEpoch() - startTime));
			return;
		}
	}

}
