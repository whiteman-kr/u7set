#pragma once

#include "..\lib\SimpleThread.h"
#include "..\lib\CircularLogger.h"

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
};
