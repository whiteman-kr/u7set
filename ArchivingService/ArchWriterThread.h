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

	void printStatistics(qint64 currentPartition);
	void sleepMs(int ms);

private:
	Archive* m_archive = nullptr;
	CircularLoggerShared m_log;

	//

	QThread* m_thisThread = nullptr;

	//

	qint64 m_totalFlushedStatesCount = 0;
	qint64 m_prevFlushedStatesCount = 0;
	qint64 m_prevPartition = -1;
	qint64 m_sleepTime = 0;
	QElapsedTimer m_statisticsTimer;
};
