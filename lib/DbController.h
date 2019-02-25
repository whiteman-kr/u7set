#pragma once

#include <QVector>
#include <QThread>

#include "../lib/DbStruct.h"
#include "../lib/DbWorker.h"
#include "../lib/DeviceObject.h"
#include "../lib/DbProgress.h"

class DbController : public QObject
{
	Q_OBJECT

public:
	DbController();
	~DbController();

	static void init();
	static void shutdown();

	//
	// Public signlas
	//
signals:
	void projectOpened(DbProject project);
	void projectClosed();

	//
	// Operations on Project Database
	//
public:

	// Project Management
	//
	bool getProjectList(std::vector<DbProject>* out, QWidget* parentWidget);
	bool createProject(QString projectName, QString administratorPassword, QWidget* parentWidget);
	bool openProject(QString projectName, QString username, QString password, QWidget* parentWidget);
	bool closeProject(QWidget* parentWidget);
	bool cloneProject(QString projectName, QString password, QString newProjectName, QWidget* parentWidget);
	bool deleteProject(QString projectName, QString password, bool doNotBackup, QWidget* parentWidget);
	bool upgradeProject(QString projectName, QString password, bool doNotBackup, QWidget* parentWidget);

	bool setProjectProperty(QString propertyName, bool propertyValue, QWidget* parentWidget);
	bool setProjectProperty(QString propertyName, QString propertyValue, QWidget* parentWidget);

	bool getProjectProperty(QString propertyName, bool* out, QWidget* parentWidget);
	bool getProjectProperty(QString propertyName, QString* out, QWidget* parentWidget);

	// User management
	//
	bool createUser(const DbUser& user, QWidget* parentWidget);
	bool updateUser(const DbUser& user, QWidget* parentWidget);
	bool getUserList(std::vector<DbUser>* out, QWidget* parentWidget);

	// File management
	//
	bool isFileExists(QString fileName, int parentId, int* fileId, QWidget* parentWidget);

	bool getFileList(std::vector<DbFileInfo>* files, int parentId, bool removeDeleted, QWidget* parentWidget);
	bool getFileList(std::vector<DbFileInfo>* files, int parentId, QString filter, bool removeDeleted, QWidget* parentWidget);

	bool getFileListTree(DbFileTree* filesTree, int parentId, bool removeDeleted, QWidget* parentWidget);
	bool getFileListTree(DbFileTree* filesTree, int parentId, QString filter, bool removeDeleted, QWidget* parentWidget);

	bool getFileInfo(int parentId, QString fileName, DbFileInfo* out, QWidget* parentWidget);
	bool getFileInfo(int fileId, DbFileInfo* out, QWidget* parentWidget);
	bool getFileInfo(std::vector<int>* fileIds, std::vector<DbFileInfo>* out, QWidget* parentWidget);

	bool addFiles(std::vector<std::shared_ptr<DbFile>>* files, int parentId, bool ensureUniquesInParentTree, int uniqueFromFileId, QWidget* parentWidget);
	bool addFiles(std::vector<std::shared_ptr<DbFile>>* files, int parentId, QWidget* parentWidget);
	bool addFile(const std::shared_ptr<DbFile>& file, int parentId, QWidget* parentWidget);
	bool addUniqueFile(const std::shared_ptr<DbFile>& file, int parentId, int uniqueFromFileId, QWidget* parentWidget);

	bool deleteFiles(std::vector<std::shared_ptr<DbFileInfo> >* files, QWidget* parentWidget);
	bool deleteFiles(std::vector<DbFileInfo>* files, QWidget* parentWidget);

	bool moveFiles(const std::vector<std::shared_ptr<DbFileInfo>>& files, int moveToParentId, std::vector<DbFileInfo>* movedFiles, QWidget* parentWidget);
	bool moveFiles(const std::vector<DbFileInfo>& files, int moveToParentId, std::vector<DbFileInfo>* movedFiles, QWidget* parentWidget);

	bool renameFile(const DbFileInfo& file, QString newFileName, DbFileInfo* updatedFileInfo, QWidget* parentWidget);

	bool getLatestVersion(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget);
	bool getLatestVersion(const DbFileInfo& file, std::shared_ptr<DbFile>* out, QWidget* parentWidget);
	bool getLatestTreeVersion(const DbFileInfo& file, std::vector<std::shared_ptr<DbFile> >* out, QWidget* parentWidget);

	bool getCheckedOutFiles(const DbFileInfo& parentFile, std::vector<DbFileInfo>* out, QWidget* parentWidget);
	bool getCheckedOutFiles(const std::vector<DbFileInfo>* parentFiles, std::vector<DbFileInfo>* out, QWidget* parentWidget);

	bool getWorkcopy(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget);
	bool getWorkcopy(const DbFileInfo& file, std::shared_ptr<DbFile>* out, QWidget* parentWidget);

	bool setWorkcopy(const std::vector<std::shared_ptr<DbFile>>& files, QWidget* parentWidget);
	bool setWorkcopy(const std::shared_ptr<DbFile>& file, QWidget* parentWidget);

	bool getSpecificCopy(const std::vector<DbFileInfo>& files, int changesetId, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget);
	bool getSpecificCopy(const DbFileInfo& file, int changesetId, std::shared_ptr<DbFile>* out, QWidget* parentWidget);

	bool getSpecificCopy(const std::vector<DbFileInfo>& files, QDateTime date, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget);
	bool getSpecificCopy(const DbFileInfo& file, QDateTime date, std::shared_ptr<DbFile>* out, QWidget* parentWidget);

	bool checkIn(DbFileInfo& file, const QString& comment, QWidget* parentWidget);
	bool checkIn(std::vector<DbFileInfo>& files, const QString& comment, QWidget* parentWidget);
	bool checkInTree(std::vector<DbFileInfo>& parentFiles, std::vector<DbFileInfo>* outCheckedIn, const QString& comment, QWidget* parentWidget);
	bool checkOut(DbFileInfo& file, QWidget* parentWidget);
	bool checkOut(std::vector<DbFileInfo>& files, QWidget* parentWidget);
	bool undoChanges(DbFileInfo& file, QWidget* parentWidget);
	bool undoChanges(std::vector<DbFileInfo>& files, QWidget* parentWidget);

	bool fileHasChildren(bool* hasChildren, DbFileInfo& file, QWidget* parentWidget);

	bool getProjectHistory(std::vector<DbChangeset>* out, QWidget* parentWidget);
	bool getFileHistory(const DbFileInfo& file, std::vector<DbChangeset>* out, QWidget* parentWidget);
	bool getFileHistoryRecursive(const DbFileInfo& parentFile, std::vector<DbChangeset>* out, QWidget* parentWidget);

	bool getChangesetDetails(int changeset, DbChangesetDetails* out, QWidget* parentWidget);

	// Hardware Configuration
	//
	bool addDeviceObject(Hardware::DeviceObject* device, int parentId, QWidget* parentWidget);
	bool deleteDeviceObjects(std::vector<Hardware::DeviceObject*>& devices, QWidget* parentWidget);
	bool getDeviceTreeLatestVersion(const DbFileInfo& file, std::shared_ptr<Hardware::DeviceObject>* out, QWidget* parentWidget);

	// Signals management
	//
	bool getSignalsIDs(QVector<int>* signalIDs, QWidget* parentWidget);
	bool getSignals(SignalSet* signalSet, bool excludeDeleted, QWidget* parentWidget);
	bool getTuningableSignals(SignalSet* signalSet, QWidget* parentWidget);
	bool getLatestSignal(int signalID, Signal* signal, QWidget* parentWidget);
	bool getLatestSignals(QVector<int> signalIDs, QVector<Signal>* signalsArray, QWidget* parentWidget);
	bool getLatestSignalsByAppSignalIDs(QStringList appSignalIDs, QVector<Signal>* signalArray, QWidget* parentWidget);
	bool getCheckedOutSignalsIDs(QVector<int>* signalIDs, QWidget* parentWidget);
	bool addSignal(E::SignalType signalType, QVector<Signal>* newSignal, QWidget* parentWidget);

	bool checkoutSignals(QVector<int>* signalIDs, QVector<ObjectState>* objectStates, QWidget* parentWidget);
	bool setSignalWorkcopy(Signal* signal, ObjectState* objectState, QWidget* parentWidget);
	bool setSignalsWorkcopies(const QVector<Signal>* signalsList, QWidget* parentWidget);

	bool deleteSignal(int signalID, ObjectState* objectState, QWidget* parentWidget);
	bool undoSignalChanges(int signalID, ObjectState* objectState, QWidget* parentWidget);

	bool checkinSignals(QVector<int>* signalIDs, QString comment, QVector<ObjectState>* objectState, QWidget* parentWidget);

	bool autoAddSignals(const std::vector<Hardware::DeviceSignal*>* deviceSignals, std::vector<Signal>* addedSignals, QWidget* parentWidget);
	bool autoDeleteSignals(const std::vector<Hardware::DeviceSignal*>* deviceSignals, QWidget* parentWidget);

	bool getSignalsIDsWithAppSignalID(QString appSignalID, QVector<int>* signalIDs, QWidget* parentWidget);
	bool getSignalsIDsWithCustomAppSignalID(QString customAppSignalID, QVector<int>* signalIDs, QWidget* parentWidget);
	bool getSignalsIDsWithEquipmentID(const QString& equipmentID, QVector<int>* signalIDs, QWidget* parentWidget);
	bool getMultipleSignalsIDsWithEquipmentID(const QStringList& equipmentIDs, QHash<QString, int>* signalIDs, QWidget* parentWidget);

	bool getSignalHistory(int signalID, std::vector<DbChangeset>* out, QWidget* parentWidget);
	bool getSpecificSignals(const std::vector<int>* signalIDs, int changesetId, std::vector<Signal>* out, QWidget* parentWidget);

	bool hasCheckedOutSignals(bool* hasCheckedOut, QWidget* parentWidget);

	// Build management
	//
	bool buildStart(QString workstation, bool release, int changeset, int* buildID, QWidget *parentWidget);
	bool buildFinish(int buildID, int errors, int warnings, QString buildLog, QWidget* parentWidget);

	// Version Control
	//
	bool isAnyCheckedOut(int* checkedOutCount);
	bool lastChangesetId(int* result);

	// Next value in global counter, use for creating signals, etc with unique identifiers
	//
	bool nextCounterValue(int* counter);
	int nextCounterValue();

signals:
	void signal_getProjectList(std::vector<DbProject>* out);
	void signal_createProject(QString projectName, QString administratorPassword);
	void signal_openProject(QString projectName, QString username, QString password);
	void signal_closeProject();
	void signal_cloneProject(QString projectName, QString password, QString newProjectName);
	void signal_deleteProject(QString projectName, QString password, bool doNotBackup);
	void signal_upgradeProject(QString projectName, QString password, bool doNotBackup);

	void signal_setProjectProperty(QString propertyName, QString propertyValue);
	void signal_getProjectProperty(QString propertyName, QString* out);

	void signal_createUser(DbUser user);
	void signal_updateUser(DbUser user);
	void signal_getUserList(std::vector<DbUser>* out);

	bool signal_isFileExists(QString fileName, int parentId, int* fileId);

	void signal_getFileList(std::vector<DbFileInfo>* files, int parentId, QString filter, bool removeDeleted);
	void signal_getFileListTree(DbFileTree* filesTree, int parentId, QString filter, bool removeDeleted);

	void signal_getFileInfo(int parentId, QString fileName, DbFileInfo* out);
	void signal_getFilesInfo(std::vector<int>* fileIds, std::vector<DbFileInfo>* out);

	void signal_addFiles(std::vector<std::shared_ptr<DbFile>>* files, int parentId, bool ensureUniquesInParentTree, int uniqueFromFileId);
	void signal_deleteFiles(std::vector<DbFileInfo>* files);

	void signal_moveFiles(const std::vector<DbFileInfo>* files, int moveToParentId, std::vector<DbFileInfo>* movedFiles);
	void signal_renameFile(const DbFileInfo& file, QString newFileName, DbFileInfo* updatedFileInfo);

	void signal_getLatestVersion(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out);
	void signal_getLatestTreeVersion(const DbFileInfo& parentFileInfo, std::vector<std::shared_ptr<DbFile>>* out);
	void signal_getCheckedOutFiles(const std::vector<DbFileInfo>* parentFiles, std::vector<DbFileInfo>* out);

	void signal_getWorkcopy(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out);
	void signal_setWorkcopy(const std::vector<std::shared_ptr<DbFile>>* files);

	void signal_getSpecificCopy(const std::vector<DbFileInfo>* files, int changesetId, std::vector<std::shared_ptr<DbFile>>* out);
	void signal_getSpecificCopy(const std::vector<DbFileInfo>* files, QDateTime date, std::vector<std::shared_ptr<DbFile>>* out);

	void signal_checkIn(std::vector<DbFileInfo>* files, QString comment);
	void signal_checkInTree(std::vector<DbFileInfo>* parentFiles, std::vector<DbFileInfo>* outCheckedIn, QString comment);
	void signal_checkOut(std::vector<DbFileInfo>* files);
	void signal_undoChanges(std::vector<DbFileInfo>* files);

	void signal_fileHasChildren(bool* hasChildren, DbFileInfo* fileInfo);

	void signal_getHistory(std::vector<DbChangeset>* out);
	void signal_getFileHistory(DbFileInfo file, std::vector<DbChangeset>* out);
	void signal_getFileHistoryRecursive(DbFileInfo parentFile, std::vector<DbChangeset>* out);

	void signal_getChangesetDetails(int changeset, DbChangesetDetails* out);

	// --
	//
	void signal_addDeviceObject(Hardware::DeviceObject* device, int parentId);

	void signal_getSignalsIDs(QVector<int>* signalIDs);
	void signal_getSignals(SignalSet* signalSet, bool excludeDeleted);
	void signal_getTuningableSignals(SignalSet* signalSet);
	void signal_getLatestSignal(int signalID, Signal* signal);
	void signal_getLatestSignals(QVector<int> signalIDs, QVector<Signal>* signalsArray);
	void signal_getLatestSignalsByAppSignalIDs(QStringList appSignalIDs, QVector<Signal>* signalArray);
	void signal_getCheckedOutSignalsIDs(QVector<int>* signalIDs);
	void signal_addSignal(E::SignalType signalType, QVector<Signal>* newSignal);

	void signal_checkoutSignals(QVector<int>* signalIDs, QVector<ObjectState>* objectStates);
	void signal_setSignalWorkcopy(Signal* signal, ObjectState* objectState);
	void signal_setSignalsWorkcopies(const QVector<Signal>* signalsList);
	void signal_deleteSignal(int signalID, ObjectState* objectState);
	void signal_undoSignalChanges(int signalID, ObjectState* objectState);
	void signal_checkinSignals(QVector<int>* signalIDs, QString comment, QVector<ObjectState>* objectState);

	void signal_autoAddSignals(const std::vector<Hardware::DeviceSignal*>* deviceSignals, std::vector<Signal>* addedSignals);
	void signal_autoDeleteSignals(const std::vector<Hardware::DeviceSignal*>* deviceSignals);

	void signal_getSignalsIDsWithAppSignalID(QString appSignalID, QVector<int>* signalIDs);
	void signal_getSignalsIDsWithCustomAppSignalID(QString customAppSignalID, QVector<int>* signalIDs);
	void signal_getSignalsIDsWithEquipmentID(QString equipID, QVector<int>* signalIDs);
	void signal_getMultipleSignalsIDsWithEquipmentID(const QStringList& equipmentIDs, QHash<QString, int>* signalIDs);

	void signal_getSignalHistory(int signalID, std::vector<DbChangeset>* out);
	void signal_getSpecificSignals(const std::vector<int>* signalIDs, int changesetId, std::vector<Signal>* out);
	void signal_hasCheckedOutSignals(bool* hasCheckedOutSignals);

	// Build management
	//
	void signal_buildStart(QString workstation, bool release, int changeset, int* buildID);
	void signal_buildFinish(int buildID, int errors, int warnings, QString buildLog);

	// Version Control
	//
	void signal_isAnyCheckedOut(int* checkedOutCount);
	void signal_lastChangesetId(int* lastChangesetId);

	// Next value in global counter, use for creating signals, etc with unique identifiers
	//
	void signal_nextCounterValue(int* counter);

	//
	// Service functions
	//
protected:
	bool initOperation();
	bool waitForComplete(QWidget* parentWidget, const QString& description);

public:
	bool isProjectOpened() const;
	static int databaseVersion();

	//
	// Properties
	//
public:
	void enableProgress();
	void disableProgress();

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

	int rootFileId() const;			// Root file
	int schemaFileId() const;		// $root$/Schemas file id
	int afblFileId() const;			// Application Functional Block Library
	int ufblFileId() const;			// User Functional Block Library
	int alFileId() const;			// Application Logic
	int hcFileId() const;			// Hardware Configuration
	int hpFileId() const;			// Hadware Presets
	int mcFileId() const;			// Module Configuration
	int mvsFileId() const;			// Monitor Video Schemas
	int tvsFileId() const;			// Tuning Video Schemas
	int dvsFileId() const;			// Diaginostics Video Schemas
	int connectionsFileId() const;	// Connections
	int busTypesFileId() const;		// BusTypes
	int etcFileId() const;			//

	std::vector<DbFileInfo> systemFiles() const;

	DbFileInfo systemFileInfo(const QString& fileName) const;
	DbFileInfo systemFileInfo(int fileId) const;
	bool isSystemFile(int fileId) const;

	QString lastError() const;

	QString username(int userId) const;

	//
	// Data
	//
private:
	QThread m_thread;
	DbWorker* m_worker;

	//mutable QMutex m_mutex;
	mutable QMutex m_operationMutex;		// Non Recursive mutex for single opartion at a time

	DbProgress m_progress;

	QString m_lastError;

	// Users
	//
	mutable QMutex m_userMutex;
	std::map<int, DbUser> m_users;
};


class HasDbController
{
public:
	HasDbController() = delete;
	explicit HasDbController(DbController* db);

	// Properties
	//
protected:
	DbController* db() noexcept
	{
		return m_db;
	}

	const DbController* db() const noexcept
	{
		return m_db;
	}

	DbController* dbc() noexcept
	{
		return m_db;
	}

	const DbController* dbc() const noexcept
	{
		return m_db;
	}

	// Data
	//
private:
	DbController* m_db = nullptr;
};

