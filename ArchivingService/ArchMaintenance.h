#pragma once

#include "..\lib\SimpleThread.h"
#include "..\lib\CircularLogger.h"
#include "..\lib\Hash.h"
#include <QVector>

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

	QVector<Hash> m_signalsHashes;
};
