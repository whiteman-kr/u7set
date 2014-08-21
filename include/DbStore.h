#pragma once

#include <QObject>
#include "DbStruct.h"
#include "DeviceObject.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////
/////////////////////////
class DbStore : public QObject
{
	Q_OBJECT

private:
	DbStore();
	virtual ~DbStore();

public:
	static DbStore* create();
	void destroy();

	// Public Methods, call these member from any thread
	//
public:
	int databaseVersion() const;

	// Private methods
	//
private:
	QSqlDatabase openPostgresDatabase();
	bool closePostgresDatabase();

	const QString& projectConnectionName() const;
	QSqlDatabase projectDatabase();

	bool initProgress();
	bool runProgress(QWidget* parentWidget, const QString& description);
	void emitError(const QSqlError& err);
	void emitError(const QString& err);

	//
	// Public signlas
	//
signals:
	void projectOpened();
	void projectClosed();

	//void error(QString message);
	//void completed(QString message);

	//
	// Asynchronous calls
	//
public:

	//
	// Blocking calls. DON't CALL THESE FUNCTIONS FROM COMMUNICATION THREAD!!!! IT WILL DEADLOCK THREADS!!!!
	//
public:
	bool debug();

	// Project Management -- Blocking
	//
	bool isProjectOpened();
	bool getProjectList(std::vector<DbProject>* projects, QWidget* parentWidget);	// Non Blocking
	void openProject(const QString& projectName, const QString& serverUsername, const QString& serverPassword);
	void closeProject();
	bool createProject(const QString& projectName, const QString& administratorPassword);
	void upgradeProject(const QString& projectName, QWidget* parentWidget);

	// User management -- Blocking
	//
	void createUser(const DbUser& user);
	void updateUser(const DbUser& user);
	void getUserList(std::vector<DbUser>& users);

	// File management
	//
	void getFileList(std::vector<DbFileInfo>& files);														// Blocking call
	void getFileList(std::vector<DbFileInfo>& files, bool justCheckedInState, const QString& filter);		// Blocking call
	void getFileInfo(int fileId, DbFileInfo* out);
	void getFileHistory(const DbFileInfo& file, std::vector<DbChangesetInfo>* out, QWidget* parentWidget);	// progress bar...
	void getCurrentUserAndCheckedInFileList(std::vector<DbFileInfo>& files, const QString& filter);			// Blocking call

	// Blocking with progress bar
	// Add file to the version control system.
	// File musn't be changed during the operation.
	// After adding file to DB, it's descripion will be changed in files list
	//
	void addFiles(std::vector<std::shared_ptr<DbFile>>* files, QWidget* parentWidget);

	bool undoFilesPendingChanges(const std::vector<DbFileInfo>& files, QWidget* parentWidget);
	bool checkInFiles(const std::vector<DbFileInfo>& files, const QString& comment, QWidget* parentWidget);
	bool checkOutFiles(const std::vector<DbFileInfo>& files, QWidget* parentWidget);

	bool getWorkcopy(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget);
	std::shared_ptr<DbFile> getWorkcopy(const DbFileInfo& file, QWidget* parentWidget);
	bool setWorkcopy(const std::vector<std::shared_ptr<DbFile>>& files, QWidget* parentWidget);
	bool setWorkcopy(const std::shared_ptr<DbFile>& file, QWidget* parentWidget);

	void getLatestCopy(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget);
	bool getSpecificCopy(int changesetId, const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget);

	// Equipment -- configuration
	//
	bool addSystem(DeviceSystem* system, QWidget* parentWidget);
	bool getEquipmentWorkcopy(DeviceRoot* out, QWidget* parentWidget);
	//bool setEquipmentWorkcopy(const DeviceRoot* root, QWidget* parentWidget);

signals:
	// Internal signals used to communicate with the pThread
	//
	void signal_debug();

	void signal_openConnection(QString ipAddress, int port, QString serverUsername, QString serverPassword);
	void signal_closeConnection();

	bool signal_isProjectOpen();
	void signal_getProjectList(std::vector<DbProject>* projects);
	void signal_openProject(QString projectName, QString username, QString password);
	void signal_closeProject();
	void signal_createProject(QString projectName, QString administratorPassword);
	void signal_upgradeProject(QString databaseName, DbProgress* progress);

	void signal_createUser(DbUser user);
	void signal_updateUser(DbUser user);
	void signal_getUserList(std::vector<DbUser>& users);

	void signal_getFileList(std::vector<DbFileInfo>& files, bool justCheckedInState, QString filter);
	void signal_getFileHistory(DbFileInfo file, std::vector<DbChangesetInfo>* out, DbProgress* progress);
	void signal_addFiles(std::vector<std::shared_ptr<DbFile>>* files, DbProgress* progress);

	void signal_undoFilesPendingChanges(const std::vector<DbFileInfo>& files, DbProgress* progress);
	void signal_checkInFiles(const std::vector<DbFileInfo>& filesIds, QString comment, DbProgress* progress);
	void signal_checkOutFiles(const std::vector<DbFileInfo>& files, DbProgress* progress);

	void signal_getWorkcopy(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, DbProgress* progress);
	void signal_setWorkcopy(const std::vector<std::shared_ptr<DbFile>>& files, DbProgress* progress);

	void signal_getLatestCopy(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, DbProgress* progress);
	void signal_getSpecificCopy(int changesetId, const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, DbProgress* progress);

	void signal_addSystem(DeviceSystem* system, DbProgress* progress);
	void signal_getEquipmentWorkcopy(DeviceRoot* out, DbProgress* progress);
	
	// These memebers must be run only in pThread context
	//
private slots:
	void slot_debug();

	bool slot_isProjectOpen();
	void slot_getProjectList(std::vector<DbProject>* projects);
	void slot_openProject(QString projectName, QString username, QString password);
	void slot_closeProject();
	void slot_createProject(const QString projectName, const QString administratorPassword);
	void slot_upgradeProject(const QString databaseName, DbProgress* progress);

	void slot_createUser(DbUser user);
	void slot_updateUser(DbUser user);
	void slot_getUserList(std::vector<DbUser>& users);
	bool db_getUserData(QSqlDatabase &db, int userId, DbUser *pUser);

	void slot_getFileList(std::vector<DbFileInfo>& files, bool justCheckedInState, QString filter);
	void slot_getFileHistory(DbFileInfo file, std::vector<DbChangesetInfo>* out, DbProgress* progress);
	void slot_addFiles(std::vector<std::shared_ptr<DbFile>>* files, DbProgress* progress);

	void slot_undoFilesPendingChanges(const std::vector<DbFileInfo>& files, DbProgress* progress);
	void slot_checkInFiles(const std::vector<DbFileInfo>& files, QString comment,  DbProgress* progress);
	void slot_checkOutFiles(const std::vector<DbFileInfo>& files, DbProgress* progress);

	void slot_getWorkcopy(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, DbProgress* progress);
	void slot_setWorkcopy(const std::vector<std::shared_ptr<DbFile>>& files, DbProgress* progress);

	void slot_getLatestCopy(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, DbProgress* progress);
	void slot_getSpecificCopy(int changesetId, const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, DbProgress* progress);

	void slot_addSystem(DeviceSystem* system, DbProgress* progress);
	void slot_getEquipmentWorkcopy(DeviceRoot* out, DbProgress* progress);

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

	// Data
	//
private:
	QThread* m_pThread;

	mutable QMutex m_mutex;
	mutable QMutex m_operationMutex;	// Non Recursive mutex for single opartion at a time

	QString m_host;
	int m_port;
	QString m_serverUsername;
	QString m_serverPassword;

	static const UpgradeItem upgradeItems[];

	DbUser m_currentUser;
	DbProject m_currentProject;

	DbProgress m_progress;
};

class HasDbStore
{

private:
	HasDbStore();
public:
	explicit HasDbStore(DbStore* dbstore);

	// Properties
	//
protected:
	DbStore* dbstore();

	// Data
	//
private:
	DbStore* m_pDbStore;
};

