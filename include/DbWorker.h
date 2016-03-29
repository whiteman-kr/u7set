#pragma once

#include <QSqlQuery>
#include <QMutex>

#include "DbStruct.h"
#include "../include/Signal.h"

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

	void getSignalData(QSqlQuery& q, Signal &s);
	QString getSignalDataStr(const Signal& s);

	void getObjectState(QSqlQuery& q, ObjectState &os);

public:
	DbWorker(DbProgress* progress);

protected:
	QString postgresConnectionName() const;
	QString projectConnectionName() const;

	bool checkDatabaseFeatures(QSqlDatabase db);

	void emitError(const QSqlError& err);
	void emitError(const QString& err);

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
	int alFileId() const;
	int hcFileId() const;
	int hpFileId() const;
	int mvsFileId() const;
	int dvsFileId() const;
	int mcFileId() const;

	std::vector<DbFileInfo> systemFiles() const;

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
	void slot_deleteProject(QString projectName, QString password, bool doNotBackup);
	void slot_upgradeProject(QString projectName, QString password, bool doNotBackup);

	// User management
	//
	void slot_createUser(DbUser user);
	void slot_updateUser(DbUser user);
	void slot_getUserList(std::vector<DbUser>* out);

	// File management
	//
	void slot_getFileList(std::vector<DbFileInfo>* files, int parentId, QString filter, bool removeDeleted);
	void getFileList_worker(std::vector<DbFileInfo>* files, int parentId, QString filter, bool removeDeleted);

	void slot_getFileInfo(std::vector<int>* fileIds, std::vector<DbFileInfo>* out);

	void slot_addFiles(std::vector<std::shared_ptr<DbFile>>* files, int parentId);
	void slot_deleteFiles(std::vector<DbFileInfo>* files);

	void slot_getLatestVersion(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out);
	void slot_getLatestTreeVersion(const DbFileInfo& parentFileInfo, std::list<std::shared_ptr<DbFile>>* out);
	void slot_getCheckedOutFiles(const std::vector<DbFileInfo>* parentFiles, std::vector<DbFileInfo>* out);

	void slot_getWorkcopy(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out);
	void slot_setWorkcopy(const std::vector<std::shared_ptr<DbFile>>* files);

	void slot_getSpecificCopy(const std::vector<DbFileInfo>* files, int changesetId, std::vector<std::shared_ptr<DbFile>>* out);

	void slot_checkIn(std::vector<DbFileInfo>* files, QString comment);
	void slot_checkInTree(std::vector<DbFileInfo>* parentFiles, std::vector<DbFileInfo>* outCheckedIn, QString comment);
	void slot_checkOut(std::vector<DbFileInfo>* files);
	void slot_undoChanges(std::vector<DbFileInfo>* files);

	void slot_fileHasChildren(bool* hasChildren, DbFileInfo* fileInfo);

	void slot_getFileHistory(DbFileInfo* file, std::vector<DbChangesetInfo>* out);

	// Hardware Configuration
	//
	void slot_addDeviceObject(Hardware::DeviceObject* device, int parentId);

	// Signal management
	//
	void slot_getSignalsIDs(QVector<int>* signalsIDs);
	void slot_getSignals(SignalSet* signalSet);
	void slot_getLatestSignal(int signalID, Signal* signal);
	void slot_addSignal(E::SignalType signalType, QVector<Signal>* newSignal);

	void addSignal(E::SignalType signalType, QVector<Signal>* newSignal);

	void slot_getUnits(UnitList* units);

	void slot_checkoutSignals(QVector<int>* signalIDs, QVector<ObjectState>* objectStates);
	void slot_setSignalWorkcopy(Signal* signal, ObjectState *objectState);

	void slot_deleteSignal(int signalID, ObjectState* objectState);
	void slot_undoSignalChanges(int signalID, ObjectState* objectState);

	void slot_checkinSignals(QVector<int>* signalIDs, QString comment, QVector<ObjectState>* objectState);

	void slot_autoAddSignals(const std::vector<Hardware::DeviceSignal*>* deviceSignals);
	void slot_autoDeleteSignals(const std::vector<Hardware::DeviceSignal*>* deviceSignals);

	// Build management
	//
	void slot_buildStart(QString workstation, bool release, int changeset, int* buildID);
	void slot_buildFinish(int buildID, int errors, int warnings, QString buildLog);

	// Version Control
	//
	void slot_isAnyCheckedOut(bool* checkedOut);
	void slot_lastChangesetId(int* lastChangesetId);

	// Service
	//
	bool db_getUserData(QSqlDatabase db, int userId, DbUser* user);
	bool db_checkUserPassword(QSqlDatabase db, QString username, QString password);
	int db_getProjectVersion(QSqlDatabase db);

	bool db_updateFileState(const QSqlQuery& q, DbFileInfo* fileInfo, bool checkFileId) const;
	bool db_updateFile(const QSqlQuery& q, DbFile* file) const;
	bool db_dbFileInfo(const QSqlQuery& q, DbFileInfo* fileInfo);

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
	mutable QMutex m_mutex;

	QString m_host;
	int m_port;
	QString m_serverUsername;
	QString m_serverPassword;
	DbProgress* m_progress;

	DbUser m_currentUser;
	DbProject m_currentProject;

	int m_afblFileId = -1;	// Application Functional Block Library
	int m_alFileId = -1;	// Application Logic
	int m_hcFileId = -1;	// Hardware Configuration
	int m_hpFileId = -1;	// Hardware Presets
	int m_mvsFileId = -1;	// Monitor Visualization Schemas
	int m_dvsFileId = -1;	// Diagnostics Visualization Schemas
	int m_mcFileId = -1;	// Module Configuration Template

	std::vector<DbFileInfo> m_systemFiles;		// All system files

	static const UpgradeItem upgradeItems[];

	static int counter;
	int m_instanceNo;				// Initialized by counter in constructor
};

