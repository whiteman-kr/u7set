#pragma once

#include "../UtilsLib/SimpleThread.h"
#include "../OnlineLib/CircularLogger.h"
#include "../CommonLib/Hash.h"
#include <QVector>
#include <QElapsedTimer>

class Archive;

class ArchMaintenanceThread : public RunOverrideThread
{
public:
	ArchMaintenanceThread(Archive& archive, CircularLoggerShared logger);

	void run() override;

private:
	void maintenance();

private:
	Archive& m_archive;
	CircularLoggerShared m_log;

	QElapsedTimer m_timerToStartMaintenance;
	bool m_timerStarted = true;

	QVector<Hash> m_signalsHashes;
};
