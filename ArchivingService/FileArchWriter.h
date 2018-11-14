#pragma once

#include "../lib/AppSignal.h"
#include "../lib/CircularLogger.h"
#include "../lib/WUtils.h"

#include "Archive.h"
#include "ArchFile.h"

struct ArchRequestParam;

class FileArchWriter : public RunOverrideThread
{
public:
	FileArchWriter(ArchiveShared archive,
				   Queue<SimpleAppSignalState>& saveStatesQueue,
				   CircularLoggerShared logger);

	QString archFullPath() const { return m_archFullPath; }
	bool flushFileBeforeReading(Hash signalHash, QString* filePath);

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

	bool initFiles();
	bool archDirIsWritableChecking();
	bool createGroupDirs();
	bool createArchFiles();

	bool processSaveStatesQueue();
	void updateCurrentPartition();
	bool writeMinuteCheckpoint(qint64 minuteSystemTime);
	void runArchiveMaintenance();
	bool writeEmergencyFiles();
	bool writeRegularFiles();
	bool archiveMaintenance();

	void shutdown();

	void addEmergencyFile(ArchFile* file);
	ArchFile* getNextEmergencyFile();


//	void takeEmergencyFilesOwnership(const QThread* newOwner);
//	void releaseEmergencyFilesOwnership(const QThread* currentOwner);

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

	qint64 m_curPartition = -1;
	qint64 m_archID = 0;

	bool m_archMaintenanceIsRunning = false;

	std::atomic<const QThread*> m_emergencyFilesOwner = { nullptr };
	QList<ArchFile*> m_emergencyFilesQueue;
	QHash<ArchFile*, bool> m_emergencyFilesInQueue;

	qint64 m_totalFlushedStatesCount = 0;
};
