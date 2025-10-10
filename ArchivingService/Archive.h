#pragma once

#include "../OnlineLib/CircularLogger.h"
#include "ArchMaintenance.h"
#include "ArchRequest.h"
#include <QHash>
#include <QMutex>
#include <QSqlDatabase>

class ArchWriterThread;

class Archive
{
public:
	static const int TIME_1S = 1000;								// 1000 millisecond
	static const int TIME_TO_EXPAND_REQUEST = 31 * TIME_1S;			// 31 seconds

	static const qint64 PARTITION_PERIOD_MS = 24 * 60 * 60 * 1000;	// day
	//static const qint64 PARTITION_PERIOD_MS = 60 * 60 * 1000;		// hour
	//static const qint64 PARTITION_PERIOD_MS = 1 * 60 * 1000;		// 1 minutes

	static const int MIN_SHORT_TERM_PERIOD_DAYS = 2;

	static const int MIN_QUEUE_SIZE_FOR_FLUSHING = 1;
	static const int DEFAULT_QUEUE_SIZE_FOR_FLUSHING = 3;
	static const int MAX_QUEUE_SIZE_FOR_FLUSHING = 10;

	static const int MIN_MAINTENANCE_DELAY_MINUTES = 0;
	static const int DEFAULT_MAINTENANCE_DELAY_MINUTES = 5;
	static const int MAX_MAINTENANCE_DELAY_MINUTES = 300;

	static QString formatTime(qint64 time);

public:
	Archive(const QString& projectID,					// Read/write archive constructor
			const QString& equipmentID,
			const QString& archDir,
			QByteArray& archFileInfoData,
			int shortTermPeriod,
			int longTermPeriod,
			int maintenanceDelayMinutes,
			int minQueueSizeForFlushing,
			CircularLoggerShared logger);

	Archive(const QString& projectID,					// Read only archive constructor
			const QString& equipmentID,
			const QString& readOnlyArchFullPath,
			QByteArray& archFileInfoData,
			CircularLoggerShared logger);

	~Archive();

	void start();
	void stop();

	qint64 msShortTermPeriod() const { return m_msShortTermPeriod; }
	qint64 msLongTermPeriod() const { return m_msLongTermPeriod; }
	int minQueueSizeForFlushing() const { return m_minQueueSizeForFlushing; }
	int maintenanceDelayMinutes() const { return m_maintenanceDelayMinutes; }

	QString projectID() const { return m_projectID; }
	QString equipmentID() const { return m_equipmentID; }
	QString archFullPath() const { return m_archFullPath; }

	bool isWorkable() const { return m_isWorkable; }
	bool isReadOnly() const { return m_readOnlyArchive; }

	std::shared_ptr<ArchRequest> startNewRequest(E::TimeType timeType,
												 qint64 sartTime,
												 qint64 endTime,
												 const QVector<Hash>& signalHashes,
												 std::shared_ptr<Network::GetAppSignalStatesFromArchiveNextReply> getNextReply);
	void finalizeRequest(quint32 requestID);

	QString getSignalID(Hash signalHash);
	bool isSignalExists(Hash signalHash) const { return m_archFiles.contains(signalHash); }

	int getFilesCount() const { return TO_INT(m_archFiles.size()); }
	void getSignalsHashes(QVector<Hash>* hashes);

	void saveState(const SimpleAppSignalState& state);

	void updateSavedDataSizePerMin();
	qint64 getSavedDataSizePerMin();
	qint64 getDiskFreeSpace() const;

	bool shutdown(ArchFileRecord* buffer, int bufferSize);

	// flushing controlling functions (public)

	bool flushImmediately(ArchFile* archFile);
	bool waitingForImmediatelyFlushing(Hash signalHash, int waitTimeoutSeconds);

	ArchFile* getNextFileForFlushing(bool* flushAnyway);							// will be called from FileArchWriter
	ArchFile* getArchFile(Hash signalHash);
	ArchFile* getArchFileByIndex(int index);

	bool isMaintenanceRequired() { return m_isMaintenanceRequired.load(); }
	void maintenanceIsStarted();

	qint64 getCurrentPartition();

	static QString timeTypeStr(E::TimeType timeType);

	void getRecordsPerMin(std::vector<std::pair<int, int>>* recordsPerMin, int count);

private:
	bool loadArchInfoFile();
	bool initArchFiles();
	bool checkAndCreateArchiveDirs();
	bool archDirIsWritableChecking();
	bool createGroupDirs();

	bool saveArchInfoProtoFile() const;

	void writeArchFilesInfoFile(const std::vector<std::vector<ArchFile *>>& archFilesGroups);

	void updateDiskFreeSpace();

	quint32 getNewRequestID();

	// flushing controlling functions (private)

	ArchFile* getNextRequiredImediatelyFlushing();
	void removeFromRequiredImmediatelyFlushing(ArchFile* file);

	void appendEmergencyFile(ArchFile* file);
	ArchFile* getNextEmergencyFile();
	void removeFromEmergencyFiles(ArchFile* file);

	ArchFile* getNextRegularFile();
	void pushBackInRegularFilesQueue(ArchFile* file);

	//

	void stopAllRequests();
	void stopMaintenanceThread();
	void stopWriteThread();

	void clear();

private:
	bool m_readOnlyArchive = true;
	QByteArray* m_archInfoFileData = nullptr;

	QString m_projectID;
	QString m_equipmentID;
	QString m_archDir;
	qint64 m_msShortTermPeriod = 0;
	qint64 m_msLongTermPeriod = 0;
	int m_maintenanceDelayMinutes = 0;
	int m_minQueueSizeForFlushing = 0;
	CircularLoggerShared m_log;

	bool m_isWorkable = false;

	std::atomic<qint64> m_savedDataSizeCounter = { 0 };
	std::atomic<qint64> m_savedDataSizePerMin = { 0 };
	std::atomic<qint64> m_diskFreeSpace = { 0 };

	//

	std::atomic<qint64> m_currentPartition = { -1 };
	ArchWriterThread* m_archWriterThread = nullptr;

	//

	std::atomic<bool> m_isMaintenanceRequired = { true };
	ArchMaintenanceThread* m_archMaintenanceThread = nullptr;

	//

	std::map<Hash, ArchFile*> m_archFiles;
	std::vector<ArchFile*> m_archFilesArray;

	QString m_archFullPath;

	QMutex m_immedaitelyFlushingMutex;
	QList<ArchFile*> m_requiredImmediatelyFlushing;			// files required immediately flushing (for example before reading)
	QHash<ArchFile*, bool> m_alreadyInRequiredImmediatelyFlushing;

	QMutex m_emergencyFilesMutex;
	QList<ArchFile*> m_emergencyFiles;
	QHash<ArchFile*, bool> m_alreadyInEmergencyFiles;

	QList<ArchFile*> m_regularFilesQueue;

	//

	static std::atomic<quint32> m_nextRequestID;
	QMutex m_requestsMutex;
	QHash<quint32, std::shared_ptr<ArchRequest>> m_requests;

	friend class ArchRequest;
};
