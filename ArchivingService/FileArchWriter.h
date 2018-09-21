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

	void flush();

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

	bool pushState(const SimpleAppSignalState& state, const QThread* thread);


private:
	void run() override;

	bool initFiles();
	bool archDirIsWritableChecking();
	bool createGroupDirs();
	bool createArchFiles();

	bool writeEmergencyFiles();
	bool writeRegularFiles();

	void shutdown();

	void addEmergencyFile(ArchFile* file, const QThread* thread);
	ArchFile* getNextEmergencyFile(const QThread* thread);

	void takeEmergencyFilesOwnership(const QThread* newOwner);
	void releaseEmergencyFilesOwnership(const QThread* currentOwner);


private:
	ArchiveShared m_archive;
	Queue<SimpleAppSignalState>& m_saveStatesQueue;
	CircularLoggerShared m_log;
	QThread* m_thisThread = nullptr;

	QString m_archFullPath;
	ArchFile* m_archFiles = nullptr;
	int m_archFilesCount = 0;
	int m_regularArchFileIndex = 0;
	QHash<Hash, ArchFile*> m_hashArchFiles;

	qint64 m_archID = 0;

	std::atomic<const QThread*> m_emergencyFilesOwner = { nullptr };
	QList<ArchFile*> m_emergencyFilesQueue;
	QHash<ArchFile*, bool> m_emergencyFilesInQueue;

	const int DISCRETES_INITIAL_QUEUE_SIZE = 20;
	const int ANALOGS_INITIAL_QUEUE_SIZE = 200;
};

