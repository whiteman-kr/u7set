#pragma once

#include "DbStruct.h"


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

	static const UpgradeItem upgradeItems[];

	static int counter;
	int m_instanceNo;				// Initialized by counter in constructor
};

