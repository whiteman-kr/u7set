#pragma once

#include "../lib/AppSignal.h"
#include "../lib/CircularLogger.h"
#include "../lib/WUtils.h"
#include "Archive.h"

class ArchWriterThread : public RunOverrideThread
{
public:
	ArchWriterThread(Archive* archive,
						CircularLoggerShared logger);
private:
	void run() override;

private:
	Archive* m_archive = nullptr;
	CircularLoggerShared m_log;

	//

	QThread* m_thisThread = nullptr;

	qint64 m_totalFlushedStatesCount = 0;
};
