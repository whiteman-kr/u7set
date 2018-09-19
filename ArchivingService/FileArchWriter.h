#pragma once

#include "../lib/AppSignal.h"
#include "../lib/CircularLogger.h"

#include "Archive.h"

class FileArchWriter : public RunOverrideThread
{
public:
	FileArchWriter(ArchiveShared archive,
				   Queue<SimpleAppSignalState>& saveStatesQueue,
				   CircularLoggerShared logger);

private:
	void run() override;

	bool initFiles();
	bool archDirIsWritableChecking();

private:
	ArchiveShared m_archive;
	Queue<SimpleAppSignalState>& m_saveStatesQueue;
	CircularLoggerShared m_log;

	QString m_archFullPath;
};

