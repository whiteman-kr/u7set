#pragma once

#include <QHash>
#include <QMutex>
#include <QSqlDatabase>

#include "../lib/Hash.h"
#include "../lib/HostAddressPort.h"
#include "../lib/TimeStamp.h"
#include "../lib/CircularLogger.h"
#include "../lib/Types.h"
#include "../lib/Queue.h"
#include "../lib/AppSignal.h"
#include "ArchRequest.h"
#include "ArchMaintenance.h"

class ArchWriterThread;

class Archive
{
public:
	static const int TIME_1S = 1000;								// 1000 millisecond
	static const int TIME_TO_EXPAND_REQUEST = 31 * TIME_1S;			// 31 seconds

	static const qint64 PARTITION_PERIOD_MS = 24 * 60 * 60 * 1000;	// day
	//static const qint64 PARTITION_PERIOD_MS = 60 * 60 * 1000;			// hour
	// static const qint64 PARTITION_PERIOD_MS = 5 * 60 * 1000;			// 5 minutes

	static QString formatTime(qint64 time);

public:
	Archive(const QString& projectID,
			const QString& equipmentID,
			const QString& archDir,
			const Proto::ArchSignals& protoArchSignals,
			int shortTermPeriod,
			int longTermPeriod,
			int maintenanceDelayMinutes,
			CircularLoggerShared logger);
	~Archive();

	void start();
	void stop();

	qint64 msShortTermPeriod() const { return m_msShortTermPeriod; }
	qint64 msLongTermPeriod() const { return m_msLongTermPeriod; }

	int maintenanceDelayMinutes() const { return m_maintenanceDelayMinutes; }

	void writeArchFilesInfoFile(const QVector<QVector<ArchFile*>>& archFilesGroups);

	std::shared_ptr<ArchRequest> startNewRequest(E::TimeType timeType,
												 qint64 sartTime,
												 qint64 endTime,
												 const QVector<Hash>& signalHashes,
												 std::shared_ptr<Network::GetAppSignalStatesFromArchiveNextReply> getNextReply);
	void finalizeRequest(quint32 requestID);

	bool isWorkable() const { return m_isWorkable; }

	QString getSignalID(Hash signalHash);

	bool isSignalExists(Hash signalHash) const { return m_archFiles.contains(signalHash); }

	int getFilesCount() const { return m_archFiles.count(); }

	void getSignalsHashes(QVector<Hash>* hashes);

	static QString timeTypeStr(E::TimeType timeType);
	static qint64 localTimeOffsetFromUtc();

	QString archDir() const { return m_archDir; }
	QString projectID() const { return m_projectID; }
	QString equipmentID() const { return m_equipmentID; }

	QString archFullPath() const { return m_archFullPath; }

	void saveState(const SimpleAppSignalState& state);

	bool checkAndCreateArchiveDirs();
	bool archDirIsWritableChecking();
	bool createGroupDirs();

	bool shutdown(ArchFileRecord* buffer, int bufferSize, const QThread* thread);

	// flushing controlling functions (public)

	bool flushImmediately(ArchFile* archFile);
	bool waitingForImmediatelyFlushing(Hash signalHash, int waitTimeoutSeconds);

	ArchFile* getNextFileForFlushing(bool* flushAnyway);							// will be called from FileArchWriter

	//

	ArchFile* getArchFile(Hash signalHash) { return m_archFiles.value(signalHash, nullptr); }

	bool isMaintenanceRequired() { return m_isMaintenanceRequired.load(); }
	void maintenanceIsStarted();

	qint64 getCurrentPartition();

private:
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
	QString m_projectID;
	QString m_equipmentID;
	QString m_archDir;
	qint64 m_msShortTermPeriod = 0;
	qint64 m_msLongTermPeriod = 0;
	int m_maintenanceDelayMinutes = 0;
	CircularLoggerShared m_log;

	//

	static std::atomic<quint32> m_nextRequestID;

	bool m_isWorkable = false;

	//

	std::atomic<qint64> m_currentPartition = { -1 };

	ArchWriterThread* m_archWriterThread = nullptr;

	std::atomic<bool> m_isMaintenanceRequired = { true };
	int m_maintenanceFileIndex = -1;
	ArchMaintenanceThread* m_archMaintenanceThread = nullptr;

	//

	QHash<Hash, ArchFile*> m_archFiles;

	qint64 m_archID = 0;

	// File Archive members

	QString m_archFullPath;

	QVector<ArchFile*> m_archFilesArray;

	QMutex m_immedaitelyFlushingMutex;
	QList<ArchFile*> m_requiredImmediatelyFlushing;			// files required immediately flushing (for example before reading)
	QHash<ArchFile*, bool> m_alreadyInRequiredImmediatelyFlushing;

	QMutex m_emergencyFilesMutex;
	QList<ArchFile*> m_emergencyFiles;
	QHash<ArchFile*, bool> m_alreadyInEmergencyFiles;

	QList<ArchFile*> m_regularFilesQueue;

	//

	QMutex m_requestsMutex;
	QHash<quint32, std::shared_ptr<ArchRequest>> m_requests;

	friend class ArchRequest;
};
