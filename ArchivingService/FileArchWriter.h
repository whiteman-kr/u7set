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
			double value;
		} state;

		quint16 crc16;
	};

#pragma pack(pop)

public:
	ArchFile();
	~ArchFile();

	bool init(const FileArchWriter* writer, const QString& signalID, Hash hash, bool isAnalogSignal);

	bool pushState(qint64 archID, const SimpleAppSignalState& state);

	bool flush(qint64 curPartition);

	bool queueIsEmpty() const { return m_queue->isEmpty(); }

	bool isEmergency() const;

	void shutdown(qint64 curPartition);

private:
	bool writeFile(qint64 partition, SignalState* buffer, int statesCount);
	void closeFile();

private:
	const FileArchWriter* m_archWriter = nullptr;
	QString m_signalID;
	Hash m_hash = 0;

	QString m_path;

	//

	QFile m_file;
	bool m_pathIsExists = false;
	bool m_fileIsOpened = false;
	bool m_fileIsAligned = false;
	qint64 m_prevPartition = -1;

	FastQueue<SignalState>* m_queue = nullptr;

	SimpleMutex m_flushMutex;

	const double QUEUE_EMERGENCY_LIMIT = 0.7;		// 70%
	const double QUEUE_EXPAND_LIMIT = 0.9;			// 90%

	static const int QUEUE_MIN_SIZE = 20;
	static const int QUEUE_MAX_SIZE = 1280;		// 20 * 2^6

	static SignalState m_buffer[QUEUE_MAX_SIZE];
};


class FileArchWriter : public RunOverrideThread
{
public:
	FileArchWriter(ArchiveShared archive,
				   Queue<SimpleAppSignalState>& saveStatesQueue,
				   CircularLoggerShared logger);

	QString archFullPath() const { return m_archFullPath; }

private:
	void run() override;

	bool initFiles();
	bool archDirIsWritableChecking();
	bool createGroupDirs();
	bool createArchFiles();

	bool processSaveStatesQueue();
	void updateCurrentPartition();
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
};

