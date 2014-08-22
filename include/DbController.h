#pragma once

#include "DbStruct.h"
#include "DbWorker.h"

class DbController : public QObject
{
	Q_OBJECT
public:

	DbController();
	~DbController();

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
	bool getProjectList(std::vector<DbProject>* out, QWidget* parentWidget);
	bool createProject(const QString& projectName, const QString& administratorPassword, QWidget* parentWidget);
	bool openProject(const QString& projectName, const QString& username, const QString& password, QWidget* parentWidget);
	bool closeProject(QWidget* parentWidget);
	bool deleteProject(const QString& projectName, const QString& password, QWidget* parentWidget);
	bool upgradeProject(const QString& projectName, const QString& password, QWidget* parentWidget);

signals:
	void signal_getProjectList(std::vector<DbProject>* out);
	void signal_createProject(QString projectName, QString administratorPassword);
	void signal_openProject(QString projectName, QString username, QString password);
	void signal_closeProject();
	void signal_deleteProject(QString projectName, QString password);
	void signal_upgradeProject(QString projectName, QString password);

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

	//
	// Data
	//
private:
	QThread m_thread;
	DbWorker* m_worker;

	//mutable QMutex m_mutex;
	mutable QMutex m_operationMutex;	// Non Recursive mutex for single opartion at a time

	DbProgress m_progress;
};
