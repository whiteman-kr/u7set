#pragma once

#include <QSqlQuery>
#include <QReadWriteLock>

#include "DbStruct.h"
#include "../AppSignalLib/AppSignal.h"

#define AUTO_COMPLETE std::shared_ptr<int*> progressCompleted(nullptr, [this](void*) { this->m_progress->setCompleted(true); } );

namespace Hardware
{
    class DeviceObject;
	class DeviceAppSignal;
}

class DbProgress;


class DbWorker : public QObject
{
	Q_OBJECT

private:
	DbWorker();

public:
    DbWorker(DbProgress* progress);

	static void getSignalData(QSqlQuery& q, AppSignal &s);
	static QString getSignalDataStr(const AppSignal& s);

protected:
	QString postgresConnectionName() const;
    QString projectConnectionName() const;

    bool checkDatabaseFeatures(QSqlDatabase db);

	void emitError(QSqlDatabase db, const QSqlError& err, bool addLogRecord = true);
	void emitError(QSqlDatabase db, const QString& err, bool addLogRecord = true);

    //
    // Public methods
    //
public:
    static int databaseVersion();
    bool isProjectOpened() const;

	int rootFileId() const;							// Get project fileId for dir $root$
	int systemFileId(DbDir dir) const;				// Get project fileId for dir
	DbFileInfo systemFileInfo(DbDir dir) const;		// Get project fileId for dir

    std::vector<DbFileInfo> systemFiles() const;

	static QString toSqlStr(const QString& str);
	static QString toSqlBoolean(bool value);
	static QString toSqlByteaStr(const QByteArray& binData);

	// Hardware Configuration
	//
	[[nodiscard]] static std::shared_ptr<Hardware::DeviceObject> deviceObjectFromDbFile(const DbFile& file);
	[[nodiscard]] static std::vector<std::shared_ptr<Hardware::DeviceObject>> deviceObjectFromDbFiles(const std::vector<std::shared_ptr<DbFile>>& files);

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

	void slot_setUserProperty(QString propertyName, QString propertyValue);
	void slot_getUserProperty(QString propertyName, QString* out);
	void slot_getUserPropertyList(QString propertyTemplate, QStringList* out);
	void slot_removeUserProperty(QString propertyName);

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

	void slot_getFullPathFilesInfo(const std::vector<QString>* fullPathFilenames, std::vector<DbFileInfo>* out);
	bool worker_getFilesInfo(const std::vector<QString>& fullPathFileNames, std::vector<DbFileInfo>* out);
	bool worker_getFileInfo(const QString& fullPathFileName, DbFileInfo* out);

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
	void slot_getSignalsIDAppSignalID(QVector<ID_AppSignalID>* signalsIDAppSignalID);
	void slot_getSignals(AppSignalSet* signalSet, bool excludeDeleted);
	void slot_getTunableSignals(AppSignalSet* signalSet);
	void getSignals(AppSignalSet* signalSet, bool excludeDeleted, bool tunableOnly);
    void slot_getLatestSignal(int signalID, AppSignal* signal);
	void slot_getLatestSignals(QVector<int> signalIDs, QVector<AppSignal>* signalsArray);
	void slot_getLatestSignalsByAppSignalIDs(QStringList appSignalIds, QVector<AppSignal>* signalArray);
	void slot_getLatestSignalsWithUserID(std::vector<AppSignal>* out);
	void slot_getCheckedOutSignalsIDs(QVector<int>* signalsIDs);
	void slot_addSignal(E::SignalType signalType, QVector<AppSignal>* newSignal);

	bool addSignal(E::SignalType signalType, QVector<AppSignal>* newSignal);
	bool setSignalWorkcopy(QSqlDatabase& db, const AppSignal& s, ObjectState& objectState, QString& errMsg);

    void slot_checkoutSignals(QVector<int>* signalIDs, QVector<ObjectState>* objectStates);
	void slot_setSignalWorkcopy(AppSignal *signal, ObjectState *objectState);
	void slot_setSignalsWorkcopies(const QVector<AppSignal>* signalsList);

    void slot_deleteSignal(int signalID, ObjectState* objectState);
    void slot_undoSignalChanges(int signalID, ObjectState* objectState);
	void slot_undoSignalsChanges(QVector<int> signalIDs, QVector<ObjectState>* objectStates);

    void slot_checkinSignals(QVector<int>* signalIDs, QString comment, QVector<ObjectState>* objectState);

	void slot_autoAddSignals(const std::vector<Hardware::DeviceAppSignal*>* deviceSignals, std::vector<AppSignal>* addedSignals);
	QString initAppSignalFromDeviceAppSignal(const Hardware::DeviceAppSignal& deviceSignal, AppSignal* appSignal);
    void slot_autoDeleteSignals(const std::vector<Hardware::DeviceAppSignal*>* deviceSignals);

	bool isSignalWithEquipmentIDExists(const QString& equipmentID);

	void slot_getSignalsIDsWithAppSignalID(QString appSignalID, QVector<int>* signalIDs);
	void slot_getSignalsIDsWithCustomAppSignalID(QString customAppSignalID, QVector<int>* signalIDs);
	void slot_getSignalsIDsWithEquipmentID(QString equipmentID, QVector<int>* signalIDs);
	void slot_getMultipleSignalsIDsWithEquipmentID(const QStringList& equipmentIDs, QMultiHash<QString, int>* signalIDs);

	void slot_getSignalHistory(int signalID, std::vector<DbChangeset>* out);

	void slot_getSpecificSignals(const std::vector<int>* signalIDs, int changesetId, std::vector<AppSignal>* out);

	void slot_getSpecificSignals(int changesetId, std::vector<AppSignal>* out);
	void slot_getSpecificSignals(QDateTime date, std::vector<AppSignal>* out);

	void readSignalsToVector(QSqlQuery& q, std::vector<AppSignal>* out);

	void slot_hasCheckedOutSignals(bool* hasCheckedOut);

	void hasCheckedOutSignals(QSqlDatabase& db, bool* hasCheckedOut);

    // Build management
    //
	void slot_buildStart(QString workstation, int changeset, int* buildID);
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

	static bool db_dbChangeset(const QSqlQuery& q, DbChangeset* out);
	static bool db_dbChangesetObject(const QSqlQuery& q, DbChangesetDetails* destination);

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
	bool processingAfterDatabaseUpgrade0302(QSqlDatabase& db, QString* errorMessage);
//	void getSignalDataAfterDatabaseUpdate0211(QSqlQuery& q, Signal& s);


private:
	mutable QReadWriteLock m_lock;

    QString m_host;
    int m_port;
    QString m_serverUsername;
    QString m_serverPassword;
	QString m_sessionKey;
    DbProgress* m_progress;

    DbUser m_currentUser;
    DbProject m_currentProject;

	std::map<DbDir, DbFileInfo>	m_systemFiles;		// All system files

    static const UpgradeItem upgradeItems[];

    static int counter;
    int m_instanceNo;				// Initialized by counter in constructor
};

