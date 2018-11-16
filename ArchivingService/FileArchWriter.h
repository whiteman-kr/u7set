#pragma once

#include "../lib/AppSignal.h"
#include "../lib/CircularLogger.h"
#include "../lib/WUtils.h"

#include "Archive.h"
#include "ArchFile.h"

class FileArchWriter : public RunOverrideThread
{
public:
	FileArchWriter(ArchiveShared archive,
				   CircularLoggerShared logger);

private:

#pragma pack(push, 1)

	struct MinuteCheckpoint
	{
		struct
		{
			qint64 minuteSystemTime = 0;
			qint64 archiveID = 0;
		} checkpoint;

		quint16 crc16 = 0;
	};

#pragma pack(pop)

private:
	void run() override;

	void updateCurrentPartition();
//	bool writeMinuteCheckpoint(qint64 minuteSystemTime);

private:
	ArchiveShared m_archive;
	CircularLoggerShared m_log;
	QThread* m_thisThread = nullptr;

	qint64 m_curPartition = -1;

	qint64 m_totalFlushedStatesCount = 0;
};
