#pragma once

#include "DbStruct.h"
#include "DbWorker.h"
#include "DeviceObject.h"
#include <QVector>

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

	// Project Management
	//
	bool getProjectList(std::vector<DbProject>* out, QWidget* parentWidget);
	bool createProject(const QString& projectName, const QString& administratorPassword, QWidget* parentWidget);
	bool openProject(const QString& projectName, const QString& username, const QString& password, QWidget* parentWidget);
	bool closeProject(QWidget* parentWidget);
	bool deleteProject(const QString& projectName, const QString& password, QWidget* parentWidget);
	bool upgradeProject(const QString& projectName, const QString& password, QWidget* parentWidget);

	// User management
	//
	bool createUser(const DbUser& user, QWidget* parentWidget);
	bool updateUser(const DbUser& user, QWidget* parentWidget);
	bool getUserList(std::vector<DbUser>* out, QWidget* parentWidget);

	// File management
	//
	bool getFileList(std::vector<DbFileInfo>* files, int parentId, QWidget* parentWidget);
	bool getFileList(std::vector<DbFileInfo>* files, int parentId, const QString& filter, QWidget* parentWidget);

	bool getFileInfo(int fileId, DbFileInfo* out, QWidget* parentWidget);
	bool getFileInfo(std::vector<int>* fileIds, std::vector<DbFileInfo>* out, QWidget* parentWidget);

	bool addFiles(std::vector<std::shared_ptr<DbFile>>* files, int parentId, QWidget* parentWidget);
	bool addFile(const std::shared_ptr<DbFile>& file, int parentId, QWidget* parentWidget);

	bool deleteFiles(std::vector<std::shared_ptr<DbFileInfo> >* files, QWidget* parentWidget);
	bool deleteFiles(std::vector<DbFileInfo>* files, QWidget* parentWidget);

	bool getLatestVersion(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget);
	bool getLatestVersion(const DbFileInfo& file, std::shared_ptr<DbFile>* out, QWidget* parentWidget);
	bool getLatestTreeVersion(const DbFileInfo& file, std::list<std::shared_ptr<DbFile>>* out, QWidget* parentWidget);

	bool getWorkcopy(const std::vector<DbFileInfo>& files, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget);
	bool getWorkcopy(const DbFileInfo& file, std::shared_ptr<DbFile>* out, QWidget* parentWidget);

	bool setWorkcopy(const std::vector<std::shared_ptr<DbFile>>& files, QWidget* parentWidget);
	bool setWorkcopy(const std::shared_ptr<DbFile>& file, QWidget* parentWidget);

	bool checkIn(DbFileInfo& file, const QString& comment, QWidget* parentWidget);
	bool checkIn(std::vector<DbFileInfo>& files, const QString& comment, QWidget* parentWidget);
	bool checkOut(DbFileInfo& file, QWidget* parentWidget);
	bool checkOut(std::vector<DbFileInfo>& files, QWidget* parentWidget);
	bool undoChanges(DbFileInfo& file, QWidget* parentWidget);
	bool undoChanges(std::vector<DbFileInfo>& files, QWidget* parentWidget);

	bool fileHasChildren(bool* hasChildren, DbFileInfo& file, QWidget* parentWidget);

	// Hardware Configuration
	//
	bool addDeviceObject(Hardware::DeviceObject* device, int parentId, QWidget* parentWidget);
	bool deleteDeviceObjects(std::vector<Hardware::DeviceObject*>& devices, QWidget* parentWidget);
	bool getDeviceTreeLatestVersion(const DbFileInfo& file, std::shared_ptr<Hardware::DeviceObject>* out, QWidget* parentWidget);

	// Signals management
	//
	bool getSignalsIDs(QVector<int>* signalIDs, QWidget* parentWidget);
	bool getSignals(SignalSet* signalSet, QWidget* parentWidget);
	bool addSignal(SignalType signalType, QVector<Signal>* newSignal, QWidget* parentWidget);

	bool getUnits(UnitList* units, QWidget* parentWidget);
	bool getDataFormats(DataFormatList* dataFormats, QWidget* parentWidget);

	bool checkoutSignals(QVector<int>* signalIDs, QVector<ObjectState>* objectStates, QWidget* parentWidget);
	bool setSignalWorkcopy(Signal* signal, ObjectState* objectState, QWidget* parentWidget);

	bool deleteSignal(int signalID, ObjectState* objectState, QWidget* parentWidget);
	bool undoSignalChanges(int signalID, ObjectState* objectState, QWidget* parentWidget);

signals:
	void signal_getProjectList(std::vector<DbProject>* out);
	void signal_createProject(QString projectName, QString administratorPassword);
	void signal_openProject(QString projectName, QString username, QString password);
	void signal_closeProject();
	void signal_deleteProject(QString projectName, QString password);
	void signal_upgradeProject(QString projectName, QString password);

	void signal_createUser(DbUser user);
	void signal_updateUser(DbUser user);
	void signal_getUserList(std::vector<DbUser>* out);

	void signal_getFileList(std::vector<DbFileInfo>* files, int parentId, QString filter);
	void signal_getFileInfo(std::vector<int>* fileIds, std::vector<DbFileInfo>* out);

	void signal_addFiles(std::vector<std::shared_ptr<DbFile>>* files, int parentId);
	void signal_deleteFiles(std::vector<DbFileInfo>* files);

	void signal_getLatestVersion(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out);
	void signal_getLatestTreeVersion(const DbFileInfo& parentFileInfo, std::list<std::shared_ptr<DbFile>>* out);

	void signal_getWorkcopy(const std::vector<DbFileInfo>* files, std::vector<std::shared_ptr<DbFile>>* out);
	void signal_setWorkcopy(const std::vector<std::shared_ptr<DbFile>>* files);

	void signal_checkIn(std::vector<DbFileInfo>* files, QString comment);
	void signal_checkOut(std::vector<DbFileInfo>* files);
	void signal_undoChanges(std::vector<DbFileInfo>* files);

	void signal_fileHasChildren(bool* hasChildren, DbFileInfo* fileInfo);

	void signal_addDeviceObject(Hardware::DeviceObject* device, int parentId);

	void signal_getSignalsIDs(QVector<int>* signalIDs);
	void signal_getSignals(SignalSet* signalSet);
	void signal_addSignal(SignalType signalType, QVector<Signal>* newSignal);

	void signal_getUnits(UnitList* units);
	void signal_getDataFormats(DataFormatList* dataFormats);
	void signal_checkoutSignals(QVector<int>* signalIDs, QVector<ObjectState>* objectStates);
	void signal_setSignalWorkcopy(Signal* signal, ObjectState* objectState);
	void signal_deleteSignal(int signalID, ObjectState* objectState);
	void signal_undoSignalChanges(int signalID, ObjectState* objectState);

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

	int rootFileId() const;		// Root file
	int afblFileId() const;		// Application Functional Block Library
	int alFileId() const;		// Application Logic
	int hcFileId() const;		// Hardware Configuration
	int hpFileId() const;		// Hadware Presets
	int wvsFileId() const;		// Workflow Video Schemes
	int dvsFileId() const;		// Diaginostics Video Schemes

	std::vector<DbFileInfo> systemFiles() const;
	DbFileInfo systemFileInfo(const QString& fileName) const;

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


class HasDbController
{

private:
	HasDbController();
public:
	explicit HasDbController(DbController* dbcontroller);

	// Properties
	//
protected:
	DbController* dbcontroller();

	// Data
	//
private:
	DbController* m_dbController;
};

