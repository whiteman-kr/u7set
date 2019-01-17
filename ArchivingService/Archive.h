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

class ArchRequest;
class ArchWriterThread;
class ArchFile;

class ArchRequestParam
{
public:
	ArchRequestParam(quint32 requestID, E::TimeType timeType, qint64 startTime, qint64 endTime, const QVector<Hash>& signalHashes);

	quint32 requestID() const { return m_requestID; }

	E::TimeType timeType() const { return m_timeType; }
	void setTimeType(E::TimeType timeType) { m_timeType = timeType; }

	qint64 startTime() const { return m_startTime; }
	void setStartTime(qint64 startTime) { m_startTime = startTime; }

	qint64 endTime() const { return m_endTime; }
	void setEndTime(qint64 endTime) { m_endTime = endTime; }

	const QVector<Hash>& signalHashes() const { return m_signalHashes; }

	void expandTimes(qint64 expandTime);

private:
	quint32 m_requestID = 0;

	E::TimeType m_timeType = E::TimeType::System;

	qint64 m_startTime = 0;
	qint64 m_endTime = 0;

	QVector<Hash> m_signalHashes;
};

enum class ArchFindResult
{
	NotFound,
	Found,

	SearchError
};

class Archive
{
public:
	static const int TIME_1S = 1000;								// 1000 millisecond
	static const int TIME_TO_EXPAND_REQUEST = 31 * TIME_1S;			// 31 seconds

public:
	Archive(const QString& projectID,
			const QString& equipmentID,
			const QString& archDir,
			const Proto::ArchSignals& protoArchSignals,
			CircularLoggerShared logger);
	~Archive();

	void start();
	void stop();

	std::shared_ptr<ArchRequest> startNewRequest(E::TimeType timeType, qint64 sartTime, qint64 endTime, const QVector<Hash>& signalHashes);
	void finalizeRequest(quint32 requestID);

	bool isWorkable() const { return m_isWorkable; }

	QString getSignalID(Hash signalHash);

	bool canReadWriteSignal(Hash signalHash);
	void setCanReadWriteSignal(Hash signalHash, bool canWrite);

	void setSignalInitialized(Hash signalHash, bool initilaized);

	bool isSignalExists(Hash signalHash) const { return m_archFiles.contains(signalHash); }

	int getFilesCount() const { return m_archFiles.count(); }

	void getArchSignalStatus(Hash signalHash, bool* canReadWrite, bool* isInitialized, bool* isAnalog);

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

	bool shutdown();

	// flushing controlling functions (public)

	bool flushImmediately(ArchFile* archFile);
	bool waitingForImmediatelyFlushing(Hash signalHash, int waitTimeoutSeconds);

	ArchFile* getNextFileForFlushing(bool* flushAnyway);							// will be called from FileArchWriter

	//

	ArchFindResult findData(const ArchRequestParam& param);

	ArchFile* getArchFile(Hash signalHash) { return m_archFiles.value(signalHash, nullptr); }

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

	void clear();

private:
	QString m_projectID;
	QString m_equipmentID;

	CircularLoggerShared m_log;

	//

	static std::atomic<quint32> m_nextRequestID;

	bool m_isWorkable = false;

	//

	ArchWriterThread* m_archWriterThread = nullptr;

	//

	QHash<Hash, ArchFile*> m_archFiles;

	qint64 m_archID = 0;

	// File Archive members

	QString m_archDir;
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

	//

//	friend class ArchFile;
	friend class ArchRequest;
};
