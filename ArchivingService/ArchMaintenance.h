#pragma once

#include "..\lib\SimpleThread.h"

class Archive;

class ArchMaintenanceThread : public RunOverrideThread
{
public:
	ArchMaintenanceThread(Archive& archive);

	void run() override;

private:
	Archive& m_archive;
};
