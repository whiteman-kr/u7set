#pragma once

#include <QSqlQuery>
#include <QMutex>

#include "DbStruct.h"
#include "../lib/Signal.h"

#define AUTO_COMPLETE std::shared_ptr<int*> progressCompleted(nullptr, [this](void*) { this->m_progress->setCompleted(true); } );

namespace Hardware
{
    class DeviceObject;
}

class DbProgress;


class DbWorker : public QObject
{
	Q_OBJECT

private:
	DbWorker();

public:
    DbWorker(DbProgress* progress);

	static void getSignalData(QSqlQuery& q, Signal &s);
	static QString getSignalDataStr(const Signal& s);

protected:
	QString postgresConnectionName() const;
    QString projectConnectionName() const;

    bool checkDatabaseFeatures(QSqlDatabase db);

	void emitError(QSqlDatabase db, const QSqlError& err, bool addLogRecord = true);
	void emitError(QSqlDatabase db, const QString& err, bool addLogRecord = true);

    //
    // Public signals
    //
signals:

    //
    // Public methods
    //
public:
    static int databaseVersion();
    bool isProjectOpened() const;

    int rootFileId() const;
    int afblFileId() const;
	int schemasFileId() const;
	int ufblFileId() const;
    int alFileId() const;
    int hcFileId() const;
    int hpFileId() const;
    int mvsFileId() const;
	int tvsFileId() const;
    int dvsFileId() const;
    int mcFileId() const;
	int connectionsFileId() const;
	int busTypesFileId() const;
	int etcFileId() const;

    std::vector<DbFileInfo> systemFiles() const;

	static QString toSqlStr(const QString& str);
	static QString toSqlBoolean(bool value);
	static QString toSqlByteaStr(const QByteArray& binData);

    //
    // Operations
    //
public slots:
    // Project Management
    //
    void slot_getProjectList(std::vector<DbProject>* out);
    void slot_createProject(QString projectName, QString administratorPassword);
    void slot_openProject(QString projectName, QString username, QString password);
    void slot_closeProject();
	void slot_cloneProject(QString projectName, QString password, QString newProjectName);
    void slot_deleteProject(QString projectName, QString password, bool doNotBackup);
    void slot_upgradeProject(QString projectName, QString password, bool doNotBackup);

    void slot_setProjectProperty(QString propertyName, QString propertyValue);
    void slot_getProjectProperty(QString propertyName, QString* out);
    void getProjectProperty_worker(QString propertyName, QString* out);

    // User management
    //
    void slot_createUser(DbUser user);
    void slot_updateUser(DbUser user);
    void slot_getUserList(std::vector<DbUser>* out);

    // File management
    //
	void slot_isFileExists(QString fileName, int parentId, int* fileId);

    void slot_getFileList(std::vector<DbFileInfo>* files, int parentId, QString filter, bool removeDeleted);
    void getFileList_worker(std::vector<DbFileInfo>* files, int parentId, QString filter, bool removeDeleted);

	void slot_getFileListTree(DbFileTree* filesTree, int parentId, QString filter, bool removeDeleted);

	void slot_getFileInfo(int parentId, QString fileName, DbFileInfo* out);
	void slot_getFilesInfo(std::vector<int>* fileIds, std::vector<DbFileInfo>* out);

	bool worker_getFilesInfo(const std::vector<QString>& fullPathFileNames, std::vector<DbFileInfo>* out);

	void slot_addFiles(std::vector<std::shared_ptr<DbFile>>* files, int parentId, bool ensureUniquesInParentTree, int uniqueFromFileId);
    void slot_deleteFiles(std::vector<DbFileInfo>* files);

	void slot_moveFiles(const std::vector<DbFileInfo>* files, int moveToParentId, std::vector<DbFileInfo>* movedFiles);
	void slot_renameFile(const DbFileInfo& file, QString newFileName, DbFileInfo* updatedFileInfo);

    void slot_getLatestVersion(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out);
    void slot_getLatestTreeVersion(const DbFileInfo& parentFileInfo, std::vector<std::shared_ptr<DbFile> >* out);
    void slot_getCheckedOutFiles(const std::vector<DbFileInfo>* parentFiles, std::vector<DbFileInfo>* out);

    void slot_getWorkcopy(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out);
    void slot_setWorkcopy(const std::vector<std::shared_ptr<DbFile>>* files);

    void slot_getSpecificCopy(const std::vector<DbFileInfo>* files, int changesetId, std::vector<std::shared_ptr<DbFile>>* out);
	void slot_getSpecificCopy(const std::vector<DbFileInfo>* files, QDateTime date, std::vector<std::shared_ptr<DbFile>>* out);

    void slot_checkIn(std::vector<DbFileInfo>* files, QString comment);
    void slot_checkInTree(std::vector<DbFileInfo>* parentFiles, std::vector<DbFileInfo>* outCheckedIn, QString comment);
    void slot_checkOut(std::vector<DbFileInfo>* files);
    void slot_undoChanges(std::vector<DbFileInfo>* files);

    void slot_fileHasChildren(bool* hasChildren, DbFileInfo* fileInfo);

	void slot_getHistory(std::vector<DbChangeset>* out);
	void slot_getFileHistory(DbFileInfo file, std::vector<DbChangeset>* out);
	void slot_getFileHistoryRecursive(DbFileInfo parentFile, std::vector<DbChangeset>* out);

	void slot_getChangesetDetails(int changeset, DbChangesetDetails* out);

    // Hardware Configuration
    //
    void slot_addDeviceObject(Hardware::DeviceObject* device, int parentId);

    // Signal management
    //
    void slot_getSignalsIDs(QVector<int>* signalsIDs);
	void slot_getSignals(SignalSet* signalSet, bool excludeDeleted);
	void slot_getTuningableSignals(SignalSet* signalSet);
	void getSignals(SignalSet* signalSet, bool excludeDeleted, bool tuningableOnly);
    void slot_getLatestSignal(int signalID, Signal* signal);
	void slot_getLatestSignalsByAppSignalIDs(QStringList appSignalIds, QVector<Signal>* signalArray);

	void slot_addSignal(E::SignalType signalType, QVector<Signal>* newSignal);

	bool addSignal(E::SignalType signalType, QVector<Signal>* newSignal);
	bool setSignalWorkcopy(QSqlDatabase& db, const Signal& s, ObjectState& objectState, QString& errMsg);

    void slot_checkoutSignals(QVector<int>* signalIDs, QVector<ObjectState>* objectStates);
	void slot_setSignalWorkcopy(Signal *signal, ObjectState *objectState);
	void slot_setSignalsWorkcopies(const QVector<Signal>* signalsList);

    void slot_deleteSignal(int signalID, ObjectState* objectState);
    void slot_undoSignalChanges(int signalID, ObjectState* objectState);

    void slot_checkinSignals(QVector<int>* signalIDs, QString comment, QVector<ObjectState>* objectState);

	void slot_autoAddSignals(const std::vector<Hardware::DeviceSignal*>* deviceSignals, std::vector<Signal>* addedSignals);
    void slot_autoDeleteSignals(const std::vector<Hardware::DeviceSignal*>* deviceSignals);

	bool isSignalWithEquipmentIDExists(const QString& equipmentID);

	void slot_getSignalsIDsWithAppSignalID(QString appSignalID, QVector<int>* signalIDs);
	void slot_getSignalsIDsWithCustomAppSignalID(QString customAppSignalID, QVector<int>* signalIDs);
	void slot_getSignalsIDsWithEquipmentID(QString equipmentID, QVector<int>* signalIDs);
	void slot_getMultipleSignalsIDsWithEquipmentID(const QStringList& equipmentIDs, QHash<QString, int>* signalIDs);

	void slot_getSignalHistory(int signalID, std::vector<DbChangeset>* out);
	void slot_getSpecificSignals(const std::vector<int>* signalIDs, int changesetId, std::vector<Signal>* out);

	void slot_hasCheckedOutSignals(bool* hasCheckedOut);

	void hasCheckedOutSignals(QSqlDatabase& db, bool* hasCheckedOut);

    // Build management
    //
    void slot_buildStart(QString workstation, bool release, int changeset, int* buildID);
    void slot_buildFinish(int buildID, int errors, int warnings, QString buildLog);

    // Version Control
    //
	void slot_isAnyCheckedOut(int* checkedOutCount);
    void slot_lastChangesetId(int* lastChangesetId);

	// Next value in global counter, use for creating signals, etc with unique identifiers
	//
	void slot_nextCounterValue(int* counter);

    // Service
    //
	bool addLogRecord(QSqlDatabase db, QString text);

	bool db_logIn(QSqlDatabase db, QString username, QString password, QString* errorMessage);
	bool db_logOut(QSqlDatabase db);

	bool db_getCurrentUserId(QSqlDatabase db, int* userId);
    bool db_getUserData(QSqlDatabase db, int userId, DbUser* user);
    bool db_checkUserPassword(QSqlDatabase db, QString username, QString password);
    int db_getProjectVersion(QSqlDatabase db);

	static bool db_updateFileState(const QSqlQuery& q, DbFileInfo* fileInfo, bool checkFileId);
	static bool db_updateFile(const QSqlQuery& q, DbFile* file);
	static bool db_dbFileInfo(const QSqlQuery& q, DbFileInfo* fileInfo);
	static bool db_objectState(QSqlQuery& q, ObjectState* os);

	bool db_dbChangeset(const QSqlQuery& q, DbChangeset* out);
	bool db_dbChangesetObject(const QSqlQuery& q, DbChangesetDetails* destination);

    // Properties
    //
public:
    const QString& host() const;
    void setHost(const QString& host);

    int port() const;
    void setPort(int port);

    const QString& serverUsername() const;
    void setServerUsername(const QString& serverUsername);

    const QString& serverPassword() const;
    void setServerPassword(const QString& serverPassword);

    DbUser currentUser() const;
    void setCurrentUser(const DbUser& user);

    DbProject currentProject() const;
    void setCurrentProject(const DbProject& project);

private:
	const QString& sessionKey() const;
	void setSessionKey(QString value);

	//

	bool processingBeforeDatabaseUpgrade(QSqlDatabase& db, int newVersion, QString* errorMessage);
	bool processingBeforeDatabaseUpgrade0215(QSqlDatabase& db,QString* errorMessage);

	bool processingAfterDatabaseUpgrade(QSqlDatabase& db, int currentVersion, QString* errorMessage);
	bool processingAfterDatabaseUpgrade0215(QSqlDatabase& db, QString* errorMessage);
//	void getSignalDataAfterDatabaseUpdate0211(QSqlQuery& q, Signal& s);


private:
    mutable QMutex m_mutex;

    QString m_host;
    int m_port;
    QString m_serverUsername;
    QString m_serverPassword;
	QString m_sessionKey;
    DbProgress* m_progress;

    DbUser m_currentUser;
    DbProject m_currentProject;

	int m_afblFileId = -1;			// Application Functional Block Library
	int m_schemasFileId = -1;			// User Functional Block Libabry
	int m_ufblFileId = -1;			// User Functional Block Libabry
	int m_alFileId = -1;			// Application Logic
	int m_hcFileId = -1;			// Hardware Configuration
	int m_hpFileId = -1;			// Hardware Presets
	int m_mvsFileId = -1;			// Monitor Visualization Schemas
	int m_tvsFileId = -1;			// TuningClient Visualization Schemas
	int m_dvsFileId = -1;			// Diagnostics Visualization Schemas
	int m_mcFileId = -1;			// Module Configuration Template
	int m_connectionsFileId = -1;	// Connections
	int m_busTypesFileId = -1;		// BusTypes
	int m_etcFileId = -1;			//

    std::vector<DbFileInfo> m_systemFiles;		// All system files

    static const UpgradeItem upgradeItems[];

    static int counter;
    int m_instanceNo;				// Initialized by counter in constructor
};

