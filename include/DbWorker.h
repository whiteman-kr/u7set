#pragma once

#include "DbStruct.h"
#include "../include/Signal.h"


#define AUTO_COMPLETE std::shared_ptr<int*> progressCompleted(nullptr, [this](void*) { this->m_progress->setCompleted(true); } );


class DbWorker : public QObject
{
	Q_OBJECT

private:
	DbWorker();

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

	void slot_getWorkcopy(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out);
	void slot_setWorkcopy(const std::vector<std::shared_ptr<DbFile>>* files);

	void slot_checkIn(std::vector<DbFileInfo>* files, QString comment);
	void slot_checkOut(std::vector<DbFileInfo>* files);
	void slot_undoChanges(std::vector<DbFileInfo>* files);

	// Hardware Configuration
	//
	void slot_addSystem(DbFile* file);

	// Signal management
	//
	void slot_getSignalsIDs(QSet<int>* signalsIDs);
	void slot_getSignals(SignalSet* signalSet);
	void slot_addSignal(SignalType signalType, int channelCount, Signal signal, QVector<int>* newSignalsIDs);

	// Service
	//
	bool db_getUserData(QSqlDatabase db, int userId, DbUser* user);
	bool db_checkUserPassword(QSqlDatabase db, QString username, QString password);
	int db_getProjectVersion(QSqlDatabase db);

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

	int m_afblFileId;	// Application Functional Block Library
	int m_alFileId;		// Application Logic
	int m_hcFileId;		// Hardware Configuration
	int m_wvsFileId;	// Workflow Visualization Schemes
	int m_dvsFileId;	// Diagnostics Visualization Schemes

	static const UpgradeItem upgradeItems[];

	static int counter;
	int m_instanceNo;				// Initialized by counter in constructor
};

