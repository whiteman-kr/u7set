#include "ArchMaintenance.h"
#include "Archive.h"

ArchMaintenanceThread::ArchMaintenanceThread(Archive& archive, CircularLoggerShared logger) :
	m_archive(archive),
	m_log(logger)
{
	archive.getSignalsHashes(&m_signalsHashes);
}

void ArchMaintenanceThread::run()
{
	m_timerStarted = false;

	for (;;)
	{
		if (m_timerStarted == true)
		{
			if (m_timerToStartMaintenance.hasExpired(m_archive.maintenanceDelayMinutes() * 60 * 1000) == true)
			{
				maintenance();
				m_timerStarted = false;
			}
		}
		else
		{
			if (m_archive.isMaintenanceRequired() == true)
			{
				m_timerToStartMaintenance.start();
				m_timerStarted = true;
			}
		}

		if (isQuitRequested() == true)
		{
			break;
		}

		msleep(500);
	}
}

void ArchMaintenanceThread::maintenance()
{
	m_archive.maintenanceIsStarted();			// m_isMaintenanceRequired flag clearing

	qint64 startTime = QDateTime::currentSecsSinceEpoch();

	DEBUG_LOG_MSG(m_log, QString("Archive maintenance is starting"));

	int deletedCount = 0;
	int packedCount = 0;
	int errorCount = 0;

	QVector<Hash> hashes = m_signalsHashes;
	QVector<Hash> retryHashes;

	int retryCount = 0;

	while(retryCount < 2)
	{
		retryHashes.clear();

		errorCount = 0;

		for(Hash signalHash : hashes)
		{
			if (isQuitRequested() == true)
			{
				DEBUG_LOG_WRN(m_log, QString("Maintenance is interrupted (quit requested)"));
				return;
			}

			ArchFile* archFile = m_archive.getArchFile(signalHash);

			if (archFile == nullptr)
			{
				assert(false);
				continue;
			}

			bool res = archFile->maintenance(m_archive.getCurrentPartition(),
								  m_archive.msShortTermPeriod(),
								  m_archive.msLongTermPeriod(),
								  &deletedCount,
								  &packedCount,
								  this);
			if (res == false)
			{
				errorCount++;

				retryHashes.append(signalHash);
			}
		}

		if (retryHashes.count() == 0)
		{
			break;
		}

		hashes = retryHashes;

		retryCount++;
	}

	if (retryHashes.count() > 0)
	{
		DEBUG_LOG_WRN(m_log, QString("Maintenance: %1 files is not processed due to permanent errors").arg(retryHashes.count()));
	}
	else
	{
		DEBUG_LOG_MSG(m_log, QString("Maintenance: %1 files is processed").arg(m_signalsHashes.count()));
	}

	DEBUG_LOG_MSG(m_log, QString("Archive maintenance is finished. Deleted %1. Packed %2. Errors %3. Time elapsed %4 s").
					arg(deletedCount).
					arg(packedCount).
					arg(errorCount).
					arg(QDateTime::currentSecsSinceEpoch() - startTime));
}
