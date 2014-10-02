#pragma once

#include "DbStruct.h"
#include "../include/Signal.h"

#define AUTO_COMPLETE std::shared_ptr<int*> progressCompleted(nullptr, [this](void*) { this->m_progress->setCompleted(true); } );

namespace Hardware
{
	class DeviceObject;
};

class DbWorker : public QObject
{
	Q_OBJECT

private:
	DbWorker();

	void getSignalData(QSqlQuery& q, Signal &s);

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
	int wvsFileId() const;
	int dvsFileId() const;

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
	void slot_deleteProject(QString projectName, QString password);
	void slot_upgradeProject(QString projectName, QString password);

	// User management
	//
	void slot_createUser(DbUser user);
	void slot_updateUser(DbUser user);
	void slot_getUserList(std::vector<DbUser>* out);

	// File management
	//
	void slot_getFileList(std::vector<DbFileInfo>* files, int parentId, QString filter);
	void slot_addFiles(std::vector<std::shared_ptr<DbFile>>* files, int parentId);
	void slot_deleteFiles(std::vector<DbFileInfo>* files);

	void slot_getLatestVersion(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out);
	void slot_getLatestTreeVersion(const DbFileInfo& parentFileInfo, std::list<std::shared_ptr<DbFile>>* out);

	void slot_getWorkcopy(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out);
	void slot_setWorkcopy(const std::vector<std::shared_ptr<DbFile>>* files);

	void slot_checkIn(std::vector<DbFileInfo>* files, QString comment);
	void slot_checkOut(std::vector<DbFileInfo>* files);
	void slot_undoChanges(std::vector<DbFileInfo>* files);

	void slot_fileHasChildren(bool* hasChildren, DbFileInfo* fileInfo);

	// Hardware Configuration
	//
	void slot_addDeviceObject(Hardware::DeviceObject* device, int parentId);

	// Signal management
	//
	void slot_getSignalsIDs(QVector<int>* signalsIDs);
	void slot_getSignals(SignalSet* signalSet);
	void slot_addSignal(SignalType signalType, QVector<Signal>* newSignal);

	void slot_getUnits(QVector<Unit>* units);
	void slot_getDataFormats(QVector<DataFormat>* dataFormats);

	// Service
	//
	bool db_getUserData(QSqlDatabase db, int userId, DbUser* user);
	bool db_checkUserPassword(QSqlDatabase db, QString username, QString password);
	int db_getProjectVersion(QSqlDatabase db);

	bool db_updateFileState(const QSqlQuery& q, DbFileInfo* fileInfo, bool checkFileId) const;
	bool db_updateFile(const QSqlQuery& q, DbFile* file) const;

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
	int m_wvsFileId = -1;	// Workflow Visualization Schemes
	int m_dvsFileId = -1;	// Diagnostics Visualization Schemes

	static const UpgradeItem upgradeItems[];

	static int counter;
	int m_instanceNo;				// Initialized by counter in constructor
};

