#pragma once

#include "../lib/AppSignal.h"
#include "../lib/CircularLogger.h"
#include "../lib/WUtils.h"

#include "Archive.h"

class FileArchWriter;

class ArchFile
{
#pragma pack(push, 1)

	struct SignalState
	{
		// light version of AppSignalState to use in queues and other AppDataService data structs
		//
		struct
		{
			qint64 archID;
			qint64 system;
			qint64 plant;

			AppSignalStateFlags flags;
			double value = 0;
		} state;

		quint32 CRC32;
	};

#pragma pack(pop)

public:
	ArchFile();

	bool init(const FileArchWriter* writer, const QString& signalID, Hash hash, int initialQueueSize);

	bool pushState(qint64 archID, const SimpleAppSignalState& state);

private:
	const FileArchWriter* m_archWriter = nullptr;
	QString m_signalID;
	Hash m_hash = 0;

	//

	bool m_pathIsExists = false;
	bool m_fileIsOpened = false;
	int m_currentPartition = -1;

	LockFreeQueue<SignalState>* m_queue = nullptr;
};


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
	bool createGroupDirs();
	bool createArchFiles();

	void shutdown();

private:
	ArchiveShared m_archive;
	Queue<SimpleAppSignalState>& m_saveStatesQueue;
	CircularLoggerShared m_log;

	QString m_archFullPath;
	ArchFile* m_archFiles = nullptr;
	QHash<Hash, ArchFile*> m_hashArchFiles;

	const int DISCRETES_INITIAL_QUEUE_SIZE = 20;
	const int ANALOGS_INITIAL_QUEUE_SIZE = 200;
};

