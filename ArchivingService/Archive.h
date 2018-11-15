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
#include "ArchFile.h"

class Archive;

class ArchSignal
{
public:
	ArchSignal(Archive* archive, const Proto::ArchSignal& protoArchSignal);

private:
	Hash hash = 0;
	QString appSignalID;
	bool isAnalog = false;

	//

	bool isInitialized = false;
	bool canReadWrite = false;

	SimpleAppSignalState lastState;

	ArchFile archFile;

	//

	friend class ArchFile;
	friend class Archive;
};

class Archive
{
public:
	static const int TIME_1S = 1000;								// 1000 millisecond
	static const int TIME_TO_EXPAND_REQUEST = 31 * TIME_1S;			// 31 seconds

	static const char* FIELD_PLANT_TIME;
	static const char* FIELD_SYSTEM_TIME;
	static const char* FIELD_ARCH_ID;
	static const char* FIELD_VALUE;
	static const char* FIELD_FLAGS;

	enum DbType
	{
		Postgres,
		WriteArchive,
		ReadArchive
	};

public:
	Archive(const QString& projectID,
			const QString& equipmentID,
			const QString& archDir,
			const HostAddressPort& dbHost,
			CircularLoggerShared logger);
	~Archive();

	bool openDatabase(DbType dbType, QSqlDatabase& destDb);

	void initArchSignals(const Proto::ArchSignals& archSignals);
	QString getSignalID(Hash signalHash);

	bool canReadWriteSignal(Hash signalHash);
	void setCanReadWriteSignal(Hash signalHash, bool canWrite);

	void setSignalInitialized(Hash signalHash, bool initilaized);

	bool isSignalExists(Hash signalHash) const { return m_archSignals.contains(signalHash); }

	void getArchSignalStatus(Hash signalHash, bool* canReadWrite, bool* isInitialized, bool* isAnalog);

	void getSignalsHashes(QVector<Hash>* hashes);

	QString postgresDatabaseName();
	QString archiveDatabaseName();
	static QString getTableName(Hash signalHash);

//	const QHash<Hash, Archive::Signal>& archSignals() const { return m_archSignals; }

	void appendExistingTable(const QString& tableName);
	bool tableIsExists(const QString& tableName);

	static QString timeTypeStr(E::TimeType timeType);

	static qint64 localTimeOffsetFromUtc();

	static QString getCmpField(E::TimeType timeType);

	QString archDir() const { return m_archDir; }
	QString projectID() const { return m_projectID; }
	QString equipmentID() const { return m_equipmentID; }

	QString archFullPath() const { return m_archFullPath; }

	void saveState(const SimpleAppSignalState& state);
	void addEmergencyFile(ArchFile* file);
	ArchFile* getNextEmergencyFile();

	bool flushImmediately(Hash signalHash);
	bool waitingForImmediatelyFlushing(Hash signalHash, int waitTimeoutSeconds);

	ArchFile* getNextRequredImediatelyFlushing();

	ArchFile* getNextRegularFile();

	Queue<SimpleAppSignalState>& dbSaveStatesQueue() { return m_dbSaveStatesQueue; }

	bool checkAndCreateArchiveDirs();
	bool archDirIsWritableChecking();
	bool createGroupDirs();

	bool shutdown();

private:
	void clear();

	QSqlDatabase getDatabase(DbType dbType);
	void removeDatabases();

	ArchFile* getArchFile(Hash signalHash);

private:
	static const char* ARCH_DB_PREFIX;
	static const char* LONG_TERM_TABLE_PREFIX;
	static const char* SHORT_TERM_TABLE_PREFIX;

private:
	QString m_projectID;
	QString m_equipmentID;

	CircularLoggerShared m_log;

	QHash<Hash, ArchSignal*> m_archSignals;

	Queue<SimpleAppSignalState> m_dbSaveStatesQueue;

	qint64 m_archID = 0;

	// Db Archive members

	HostAddressPort m_dbHost;
	QString m_dbUser;
	QString m_dbPassword;

	QHash<QString, QString> m_existingTables;
	QMutex m_dbMutex;

	// File Archive members

	QString m_archDir;
	QString m_archFullPath;

	QVector<ArchFile*> m_archFiles;

	QMutex m_emergencyFilesMutex;
	QList<ArchFile*> m_emergencyFilesQueue;
	QHash<ArchFile*, bool> m_emergencyFilesInQueue;

	QMutex m_immedaitelyFlushingMutex;
	QList<ArchFile*> m_requiredImmediatelyFlushing;			// files required immediately flushing (for example before reading)

	QMutex m_regularFilesMutex;
	int m_regularFileIndex = 0;

	friend class ArchFile;
};

typedef std::shared_ptr<Archive> ArchiveShared;
