#ifndef DB_LIB_DOMAIN
#error Don't include this file in the project! Link DbLib instead.
#endif

#include "DbController.h"
#include <QDebug>
#include <QtConcurrent>

DbController::DbController() :
	m_worker(nullptr),
	m_operationMutex(QMutex::NonRecursive)

{
	m_thread.setObjectName("DbWorkerThread");

	m_worker = new DbWorker(&m_progress);
	m_worker->moveToThread(&m_thread);

	connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);		// delete m_worker on thread termination

	//connect(&m_thread, &QThread::started, [](){qDebug() << "Database communication thread has been started";});
	//connect(&m_thread, &QThread::finished, [](){qDebug() << "Database communication thread has been finished";});

	// Notifications
	//

	// Operations
	//
	connect(this, &DbController::signal_getProjectList, m_worker, &DbWorker::slot_getProjectList);
	connect(this, &DbController::signal_createProject, m_worker, &DbWorker::slot_createProject);
	connect(this, &DbController::signal_openProject, m_worker, &DbWorker::slot_openProject);
	connect(this, &DbController::signal_closeProject, m_worker, &DbWorker::slot_closeProject);
	connect(this, &DbController::signal_cloneProject, m_worker, &DbWorker::slot_cloneProject);
	connect(this, &DbController::signal_deleteProject, m_worker, &DbWorker::slot_deleteProject);
	connect(this, &DbController::signal_upgradeProject, m_worker, &DbWorker::slot_upgradeProject);

	connect(this, &DbController::signal_setProjectProperty, m_worker, &DbWorker::slot_setProjectProperty);
	connect(this, &DbController::signal_getProjectProperty, m_worker, &DbWorker::slot_getProjectProperty);

	connect(this, &DbController::signal_setUserProperty, m_worker, &DbWorker::slot_setUserProperty);
	connect(this, &DbController::signal_getUserProperty, m_worker, &DbWorker::slot_getUserProperty);
	connect(this, &DbController::signal_getUserPropertyList, m_worker, &DbWorker::slot_getUserPropertyList);
	connect(this, &DbController::signal_removeUserProperty, m_worker, &DbWorker::slot_removeUserProperty);

	connect(this, &DbController::signal_createUser, m_worker, &DbWorker::slot_createUser);
	connect(this, &DbController::signal_updateUser, m_worker, &DbWorker::slot_updateUser);
	connect(this, &DbController::signal_getUserList, m_worker, &DbWorker::slot_getUserList);

	connect(this, &DbController::signal_isFileExists, m_worker, &DbWorker::slot_isFileExists);

	connect(this, &DbController::signal_getFileList, m_worker, &DbWorker::slot_getFileList);
	connect(this, &DbController::signal_getFileListTree, m_worker, &DbWorker::slot_getFileListTree);

	connect(this, &DbController::signal_getFileInfo, m_worker, &DbWorker::slot_getFileInfo);
	connect(this, &DbController::signal_getFilesInfo, m_worker, &DbWorker::slot_getFilesInfo);

	connect(this, &DbController::signal_getFullPathFilesInfo, m_worker, &DbWorker::slot_getFullPathFilesInfo);

	connect(this, &DbController::signal_addFiles, m_worker, &DbWorker::slot_addFiles);
	connect(this, &DbController::signal_deleteFiles, m_worker, &DbWorker::slot_deleteFiles);
	connect(this, &DbController::signal_moveFiles, m_worker, &DbWorker::slot_moveFiles);
	connect(this, &DbController::signal_renameFile, m_worker, &DbWorker::slot_renameFile);

	connect(this, &DbController::signal_getLatestVersion, m_worker, &DbWorker::slot_getLatestVersion);
	connect(this, &DbController::signal_getLatestTreeVersion, m_worker, &DbWorker::slot_getLatestTreeVersion);
	connect(this, &DbController::signal_getCheckedOutFiles, m_worker, &DbWorker::slot_getCheckedOutFiles);

	connect(this, &DbController::signal_getWorkcopy, m_worker, &DbWorker::slot_getWorkcopy);
	connect(this, &DbController::signal_setWorkcopy, m_worker, &DbWorker::slot_setWorkcopy);

	connect(this, static_cast<void(DbController::*)(const std::vector<DbFileInfo>*, int, std::vector<std::shared_ptr<DbFile>>*)>(&DbController::signal_getSpecificCopy),
			m_worker, static_cast<void(DbWorker::*)(const std::vector<DbFileInfo>*, int, std::vector<std::shared_ptr<DbFile>>*)>(&DbWorker::slot_getSpecificCopy));

	connect(this, static_cast<void(DbController::*)(const std::vector<DbFileInfo>*, QDateTime, std::vector<std::shared_ptr<DbFile>>*)>(&DbController::signal_getSpecificCopy),
			m_worker, static_cast<void(DbWorker::*)(const std::vector<DbFileInfo>*, QDateTime, std::vector<std::shared_ptr<DbFile>>*)>(&DbWorker::slot_getSpecificCopy));

	connect(this, &DbController::signal_checkIn, m_worker, &DbWorker::slot_checkIn);
	connect(this, &DbController::signal_checkInTree, m_worker, &DbWorker::slot_checkInTree);
	connect(this, &DbController::signal_checkOut, m_worker, &DbWorker::slot_checkOut);
	connect(this, &DbController::signal_undoChanges, m_worker, &DbWorker::slot_undoChanges);

	connect(this, &DbController::signal_fileHasChildren, m_worker, &DbWorker::slot_fileHasChildren);

	connect(this, &DbController::signal_getHistory, m_worker, &DbWorker::slot_getHistory);
	connect(this, &DbController::signal_getFileHistory, m_worker, &DbWorker::slot_getFileHistory);
	connect(this, &DbController::signal_getFileHistoryRecursive, m_worker, &DbWorker::slot_getFileHistoryRecursive);

	connect(this, &DbController::signal_getChangesetDetails, m_worker, &DbWorker::slot_getChangesetDetails);

	connect(this, &DbController::signal_addDeviceObject, m_worker, &DbWorker::slot_addDeviceObject);

	connect(this, &DbController::signal_getSignalsIDs, m_worker, &DbWorker::slot_getSignalsIDs);
	connect(this, &DbController::signal_getSignalsIDAppSignalID, m_worker, &DbWorker::slot_getSignalsIDAppSignalID);
	connect(this, &DbController::signal_getSignals, m_worker, &DbWorker::slot_getSignals);
	connect(this, &DbController::signal_getTunableSignals, m_worker, &DbWorker::slot_getTunableSignals);
	connect(this, &DbController::signal_getLatestSignal, m_worker, &DbWorker::slot_getLatestSignal);
	connect(this, &DbController::signal_getLatestSignals, m_worker, &DbWorker::slot_getLatestSignals);
	connect(this, &DbController::signal_getLatestSignalsByAppSignalIDs, m_worker, &DbWorker::slot_getLatestSignalsByAppSignalIDs);
	connect(this, &DbController::signal_getLatestSignalsWithUserID, m_worker, &DbWorker::slot_getLatestSignalsWithUserID);
	connect(this, &DbController::signal_getCheckedOutSignalsIDs, m_worker, &DbWorker::slot_getCheckedOutSignalsIDs);
	connect(this, &DbController::signal_addSignal, m_worker, &DbWorker::slot_addSignal);
	connect(this, &DbController::signal_checkoutSignals, m_worker, &DbWorker::slot_checkoutSignals);
	connect(this, &DbController::signal_setSignalWorkcopy, m_worker, &DbWorker::slot_setSignalWorkcopy);
	connect(this, &DbController::signal_setSignalsWorkcopies, m_worker, &DbWorker::slot_setSignalsWorkcopies);
	connect(this, &DbController::signal_deleteSignal, m_worker, &DbWorker::slot_deleteSignal);
	connect(this, &DbController::signal_undoSignalChanges, m_worker, &DbWorker::slot_undoSignalChanges);
	connect(this, &DbController::signal_undoSignalsChanges, m_worker, &DbWorker::slot_undoSignalsChanges);
	connect(this, &DbController::signal_checkinSignals, m_worker, &DbWorker::slot_checkinSignals);
	connect(this, &DbController::signal_autoAddSignals, m_worker, &DbWorker::slot_autoAddSignals);
	connect(this, &DbController::signal_autoDeleteSignals, m_worker, &DbWorker::slot_autoDeleteSignals);
	connect(this, &DbController::signal_getSignalsIDsWithAppSignalID, m_worker, &DbWorker::slot_getSignalsIDsWithAppSignalID);
	connect(this, &DbController::signal_getSignalsIDsWithCustomAppSignalID, m_worker, &DbWorker::slot_getSignalsIDsWithCustomAppSignalID);
	connect(this, &DbController::signal_getSignalsIDsWithEquipmentID, m_worker, &DbWorker::slot_getSignalsIDsWithEquipmentID);
	connect(this, &DbController::signal_getMultipleSignalsIDsWithEquipmentID, m_worker, &DbWorker::slot_getMultipleSignalsIDsWithEquipmentID);
	connect(this, &DbController::signal_getSignalHistory, m_worker, &DbWorker::slot_getSignalHistory);

	connect(this, static_cast<void(DbController::*)(const std::vector<int>*, int, std::vector<AppSignal>*)>(&DbController::signal_getSpecificSignals),
			m_worker, static_cast<void(DbWorker::*)(const std::vector<int>*, int, std::vector<AppSignal>*)>(&DbWorker::slot_getSpecificSignals));

	connect(this, static_cast<void(DbController::*)(int, std::vector<AppSignal>*)>(&DbController::signal_getSpecificSignals),
			m_worker, static_cast<void(DbWorker::*)(int, std::vector<AppSignal>*)>(&DbWorker::slot_getSpecificSignals));

	connect(this, static_cast<void(DbController::*)(QDateTime, std::vector<AppSignal>*)>(&DbController::signal_getSpecificSignals),
			m_worker, static_cast<void(DbWorker::*)(QDateTime, std::vector<AppSignal>*)>(&DbWorker::slot_getSpecificSignals));

	connect(this, &DbController::signal_hasCheckedOutSignals, m_worker, &DbWorker::slot_hasCheckedOutSignals);

	connect(this, &DbController::signal_buildStart, m_worker, &DbWorker::slot_buildStart);
	connect(this, &DbController::signal_buildFinish, m_worker, &DbWorker::slot_buildFinish);

	connect(this, &DbController::signal_isAnyCheckedOut, m_worker, &DbWorker::slot_isAnyCheckedOut);
	connect(this, &DbController::signal_lastChangesetId, m_worker, &DbWorker::slot_lastChangesetId);

	connect(this, &DbController::signal_nextCounterValue, m_worker, &DbWorker::slot_nextCounterValue);

	m_thread.start();
}

DbController::~DbController()
{
	if (isProjectOpened() == true)
	{
		closeProject(nullptr);
	}

	m_thread.quit();
	m_thread.wait();
}

void DbController::init()
{
	qRegisterMetaType<DbUser>();
	qRegisterMetaType<DbFileInfo>();
	qRegisterMetaType<DbFile>();
	qRegisterMetaType<DbChangeset>();
	qRegisterMetaType<DbChangesetDetails>();
	qRegisterMetaType<DbProject>();
	qRegisterMetaType<std::vector<DbProject>>();
	qRegisterMetaType<std::vector<DbFileInfo>>();
	qRegisterMetaType<std::vector<std::shared_ptr<DbFile>>>();

	return;
}

void DbController::shutdown()
{
	return;
}

bool DbController::getProjectList(std::vector<DbProject>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (out == nullptr)
	{
		assert(out != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getProjectList(out);

	bool result = waitForComplete(parentWidget, tr("Getting projects list"));
	return result;
}

bool DbController::createProject(QString projectName, QString administratorPassword, QWidget* parentWidget)
{
	// Check parameters
	//
	if (projectName.isEmpty())
	{
		assert(projectName.isEmpty() == false);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_createProject(projectName, administratorPassword);

	bool result = waitForComplete(parentWidget, tr("Creating new project"));
	return result;
}

bool DbController::openProject(QString projectName, QString username, QString password, QWidget* parentWidget)
{
	// Check parameters
	//
	if (projectName.isEmpty() || username.isEmpty() || password.isEmpty())
	{
		assert(projectName.isEmpty() == false);
		assert(username.isEmpty() == false);
		assert(password.isEmpty() == false);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_openProject(projectName, username, password);

	bool result = waitForComplete(parentWidget, tr("Opening project"));

	if (result == false)
	{
		return result;
	}

	// Get user list
	//
	std::vector<DbUser> users;

	result = getUserList(&users, parentWidget);
	if (result == false)
	{
		return result;
	}

	{
		QMutexLocker um(&m_userMutex);

		m_users.clear();
		for (const DbUser& u : users)
		{
			m_users[u.userId()] = u;
		}
	}

	if (result == true)
	{
		emit projectOpened(m_worker->currentProject());
	}

	return result;
}

bool DbController::closeProject(QWidget* parentWidget)
{
	// Check parameters
	//

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_closeProject();

	bool result = waitForComplete(parentWidget, tr("Closing project"));

	if (result == true)
	{
		emit projectClosed();
	}

	return result;
}

bool DbController::cloneProject(QString projectName, QString password, QString newProjectName, QWidget* parentWidget)
{
	// Check parameters
	//
	if (projectName.isEmpty() || password.isEmpty())
	{
		assert(projectName.isEmpty() == false);
		assert(password.isEmpty() == false);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_cloneProject(projectName, password, newProjectName);

	bool result = waitForComplete(parentWidget, tr("Cloning project"));
	return result;
}

bool DbController::deleteProject(QString projectName, QString password, bool doNotBackup, QWidget* parentWidget)
{
	// Check parameters
	//
	if (projectName.isEmpty() || password.isEmpty())
	{
		assert(projectName.isEmpty() == false);
		assert(password.isEmpty() == false);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_deleteProject(projectName, password, doNotBackup);

	bool result = waitForComplete(parentWidget, tr("Deleting project"));
	return result;
}

bool DbController::upgradeProject(QString projectName, QString password, bool doNotBackup, QWidget* parentWidget)
{
	// Check parameters
	//
	if (projectName.isEmpty() || password.isEmpty())
	{
		assert(projectName.isEmpty() == false);
		assert(password.isEmpty() == false);
		return false;
	}


	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_upgradeProject(projectName, password, doNotBackup);

	bool result = waitForComplete(parentWidget, tr("Upgrading project"));
	return result;
}

bool DbController::setProjectProperty(QString propertyName, bool propertyValue, QWidget* parentWidget)
{
	return setProjectProperty(propertyName, propertyValue ? QString("true") : QString("false") , parentWidget);
}

bool DbController::setProjectProperty(QString propertyName, int propertyValue, QWidget* parentWidget)
{
	return setProjectProperty(propertyName, QString::number(propertyValue, 10), parentWidget);
}

bool DbController::setProjectProperty(QString propertyName, QString propertyValue, QWidget* parentWidget)
{
	// Check parameters
	//
	if (propertyName.isEmpty() == true)
	{
		assert(propertyName.isEmpty() == false);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_setProjectProperty(propertyName, propertyValue);

	bool result = waitForComplete(parentWidget, tr("Setting project property"));
	return result;
}

bool DbController::getProjectProperty(QString propertyName, bool* out, QWidget* parentWidget)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	QString propValueStr;

	bool ok = getProjectProperty(propertyName, &propValueStr, parentWidget);
	if (ok == false)
	{
		return false;
	}

	*out = propValueStr.compare("true", Qt::CaseInsensitive) == 0 ? true : false;

	return true;
}

bool DbController::getProjectProperty(QString propertyName, int* out, QWidget* parentWidget)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	QString propValueStr;

	bool ok = getProjectProperty(propertyName, &propValueStr, parentWidget);
	if (ok == false)
	{
		return false;
	}

	*out = propValueStr.toInt(&ok, 10);

	return ok;
}

bool DbController::getProjectProperty(QString propertyName, QString* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (propertyName.isEmpty() == true ||
		out == nullptr)
	{
		assert(propertyName.isEmpty() == false);
		assert(out);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getProjectProperty(propertyName, out);

	bool result = waitForComplete(parentWidget, tr("Getting project property"));
	return result;
}

bool DbController::getProjectProperties(DbProjectProperties* out, QWidget* parentWidget)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	if (isProjectOpened() == false)
	{
		return false;
	}

	bool ok = true;

	QString description;
	bool safetyProject = true;
	QString suppressWarningsStr;
	bool uppercaseAppSignalId = true;
	bool generateAppSignalXml = false;
	bool generateAppLogicDrawings = false;
	bool generateExtarDebugInfo = false;

	bool runSimTestsOnBuild = true;
	int simTestsTimeout = true;

	ok &= getProjectProperty(Db::ProjectProperty::Description, &description, parentWidget);
	ok &= getProjectProperty(Db::ProjectProperty::SafetyProject, &safetyProject, parentWidget);

	ok &= getProjectProperty(Db::ProjectProperty::SuppressWarnings, &suppressWarningsStr, parentWidget);
	ok &= getProjectProperty(Db::ProjectProperty::UppercaseAppSignalId, &uppercaseAppSignalId, parentWidget);
	ok &= getProjectProperty(Db::ProjectProperty::GenerateAppSignalsXml, &generateAppSignalXml, parentWidget);
	ok &= getProjectProperty(Db::ProjectProperty::GenerateAppLogicDrawings, &generateAppLogicDrawings, parentWidget);
	ok &= getProjectProperty(Db::ProjectProperty::GenerateExtraDebugInfo, &generateExtarDebugInfo, parentWidget);

	ok &= getProjectProperty(Db::ProjectProperty::RunSimTestsOnBuild, &runSimTestsOnBuild, parentWidget);
	ok &= getProjectProperty(Db::ProjectProperty::SimulatorTestsTimeout, &simTestsTimeout, parentWidget);

	// --
	//
	if (ok == false)
	{
		return false;
	}

	out->setDescription(description);
	out->setSafetyProject(safetyProject);
	out->setSuppressWarnings(suppressWarningsStr);
	out->setUppercaseAppSignalId(uppercaseAppSignalId);
	out->setGenerateAppSignalsXml(generateAppSignalXml);
	out->setGenerateAppLogicDrawings(generateAppLogicDrawings);
	out->setGenerateExtraDebugInfo(generateExtarDebugInfo);

	out->setRunSimTestsOnBuild(runSimTestsOnBuild);
	out->setSimTestsTimeout(simTestsTimeout);

	return true;
}

bool DbController::setProjectProperties(const DbProjectProperties& in, QWidget* parentWidget)
{
	if (isProjectOpened() == false)
	{
		return false;
	}

	bool ok = true;

	ok &= setProjectProperty(Db::ProjectProperty::Description, in.description(), parentWidget);
	ok &= setProjectProperty(Db::ProjectProperty::SafetyProject, in.safetyProject(), parentWidget);
	ok &= setProjectProperty(Db::ProjectProperty::SuppressWarnings, in.suppressWarningsAsString(), parentWidget);
	ok &= setProjectProperty(Db::ProjectProperty::UppercaseAppSignalId, in.uppercaseAppSignalId(), parentWidget);
	ok &= setProjectProperty(Db::ProjectProperty::GenerateAppSignalsXml, in.generateAppSignalsXml(), parentWidget);
	ok &= setProjectProperty(Db::ProjectProperty::GenerateAppLogicDrawings, in.generateAppLogicDrawings(), parentWidget);
	ok &= setProjectProperty(Db::ProjectProperty::GenerateExtraDebugInfo, in.generateExtraDebugInfo(), parentWidget);

	ok &= setProjectProperty(Db::ProjectProperty::RunSimTestsOnBuild, in.runSimTestsOnBuild(), parentWidget);
	ok &= setProjectProperty(Db::ProjectProperty::SimulatorTestsTimeout, in.simTestsTimeout(), parentWidget);

	return ok;
}

bool DbController::createUser(const DbUser& user, QWidget* parentWidget)
{
	// Check parameters
	//
	if (user.username().isEmpty() == true)
	{
		assert(user.username().isEmpty() == false);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_createUser(user);

	bool result = waitForComplete(parentWidget, tr("Creating user"));
	return result;
}

bool DbController::updateUser(const DbUser& user, QWidget* parentWidget)
{
	// Check parameters
	//
	if (user.username().isEmpty() == true)
	{
		assert(user.username().isEmpty() == false);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_updateUser(user);

	bool result = waitForComplete(parentWidget, tr("Updating user profile"));
	return result;
}

bool DbController::setUserProperty(const QString& property, const QString& value, QWidget* parentWidget)
{
	if (property.isEmpty() == true)
	{
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_setUserProperty(property, value);

	bool result = waitForComplete(parentWidget, tr("Setting user property"));
	return result;
}

bool DbController::getUserProperty(const QString& property, QString* value, QWidget* parentWidget)
{
	if (property.isEmpty() == true || value == nullptr)
	{
		assert(value);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getUserProperty(property, value);

	bool result = waitForComplete(parentWidget, tr("Getting user property"));
	return result;
}

bool DbController::getUserProperty(const QString& property, QString* value, const QString& defaultValue, QWidget* parentWidget)
{
	bool ok = getUserProperty(property, value, parentWidget);
	if (ok == false)
	{
		*value = defaultValue;
	}

	// Return false even in case of setting default value
	//
	return ok;
}

bool DbController::getUserPropertyList(QString propertyTemplate, QStringList* out, QWidget* parentWidget)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	if (propertyTemplate.isEmpty() == true)
	{
		propertyTemplate = "%";
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getUserPropertyList(propertyTemplate, out);

	bool result = waitForComplete(parentWidget, tr("Getting user property list"));
	return result;
}

bool DbController::removeUserProperty(const QString& property, QWidget* parentWidget)
{
	if (property.isEmpty() == true)
	{
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_removeUserProperty(property);

	bool result = waitForComplete(parentWidget, tr("Removing user property"));
	return result;
}

bool DbController::isFileExists(QString fileName, int parentId, int* fileId, QWidget* parentWidget)
{
	// Check parameters
	//
	if (fileId == nullptr)
	{
		assert(fileId != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_isFileExists(fileName, parentId, fileId);

	bool result = waitForComplete(parentWidget, tr("Getting file information..."));
	return result;
}

bool DbController::getFileList(std::vector<DbFileInfo>* files, int parentId, bool removeDeleted, QWidget* parentWidget)
{
	return getFileList(files, parentId, QString(), removeDeleted, parentWidget);
}

bool DbController::getFileList(std::vector<DbFileInfo>* files, DbDir systemDir, bool removeDeleted, QWidget* parentWidget)
{
	return getFileList(files, systemFileId(systemDir), removeDeleted, parentWidget);
}

bool DbController::getFileList(std::vector<DbFileInfo>* files, int parentId, QString filter, bool removeDeleted, QWidget* parentWidget)
{
	// Check parameters
	//
	if (files == nullptr)
	{
		assert(files != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getFileList(files, parentId, filter, removeDeleted);

	bool result = waitForComplete(parentWidget, tr("Geting file list"));
	return result;
}

bool DbController::getFileList(std::vector<DbFileInfo>* files, DbDir systemDir, QString filter, bool removeDeleted, QWidget* parentWidget)
{
	int parentFileId = systemFileId(systemDir);
	return getFileList(files, parentFileId, filter, removeDeleted, parentWidget);
}

bool DbController::getFileListTree(DbFileTree* filesTree, int parentId, bool removeDeleted, QWidget* parentWidget)
{
	return getFileListTree(filesTree, parentId, QString{}, removeDeleted, parentWidget);
}

bool DbController::getFileListTree(DbFileTree* filesTree, DbDir parentSystemDir, bool removeDeleted, QWidget* parentWidget)
{
	return getFileListTree(filesTree, systemFileId(parentSystemDir), removeDeleted, parentWidget);
}

bool DbController::getFileListTree(DbFileTree* filesTree, int parentId, QString filter, bool removeDeleted, QWidget* parentWidget)
{
	// Check parameters
	//
	if (filesTree == nullptr)
	{
		assert(filesTree != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getFileListTree(filesTree, parentId, filter, removeDeleted);

	bool result = waitForComplete(parentWidget, tr("Geting file list tree"));
	return result;
}

bool DbController::getFileListTree(DbFileTree* filesTree, DbDir parentSystemDir, QString filter, bool removeDeleted, QWidget* parentWidget)
{
	return getFileListTree(filesTree, systemFileId(parentSystemDir), filter, removeDeleted, parentWidget);
}

bool DbController::getFileInfo(int parentId, QString fileName, DbFileInfo* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (out == nullptr)
	{
		assert(out != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getFileInfo(parentId, fileName, out);

	bool result = waitForComplete(parentWidget, tr("Geting file info"));
	return result;
}

bool DbController::getFileInfo(int fileId, DbFileInfo* out, QWidget* parentWidget)
{
	if (out == nullptr)
	{
		assert(out != nullptr);
		return false;
	}

	std::vector<int> fileIds;
	fileIds.push_back(fileId);

	std::vector<DbFileInfo> proxyOut;

	bool result = getFileInfo(&fileIds, &proxyOut, parentWidget);

	if (result == false || proxyOut.size() != 1)
	{
		return false;
	}

	assert(fileId == proxyOut[0].fileId());

	*out = proxyOut[0];

	return result;
}

bool DbController::getFileInfo(std::vector<int>* fileIds, std::vector<DbFileInfo>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (fileIds == nullptr ||
		fileIds->empty() == true ||
		out == nullptr)
	{
		assert(fileIds != nullptr);
		assert(out != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getFilesInfo(fileIds, out);

	bool result = waitForComplete(parentWidget, tr("Geting files info"));
	return result;
}

bool DbController::getFileInfo(QString fullPathFileName, DbFileInfo* out, QWidget* parentWidget)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	std::vector<QString> fullPathFileNames;
	fullPathFileNames.push_back(fullPathFileName);

	std::vector<DbFileInfo> outs;

	bool ok = getFileInfo(&fullPathFileNames, &outs, parentWidget);

	if (ok == true )
	{
		assert(outs.size() == 1);

		if (outs.size() == 1)
		{
			*out = outs[0];
		}
	}

	return ok;
}

bool DbController::getFileInfo(const std::vector<QString>* fullPathFileNames, std::vector<DbFileInfo>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (fullPathFileNames == nullptr ||
		fullPathFileNames->empty() == true ||
		out == nullptr)
	{
		assert(fullPathFileNames);
		assert(fullPathFileNames->empty() == false);
		assert(out != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getFullPathFilesInfo(fullPathFileNames, out);

	bool result = waitForComplete(parentWidget, tr("Geting files info"));
	return result;
}

bool DbController::addFiles(std::vector<std::shared_ptr<DbFile>>* files,
							int parentId,
							bool ensureUniquesInParentTree,
							int uniqueFromFileId,
							QWidget* parentWidget)
{
	// Check parameters
	//
	if (files == nullptr)
	{
		assert(files != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_addFiles(files, parentId, ensureUniquesInParentTree, uniqueFromFileId);

	bool result = waitForComplete(parentWidget, tr("Adding files"));
	return result;
}

bool DbController::addFiles(std::vector<std::shared_ptr<DbFile>>* files, int parentId, QWidget* parentWidget)
{
	return addFiles(files, parentId, false, -1, parentWidget);
}

bool DbController::addFile(const std::shared_ptr<DbFile>& file, int parentId, QWidget* parentWidget)
{
	std::vector<std::shared_ptr<DbFile>> v;
	v.push_back(file);

	return addFiles(&v, parentId, false, -1, parentWidget);
}

bool DbController::addFile(const std::shared_ptr<DbFile>& file, DbDir systemDir, QWidget* parentWidget)
{
	return addFile(file, systemFileId(systemDir), parentWidget);
}

bool DbController::addUniqueFile(const std::shared_ptr<DbFile>& file, int parentId, int uniqueFromFileId, QWidget* parentWidget)
{
	std::vector<std::shared_ptr<DbFile>> v;
	v.push_back(file);

	return addFiles(&v, parentId, true, uniqueFromFileId, parentWidget);
}

bool DbController::deleteFiles(std::vector<std::shared_ptr<DbFileInfo>>* files, QWidget* parentWidget)
{
	if (files == nullptr)
	{
		assert(files);
		return false;
	}

	std::vector<DbFileInfo> v;
	v.reserve(files->size());

	for (auto& f : *files)
	{
		v.push_back(*(f.get()));
	}

	bool result = deleteFiles(&v, parentWidget);

	if (result == true)
	{
		// set file state to the input array
		//
		assert(v.size() == files->size());

		for (size_t i = 0; i < files->size(); i++)
		{
			// FileID can be different, as for permanently deleted files it is marked as -1
			// and, we cannot rely  on the file order
			//
			auto& f = files->operator [](i);

			auto findInResult = std::find_if(v.begin(), v.end(),
				[&f](const DbFileInfo& vf)
				{
					return vf.fileName() == f->fileName() && vf.parentId() == f->parentId();
				});

			if (findInResult == v.end())
			{
				assert(false);			// file not found here
				continue;
			}
			else
			{
				*(f.get()) = *findInResult;
			}
		}
	}

	return result;
}

bool DbController::deleteFiles(std::vector<DbFileInfo>* files, QWidget* parentWidget)
{
	// Check parameters
	//
	if (files == nullptr)
	{
		assert(files != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_deleteFiles(files);

	bool result = waitForComplete(parentWidget, tr("Deleting files"));
	return result;
}

bool DbController::moveFiles(const std::vector<std::shared_ptr<DbFileInfo>>& files,
							 int moveToParentId,
							 std::vector<DbFileInfo>* movedFiles,
							 QWidget* parentWidget)
{
	if (movedFiles == nullptr)
	{
		assert(movedFiles);
		return false;
	}

	std::vector<DbFileInfo> v;
	v.reserve(files.size());

	for (auto& f : files)
	{
		v.push_back(*(f.get()));
	}

	bool result = moveFiles(v, moveToParentId, movedFiles, parentWidget);
	return result;
}

bool DbController::moveFiles(const std::vector<DbFileInfo>& files, int moveToParentId, std::vector<DbFileInfo>* movedFiles, QWidget* parentWidget)
{
	// Check parameters
	//
	if (movedFiles == nullptr)
	{
		assert(movedFiles != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_moveFiles(&files, moveToParentId, movedFiles);

	bool result = waitForComplete(parentWidget, tr("Moving files"));
	return result;
}

bool DbController::renameFile(const DbFileInfo& file, QString newFileName, DbFileInfo* updatedFileInfo, QWidget* parentWidget)
{
	// Check parameters
	//
	if (updatedFileInfo == nullptr ||
		file.isNull() == true ||
		newFileName.isEmpty() == true)
	{
		assert(updatedFileInfo != nullptr);
		assert(file.isNull() == false);
		assert(newFileName.isEmpty() == false);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_renameFile(file, newFileName, updatedFileInfo);

	bool result = waitForComplete(parentWidget, tr("Renaming file"));
	return result;
}

bool DbController::getLatestVersion(const std::vector<DbFileInfo>& files,
					  std::vector<std::shared_ptr<DbFile>>* out,
					  QWidget* parentWidget)
{
	// Check parameters
	//
	if (out == nullptr || files.empty() == true)
	{
		assert(out != nullptr);
		assert(files.empty() == true);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getLatestVersion(&files, out);

	ok = waitForComplete(parentWidget, tr("Getting file"));
	return out;
}

bool DbController::getLatestVersion(const DbFileInfo& file, std::shared_ptr<DbFile>* out, QWidget* parentWidget)
{
	if (out == nullptr)
	{
		assert(out != nullptr);
		return false;
	}

	std::vector<DbFileInfo> fiv;
	fiv.push_back(file);

	std::vector<std::shared_ptr<DbFile>> outvector;
	bool result = getLatestVersion(fiv, &outvector, parentWidget);

	if (result == false || outvector.size() != 1)
	{
		return false;
	}

	*out = outvector.front();
	return true;
}

bool DbController::getLatestTreeVersion(const DbFileInfo& file, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (out == nullptr || file.fileId() == -1)
	{
		assert(out != nullptr);
		assert(file.fileId() != -1);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getLatestTreeVersion(file, out);

	ok = waitForComplete(parentWidget, tr("Getting files"));
	return out;
}

bool DbController::getCheckedOutFiles(const DbFileInfo& parentFile, std::vector<DbFileInfo>* out, QWidget* parentWidget)
{
	std::vector<DbFileInfo> fv = {parentFile};
	return getCheckedOutFiles(&fv, out, parentWidget);
}

bool DbController::getCheckedOutFiles(const std::vector<DbFileInfo>* parentFiles, std::vector<DbFileInfo>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (parentFiles == nullptr ||
		parentFiles->empty() == true
		)
	{
		assert(parentFiles);
		assert(parentFiles->empty() == false);
	}

	if (out == nullptr)
	{
		assert(out != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getCheckedOutFiles(parentFiles, out);

	ok = waitForComplete(parentWidget, tr("Getting checked out files"));
	return out;
}

bool DbController::getWorkcopy(const std::vector<DbFileInfo>& files,
							   std::vector<std::shared_ptr<DbFile>>* out,
							   QWidget* parentWidget)
{
	// Check parameters
	//
	if (out == nullptr || files.empty() == true)
	{
		assert(out != nullptr);
		assert(files.empty() == true);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getWorkcopy(&files, out);

	ok = waitForComplete(parentWidget, tr("Getting file workcopy"));
	return out;
}

bool DbController::getWorkcopy(const DbFileInfo& file, std::shared_ptr<DbFile>* out, QWidget* parentWidget)
{
	if (out == nullptr)
	{
		assert(out != nullptr);
		return false;
	}

	std::vector<DbFileInfo> fiv;
	fiv.push_back(file);

	std::vector<std::shared_ptr<DbFile>> outvector;
	bool result = getWorkcopy(fiv, &outvector, parentWidget);

	if (result == false || outvector.size() != 1)
	{
		return false;
	}

	*out = outvector.front();
	return true;
}

bool DbController::setWorkcopy(const std::vector<std::shared_ptr<DbFile>>& files, QWidget* parentWidget)
{
	// Check parameters
	//
	if (files.empty() == true)
	{
		assert(files.empty() == true);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_setWorkcopy(&files);

	ok = waitForComplete(parentWidget, tr("Setting file workcopy"));
	return true;
}

bool DbController::setWorkcopy(const std::shared_ptr<DbFile>& file, QWidget* parentWidget)
{
	std::vector<std::shared_ptr<DbFile>> files;
	files.push_back(file);

	return setWorkcopy(files, parentWidget);
}

bool DbController::getSpecificCopy(const std::vector<DbFileInfo>& files, int changesetId, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (out == nullptr || files.empty() == true)
	{
		assert(out != nullptr);
		assert(files.empty() == true);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getSpecificCopy(&files, changesetId, out);

	ok = waitForComplete(parentWidget, tr("Getting file copy"));
	return out;
}

bool DbController::getSpecificCopy(const DbFileInfo& file, int changesetId, std::shared_ptr<DbFile>* out, QWidget* parentWidget)
{
	if (out == nullptr)
	{
		assert(out != nullptr);
		return false;
	}

	std::vector<DbFileInfo> fiv;
	fiv.push_back(file);

	std::vector<std::shared_ptr<DbFile>> outvector;
	bool result = getSpecificCopy(fiv, changesetId, &outvector, parentWidget);

	if (result == false || outvector.size() != 1)
	{
		return false;
	}

	*out = outvector.front();
	return true;
}

bool DbController::getSpecificCopy(const std::vector<DbFileInfo>& files, QDateTime date, std::vector<std::shared_ptr<DbFile>>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (out == nullptr || files.empty() == true)
	{
		assert(out != nullptr);
		assert(files.empty() == true);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getSpecificCopy(&files, date, out);

	ok = waitForComplete(parentWidget, tr("Getting file copy"));
	return out;
}

bool DbController::getSpecificCopy(const DbFileInfo& file, QDateTime date, std::shared_ptr<DbFile>* out, QWidget* parentWidget)
{
	if (out == nullptr)
	{
		assert(out != nullptr);
		return false;
	}

	std::vector<DbFileInfo> fiv;
	fiv.push_back(file);

	std::vector<std::shared_ptr<DbFile>> outvector;
	bool result = getSpecificCopy(fiv, date, &outvector, parentWidget);

	if (result == false || outvector.size() != 1)
	{
		return false;
	}

	*out = outvector.front();
	return true;
}

bool DbController::checkIn(DbFileInfo& file, const QString& comment, QWidget* parentWidget)
{
	std::vector<DbFileInfo> fv {file};

	bool ok = checkIn(fv, comment, parentWidget);

	file = fv.front();

	return ok;
}

bool DbController::checkIn(std::vector<DbFileInfo>& files, const QString& comment, QWidget* parentWidget)
{
	// Check parameters
	//
	if (files.empty() == true)
	{
		assert(files.empty() == true);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	QString trimmedCommnet = comment.trimmed();

	emit signal_checkIn(&files, trimmedCommnet);

	ok = waitForComplete(parentWidget, tr("Checking in files"));
	return true;
}

bool DbController::checkInTree(std::vector<DbFileInfo>& parentFiles, std::vector<DbFileInfo>* outCheckedIn, const QString& comment, QWidget* parentWidget)
{
	// Check parameters
	//
	if (parentFiles.empty() == true ||
		outCheckedIn == nullptr)
	{
		assert(parentFiles.empty() == true);
		assert(outCheckedIn != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	QString trimmedCommnet = comment.trimmed();

	emit signal_checkInTree(&parentFiles, outCheckedIn, trimmedCommnet);

	ok = waitForComplete(parentWidget, tr("Checking in files"));
	return true;
}

bool DbController::checkOut(DbFileInfo& file, QWidget* parentWidget)
{
	std::vector<DbFileInfo> fv {file};

	bool ok = checkOut(fv, parentWidget);

	file = fv.front();

	return ok;
}

bool DbController::checkOut(std::vector<DbFileInfo>& files, QWidget* parentWidget)
{
	// Check parameters
	//
	if (files.empty() == true)
	{
		assert(files.empty() == true);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_checkOut(&files);

	ok = waitForComplete(parentWidget, tr("Checking out files"));
	return true;
}


bool DbController::undoChanges(DbFileInfo& file, QWidget* parentWidget)
{
	std::vector<DbFileInfo> fv {file};

	bool ok = undoChanges(fv, parentWidget);

	file = fv.front();

	return ok;
}

bool DbController::undoChanges(std::vector<DbFileInfo>& files, QWidget* parentWidget)
{
	// Check parameters
	//
	if (files.empty() == true)
	{
		assert(files.empty() == true);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_undoChanges(&files);

	ok = waitForComplete(parentWidget, tr("Undo pending changes"));
	return true;
}

bool DbController::fileHasChildren(bool* hasChildren, DbFileInfo& file, QWidget* parentWidget)
{
	// Check parameters
	//
	if (hasChildren == nullptr)
	{
		assert(hasChildren != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_fileHasChildren(hasChildren, &file);

	ok = waitForComplete(parentWidget, tr("Checking file children"));
	return true;
}

bool DbController::getProjectHistory(std::vector<DbChangeset>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (out == nullptr)
	{
		assert(out != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getHistory(out);

	ok = waitForComplete(parentWidget, tr("Getting project history"));
	return true;
}

bool DbController::getFileHistory(const DbFileInfo& file, std::vector<DbChangeset>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (file.fileId() == -1 || out == nullptr)
	{
		assert(file.fileId() != -1);
		assert(out != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getFileHistory(file, out);

	ok = waitForComplete(parentWidget, tr("Getting file history"));
	return true;
}

bool DbController::getFileHistoryRecursive(const DbFileInfo& parentFile, std::vector<DbChangeset>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (parentFile.fileId() == -1 || out == nullptr)
	{
		assert(parentFile.fileId() != -1);
		assert(out != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getFileHistoryRecursive(parentFile, out);

	ok = waitForComplete(parentWidget, tr("Getting file history"));
	return true;
}

bool DbController::getChangesetDetails(int changeset, DbChangesetDetails* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getChangesetDetails(changeset, out);

	ok = waitForComplete(parentWidget, tr("Getting changeset %1 details").arg(changeset));
	return true;
}

bool DbController::addDeviceObject(Hardware::DeviceObject* device, int parentId, QWidget* parentWidget)
{
	// Check parameters
	//
	if (device == nullptr)
	{
		assert(device != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_addDeviceObject(device, parentId);

	ok = waitForComplete(parentWidget, tr("Adding device object"));


	return ok;
}

bool DbController::deleteDeviceObjects(std::vector<Hardware::DeviceObject*>& devices, QWidget* parentWidget)
{
	if (devices.empty() == true)
	{
		assert(devices.empty() == false);
		return false;
	}

	std::vector<DbFileInfo> files;

	for (auto& d : devices)
	{
		const DbFileInfo* fi = d->data();
		Q_ASSERT(fi);

		files.push_back(*fi);
	}

	bool ok = deleteFiles(&files, parentWidget);
	if (ok == false)
	{
		return false;
	}

	// During delete file output can be in differetn order, so sort it in DESCENDING order, assume that files
	// already sorted in desc order in DbWorker::slot_deleteFiles, but sort it again just in case
	//
	assert(devices.size() == files.size());

	std::sort(devices.begin(), devices.end(),
		[](const Hardware::DeviceObject* d1, const Hardware::DeviceObject* d2)
		{
			const DbFileInfo* f1 = d1->data();
			const DbFileInfo* f2 = d2->data();

			Q_ASSERT(f1);
			Q_ASSERT(f2);

			return f1->fileId() >= f2->fileId();
		});

	std::sort(files.begin(), files.end(),
		[](const DbFileInfo& f1, const DbFileInfo& f2)
		{
			return f1.fileId() >= f2.fileId();
		});

	for (size_t i = 0; i < devices.size(); i++)
	{
		Hardware::DeviceObject* deviceObject = devices[i];

		const DbFileInfo* fi = deviceObject->data();
		Q_ASSERT(fi);

		Q_ASSERT(fi->fileId() == files[i].fileId());

		auto fileInfo = std::make_shared<DbFileInfo>(files[i], deviceObject->details());
		deviceObject->setData(fileInfo);
	}

	return true;
}

bool DbController::getDeviceTreeLatestVersion(const DbFileInfo& file, std::shared_ptr<Hardware::DeviceObject>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (file.fileId() == -1 || out == nullptr)
	{
		assert(file.fileId() >= 0);
		assert(out != nullptr);
		return false;
	}

	out->reset();

	// 1. Get files
	//

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	std::vector<std::shared_ptr<DbFile>> files;

	emit signal_getLatestTreeVersion(file, &files);

	ok = waitForComplete(parentWidget, tr("Getting device"));

	if (ok == false)
	{
		return false;
	}

	// 2.1 read all items
	//
	std::map<int, std::shared_ptr<Hardware::DeviceObject>> objectsMap;	// key is fileId

	std::list<QFuture<std::vector<std::shared_ptr<Hardware::DeviceObject>>>> threads;

	const size_t fileCountPerThread = 2048;
	std::vector<std::shared_ptr<DbFile>> threadFiles;
	threadFiles.reserve(fileCountPerThread);

	int hcFileId = systemFileId(DbDir::HardwareConfigurationDir);
	int hpFileId = systemFileId(DbDir::HardwarePresetsDir);

	for (const std::shared_ptr<DbFile>& f : files)
	{
		if (f->fileId() == hcFileId || f->fileId() == hpFileId)
		{
			std::shared_ptr<Hardware::DeviceObject> object = std::make_shared<Hardware::DeviceRoot>();

			auto fio = std::make_shared<DbFileInfo>(*f.get(), object->details());
			object->setData(fio);

			objectsMap[fio->fileId()] = object;
		}
		else
		{
			threadFiles.push_back(f);

			if (threadFiles.size() >= fileCountPerThread)
			{
				QFuture<std::vector<std::shared_ptr<Hardware::DeviceObject>>> thread =
						QtConcurrent::run(DbWorker::deviceObjectFromDbFiles, threadFiles);

				threads.push_back(thread);

				threadFiles.clear();
			}
		}
	}

	if (threadFiles.empty() == false)
	{
		QFuture<std::vector<std::shared_ptr<Hardware::DeviceObject>>> thread = QtConcurrent::run(DbWorker::deviceObjectFromDbFiles, threadFiles);
		threads.push_back(thread);

		threadFiles.clear();
	}

	for (QFuture<std::vector<std::shared_ptr<Hardware::DeviceObject>>>& future : threads)
	{
		std::vector<std::shared_ptr<Hardware::DeviceObject>> v = future.result();

		for (std::shared_ptr<Hardware::DeviceObject>& object : v)
		{
			const DbFileInfo* fio = object->data();
			Q_ASSERT(fio);

			objectsMap[fio->fileId()] = object;
		}
	}

	assert(objectsMap.size() == files.size());

	// 2.2 Set child to items
	//
	for (auto [fileId, deviceObject] : objectsMap)
	{
		// Get parentId
		//
		const DbFileInfo* fio = deviceObject->data();

		Q_ASSERT(fio);
		Q_ASSERT(fileId == fio->fileId());

		int parentId = fio->parentId();

		auto parentIterator = objectsMap.find(parentId);
		if (parentIterator == objectsMap.end())
		{
			// Apparently it is the root item, so, we have to check it and set flag that we already found it
			//
			assert(out->get() == nullptr);
			assert(file.fileId() == fileId);

			*out = deviceObject;
			continue;
		}

		(*parentIterator).second->addChild(deviceObject);
	}

	if (out->get() == nullptr)
	{
		assert(out->get());
		return false;
	}

	return ok;
}

bool DbController::getSignalsIDs(QVector<int>* signalIDs, QWidget* parentWidget)
{
	if (signalIDs == nullptr)
	{
		Q_ASSERT(signalIDs);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getSignalsIDs(signalIDs);

	ok = waitForComplete(parentWidget, tr("Getting signals' IDs"));

	return ok;
}

bool DbController::getSignalsIDAppSignalID(QVector<ID_AppSignalID>* signalsIDAppSignalID, QWidget* parentWidget)
{
	if (signalsIDAppSignalID == nullptr)
	{
		Q_ASSERT(signalsIDAppSignalID);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getSignalsIDAppSignalID(signalsIDAppSignalID);

	ok = waitForComplete(parentWidget, tr("Getting signals' IDs"));

	return ok;
}

bool DbController::getSignals(AppSignalSet* signalSet, bool excludeDeleted, QWidget* parentWidget)
{
	if (signalSet == nullptr)
	{
		assert(signalSet != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getSignals(signalSet, excludeDeleted);

	ok = waitForComplete(parentWidget, tr("Reading signals"));

	return ok;
}

bool DbController::getTunableSignals(AppSignalSet* signalSet, QWidget* parentWidget)
{
	if (signalSet == nullptr)
	{
		assert(signalSet != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getTunableSignals(signalSet);

	ok = waitForComplete(parentWidget, tr("Reading tunable signals"));

	return ok;
}

bool DbController::getLatestSignal(int signalID, AppSignal* signal, QWidget* parentWidget)
{
	if (signal == nullptr)
	{
		assert(signal != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getLatestSignal(signalID, signal);

	ok = waitForComplete(parentWidget, tr("Reading latest signal"));

	return ok;
}

bool DbController::getLatestSignals(QVector<int> signalIDs, QVector<AppSignal>* signalsArray, QWidget* parentWidget)
{
	if (signalsArray == nullptr)
	{
		assert(signalsArray != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getLatestSignals(signalIDs, signalsArray);

	ok = waitForComplete(parentWidget, tr("Reading latest signals"));

	return ok;
}


bool DbController::getLatestSignalsByAppSignalIDs(QStringList appSignalIDs, QVector<AppSignal>* signalArray, QWidget* parentWidget)
{
	if (signalArray == nullptr)
	{
		assert(signalArray != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getLatestSignalsByAppSignalIDs(appSignalIDs, signalArray);

	ok = waitForComplete(parentWidget, tr("Reading latest signals"));

	return ok;

}

bool DbController::getCheckedOutSignalsIDs(QVector<int> *signalIDs, QWidget* parentWidget)
{
	if (signalIDs == nullptr)
	{
		assert(signalIDs != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getCheckedOutSignalsIDs(signalIDs);

	ok = waitForComplete(parentWidget, tr("Getting checked out signals' IDs"));

	return ok;
}

bool DbController::addSignal(E::SignalType signalType, QVector<AppSignal>* newSignal, QWidget* parentWidget)
{
	if (newSignal == nullptr)
	{
		assert(newSignal != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_addSignal(signalType, newSignal);

	ok = waitForComplete(parentWidget, tr("Adding signals"));

	return ok;
}

bool DbController::getLatestSignalsWithoutProgress(QVector<int> signalIDs, QVector<AppSignal>* signalsArray, QWidget* parentWidget)
{
	if (signalsArray == nullptr)
	{
		assert(signalsArray != nullptr);
		return false;
	}

	if (signalIDs.size() > 250)
	{
		assert(false);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	bool progressIsEnabled = isProgressEnabled();

	disableProgress();

	emit signal_getLatestSignals(signalIDs, signalsArray);

	ok = waitForComplete(parentWidget, tr("Reading latest signals"));

	if (progressIsEnabled == true)
	{
		enableProgress();
	}

	return ok;
}

bool DbController::getLatestSignalsWithUserID(std::vector<AppSignal>* out, QWidget* parentWidget)
{
	if (out == nullptr)
	{
		Q_ASSERT(out);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getLatestSignalsWithUserID(out);

	ok = waitForComplete(parentWidget, tr("Reading latest signals with UserID"));

	return ok;

}



bool DbController::checkoutSignals(QVector<int>* signalIDs, QVector<ObjectState>* objectStates, QWidget* parentWidget)
{
	if (signalIDs == nullptr)
	{
		assert(signalIDs != nullptr);
		return false;
	}

	if (objectStates == nullptr)
	{
		assert(objectStates != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_checkoutSignals(signalIDs, objectStates);

	ok = waitForComplete(parentWidget, tr("Checkout signals"));

	return ok;
}


bool DbController::setSignalWorkcopy(AppSignal *signal, ObjectState *objectState, QWidget* parentWidget)
{
	if (signal == nullptr ||
		objectState == nullptr)
	{
		Q_ASSERT(signal);
		Q_ASSERT(objectState);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_setSignalWorkcopy(signal, objectState);

	ok = waitForComplete(parentWidget, tr("Set signal workcopy"));

	return ok;
}

bool DbController::setSignalsWorkcopies(const QVector<AppSignal>* signalsList, QWidget* parentWidget)
{
	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_setSignalsWorkcopies(signalsList);

	ok = waitForComplete(parentWidget, tr("Set signals workcopies"));

	return ok;

}

bool DbController::deleteSignal(int signalID, ObjectState* objectState, QWidget* parentWidget)
{
	if (objectState == nullptr)
	{
		assert(objectState != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_deleteSignal(signalID, objectState);

	ok = waitForComplete(parentWidget, tr("Delete signal"));

	return ok;
}

bool DbController::undoSignalChanges(int signalID, ObjectState* objectState, QWidget* parentWidget)
{
	if (objectState == nullptr)
	{
		assert(objectState != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_undoSignalChanges(signalID, objectState);

	ok = waitForComplete(parentWidget, tr("Undo signal changes"));

	return ok;
}

bool DbController::undoSignalsChanges(QVector<int> signalIDs, QVector<ObjectState>* objectStates, QWidget* parentWidget)
{
	if (objectStates == nullptr)
	{
		assert(objectStates != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_undoSignalsChanges(signalIDs, objectStates);

	ok = waitForComplete(parentWidget, tr("Undo signals changes"));

	return ok;

}

bool DbController::checkinSignals(QVector<int>* signalIDs, QString comment, QVector<ObjectState> *objectState, QWidget* parentWidget)
{
	if (signalIDs == nullptr)
	{
		assert(signalIDs != nullptr);
		return false;
	}

	if (objectState == nullptr)
	{
		assert(objectState != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	QString trimmedCommnet = comment.trimmed();

	emit signal_checkinSignals(signalIDs, trimmedCommnet, objectState);

	ok = waitForComplete(parentWidget, tr("Checkin signals"));

	return ok;
}



bool DbController::autoAddSignals(const std::vector<Hardware::DeviceAppSignal*>* deviceSignals, std::vector<AppSignal>* addedSignals, QWidget* parentWidget)
{
	if (deviceSignals == nullptr)
	{
		assert(deviceSignals != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_autoAddSignals(deviceSignals, addedSignals);

	ok = waitForComplete(parentWidget, tr("Auto add signals"));

	return ok;
}


bool DbController::autoDeleteSignals(const std::vector<Hardware::DeviceAppSignal*>* deviceSignals, QWidget* parentWidget)
{
	if (deviceSignals == nullptr)
	{
		assert(deviceSignals != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_autoDeleteSignals(deviceSignals);

	ok = waitForComplete(parentWidget, tr("Auto delete signals"));

	return ok;
}


bool DbController::getSignalsIDsWithAppSignalID(QString appSignalID, QVector<int>* signalIDs, QWidget* parentWidget)
{
	if (signalIDs == nullptr)
	{
		assert(signalIDs != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getSignalsIDsWithAppSignalID(appSignalID, signalIDs);

	ok = waitForComplete(parentWidget, tr("Get signals IDs with AppSignalID"));

	return ok;
}


bool DbController::getSignalsIDsWithCustomAppSignalID(QString customAppSignalID, QVector<int>* signalIDs, QWidget* parentWidget)
{
	if (signalIDs == nullptr)
	{
		assert(signalIDs != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getSignalsIDsWithCustomAppSignalID(customAppSignalID, signalIDs);

	ok = waitForComplete(parentWidget, tr("Get signals IDs with CustomAppSignalID"));

	return ok;
}

bool DbController::getSignalsIDsWithEquipmentID(const QString& equipmentID, QVector<int>* signalIDs, QWidget* parentWidget)
{
	if (signalIDs == nullptr)
	{
		assert(signalIDs != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getSignalsIDsWithEquipmentID(equipmentID, signalIDs);

	ok = waitForComplete(parentWidget, tr("Get signals IDs with EquipmentID"));

	return ok;
}

bool DbController::getMultipleSignalsIDsWithEquipmentID(const QStringList& equipmentIDs, QMultiHash<QString, int>* signalIDs, QWidget* parentWidget)
{
	if (signalIDs == nullptr)
	{
		assert(signalIDs != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_getMultipleSignalsIDsWithEquipmentID(equipmentIDs, signalIDs);

	ok = waitForComplete(parentWidget, tr("Get signals IDs with EquipmentID"));

	return ok;
}

bool DbController::getSignalHistory(int signalID, std::vector<DbChangeset>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (out == nullptr)
	{
		assert(out != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getSignalHistory(signalID, out);

	ok = waitForComplete(parentWidget, tr("Getting signal history"));

	return true;
}


bool DbController::getSpecificSignals(const std::vector<int>* signalIDs, int changesetId, std::vector<AppSignal>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (signalIDs == nullptr)
	{
		assert(signalIDs != nullptr);
		return false;
	}

	if (out == nullptr)
	{
		assert(out != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getSpecificSignals(signalIDs, changesetId, out);

	ok = waitForComplete(parentWidget, tr("Getting specific signals"));

	return true;
}

bool DbController::getSpecificSignals(int changesetId, std::vector<AppSignal>* out, QWidget* parentWidget)
{
	if (out == nullptr)
	{
		Q_ASSERT(out);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getSpecificSignals(changesetId, out);

	ok = waitForComplete(parentWidget, tr("Getting specific signals"));

	return true;
}

bool DbController::getSpecificSignals(QDateTime date, std::vector<AppSignal>* out, QWidget* parentWidget)
{
	if (out == nullptr)
		{
			Q_ASSERT(out);
			return false;
		}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getSpecificSignals(date, out);

	ok = waitForComplete(parentWidget, tr("Getting specific signals"));

	return true;
}

bool DbController::hasCheckedOutSignals(bool* hasCheckedOut, QWidget* parentWidget)
{
	// Check parameters
	//
	if (hasCheckedOut == nullptr)
	{
		assert(false);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_hasCheckedOutSignals(hasCheckedOut);

	ok = waitForComplete(parentWidget, tr("HasCheckedOutSignals checking"));

	return true;

}


bool DbController::buildStart(QString workstation, int changeset, int* buildID, QWidget* parentWidget)
{
	if (buildID == nullptr)
	{
		assert(buildID != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_buildStart(workstation, changeset, buildID);

	ok = waitForComplete(parentWidget, tr("Build started"));

	return ok;
}


bool DbController::buildFinish(int buildID, int errors, int warnings, QString buildLog, QWidget *parentWidget)
{
	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_buildFinish(buildID, errors, warnings, buildLog);

	ok = waitForComplete(parentWidget, tr("Build started"));

	return ok;
}


bool DbController::isAnyCheckedOut(int* checkedOutCount)
{
	if (checkedOutCount == nullptr)
	{
		assert(checkedOutCount != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_isAnyCheckedOut(checkedOutCount);

	ok = waitForComplete(nullptr, tr("Is Any Checked Out?"));

	return ok;
}

bool DbController::lastChangesetId(int* result)
{
	if (result == nullptr)
	{
		assert(result != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_lastChangesetId(result);

	ok = waitForComplete(nullptr, tr("Getting last ChangesetId"));

	return ok;
}

bool DbController::nextCounterValue(int* counter)
{
	if (counter == nullptr)
	{
		assert(counter != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();

	if (ok == false)
	{
		return false;
	}

	emit signal_nextCounterValue(counter);

	ok = waitForComplete(nullptr, tr("Getting next counter value"));

	return ok;
}

int DbController::nextCounterValue()
{
	int value = -1;
	bool ok = nextCounterValue(&value);

	if (ok == false)
	{
		return -1;
	}
	else
	{
		return value;
	}
}

bool DbController::getTags(std::vector<DbTag>* tags)
{
	if (tags == nullptr)
	{
		Q_ASSERT(tags);
		return false;
	}

	static const std::vector<DbTag> defaultTags = {{"applogic", "Application Logic Schema"},
												   {"monitor", "Monitor Schema"},
												   {"tuning", "TuningClient Schema"},
												   {"diagnostics", "Diagnostics Schema"},
												   {"ufb", "UFB Schema"},
												   {"wiring", "Wiring Schema"},
												   {"in", "Input Signal"},
												   {"out", "Output Signal"},
												   {"view_linear", "Signal with Linear Grid"},
												   {"view_log10", "Signal with Logarithmic Grid"},
												   {"view_period", "Signal with Reactor Period Grid"}};

	std::vector<DbFileInfo> fileList;

	bool ok = getFileList(&fileList, DbDir::EtcDir, Db::File::TagsFileName, true, nullptr);
	if (ok == false || fileList.size() != 1)
	{
		*tags = defaultTags;
		return true;	// File does not exist
	}

	std::shared_ptr<DbFile> file;

	if (getLatestVersion(fileList[0], &file, nullptr) == false)
	{
		return false;
	}

	QTextStream in(file->data());

	while (in.atEnd() == false)
	{
		QString str = in.readLine();
		if (str.isEmpty() == false)
		{
			str = str.trimmed();

			QStringList tag = str.split(';', Qt::KeepEmptyParts);

			if (tag.size() != 2)
			{
				Q_ASSERT(false);
				continue;
			}

			tags->push_back({tag[0], tag[1]});
		}
	}

	return true;
}

bool DbController::writeTags(const std::vector<DbTag> tags, const QString& comment)
{
	// save to db
	//
	std::shared_ptr<DbFile> file = nullptr;
	std::vector<DbFileInfo> fileList;
	int etcFileId = systemFileId(DbDir::EtcDir);

	bool ok = getFileList(&fileList, etcFileId, Db::File::TagsFileName, true, nullptr);
	if (ok == false || fileList.size() != 1)
	{
		// create a file, if it does not exists
		//
		std::shared_ptr<DbFile> pf = std::make_shared<DbFile>();
		pf->setFileName(Db::File::TagsFileName);

		if (addFile(pf, etcFileId, nullptr) == false)
		{
			return false;
		}

		ok = getFileList(&fileList, etcFileId, Db::File::TagsFileName, true, nullptr);
		if (ok == false || fileList.size() != 1)
		{
			return false;
		}
	}

	ok = getLatestVersion(fileList[0], &file, nullptr);
	if (ok == false || file == nullptr)
	{
		return false;
	}

	if (file->state() != E::VcsState::CheckedOut)
	{
		if (checkOut(fileList[0], nullptr) == false)
		{
			return false;
		}
	}

	QByteArray data;

	for (const DbTag& tag : tags)
	{
		data.append((tr("%1;%2\n").arg(tag.tag).arg(tag.description)).toUtf8());
	}

	file->swapData(data);

	if (setWorkcopy(file, nullptr) == false)
	{
		return false;
	}

	if (checkIn(fileList[0], comment, nullptr) == false)
	{
		return false;
	}

	return true;
}


bool DbController::getUserList(std::vector<DbUser>* out, QWidget* parentWidget)
{
	// Check parameters
	//
	if (out == nullptr)
	{
		assert(out != nullptr);
		return false;
	}

	// Init progress and check availability
	//
	bool ok = initOperation();
	if (ok == false)
	{
		return false;
	}

	// Emit signal end wait for complete
	//
	emit signal_getUserList(out);

	bool result = waitForComplete(parentWidget, tr("Getting user list"));
	return result;
}


// Must be called from the GUI thread
//
bool DbController::initOperation()
{
	if (m_operationMutex.tryLock() == false)		// MUST BE UNLOCKED LATER (in waitForComplete!!!)
	{
		qDebug() << "DbController: Another operation is in progress!";
		return false;
	}

	m_lastError.clear();

	return m_progress.init();
}

// Must be called from the GUI thread
//
bool DbController::waitForComplete(QWidget* parentWidget, const QString& description)
{
	bool result = m_progress.run(parentWidget, description);

	if (result == false || m_progress.hasError() == true)
	{
		m_lastError = m_progress.errorMessage();
	}

	m_operationMutex.unlock();						// WAS LOCKED IN initOperation
	return result;
}

bool DbController::isProjectOpened() const
{
	assert(m_worker);
	return m_worker->isProjectOpened();
}

int DbController::databaseVersion()
{
	return DbWorker::databaseVersion();
}

void DbController::enableProgress()
{
	m_progress.enableProgress();
}

void DbController::disableProgress()
{
	m_progress.disableProgress();
}

bool DbController::isProgressEnabled() const
{
	return m_progress.isProgressEnabled();
}

const QString& DbController::host() const
{
	assert(m_worker);
	return m_worker->host();
}

void DbController::setHost(const QString& host)
{
	assert(m_worker);
	m_worker->setHost(host);
}

int DbController::port() const
{
	assert(m_worker);
	return m_worker->port();
}

void DbController::setPort(int port)
{
	assert(m_worker);
	m_worker->setPort(port);
}

const QString& DbController::serverUsername() const
{
	assert(m_worker);
	return m_worker->serverUsername();
}

void DbController::setServerUsername(const QString& username)
{
	assert(m_worker);
	m_worker->setServerUsername(username);
}

const QString& DbController::serverPassword() const
{
	assert(m_worker);
	return m_worker->serverPassword();
}

void DbController::setServerPassword(const QString& password)
{
	assert(m_worker);
	m_worker->setServerPassword(password);
}

DbUser DbController::currentUser() const
{
	assert(m_worker);
	return m_worker->currentUser();
}

void DbController::setCurrentUser(const DbUser& /*user*/)
{
	// Do we really need this function?
	assert(false);
}

DbProject DbController::currentProject() const
{
	assert(m_worker);
	return m_worker->currentProject();
}

void DbController::setCurrentProject(const DbProject& /*project*/)
{
	// Do we really need this function?
	assert(false);
}

int DbController::rootFileId() const
{
	return m_worker->rootFileId();
}

int DbController::systemFileId(DbDir dir) const
{
	return m_worker->systemFileId(dir);
}

DbFileInfo DbController::systemFileInfo(DbDir dir) const
{
	return m_worker->systemFileInfo(dir);
}

std::vector<DbFileInfo> DbController::systemFiles() const
{
	return m_worker->systemFiles();
}

DbFileInfo DbController::systemFileInfo(const QString& fileName) const
{
	QString shortFileName = DbFileInfo::fullPathToFileName(fileName);

	DbFileInfo result;
	result.setFileId(-1);

	std::vector<DbFileInfo> systemFiles = m_worker->systemFiles();

	auto pos = std::find_if(systemFiles.begin(), systemFiles.end(),
		[&shortFileName](const DbFileInfo& fi)
		{
			return fi.fileName() == shortFileName;
		});

	if (pos != systemFiles.end())
	{
		result = *pos;
	}

	return result;
}

DbFileInfo DbController::systemFileInfo(int fileId) const
{
	DbFileInfo result;
	result.setFileId(DbFileInfo::Null);

	std::vector<DbFileInfo> systemFiles = m_worker->systemFiles();

	auto pos = std::find_if(systemFiles.begin(), systemFiles.end(),
		[&fileId](const DbFileInfo& fi)
		{
			return fi.fileId() == fileId;
		});

	if (pos != systemFiles.end())
	{
		result = *pos;
	}

	return result;
}

bool DbController::isSystemFile(int fileId) const
{
	std::vector<DbFileInfo> systemFiles = m_worker->systemFiles();

	auto pos = std::find_if(systemFiles.begin(), systemFiles.end(),
		[&fileId](const DbFileInfo& fi)
		{
			return fi.fileId() == fileId;
		});

	return pos != systemFiles.end();
}

QString DbController::lastError() const
{
	return m_lastError;
}

QString DbController::username(int userId) const
{
	QMutexLocker ml(&m_userMutex);

	auto it = m_users.find(userId);

	if (it == m_users.end())
	{
		return QString("Undefined");
	}
	else
	{
		return it->second.username();
	}
}

HasDbController::HasDbController(DbController* dbcontroller) :
	m_db(dbcontroller)
{
	assert(dbcontroller);
}
