#include "../include/DbController.h"

DbController::DbController() :
	m_worker(nullptr),
	m_operationMutex(QMutex::NonRecursive)

{
	m_worker = new DbWorker(&m_progress);
	m_worker->moveToThread(&m_thread);

	connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);		// delete m_worker on thread termination

	connect(&m_thread, &QThread::started, [](){qDebug() << "Database communication thread has been started";});
	connect(&m_thread, &QThread::finished, [](){qDebug() << "Database communication thread has been finished";});

	// Notifications
	//

	// Operations
	//
	connect(this, &DbController::signal_getProjectList, m_worker, &DbWorker::slot_getProjectList);
	connect(this, &DbController::signal_createProject, m_worker, &DbWorker::slot_createProject);
	connect(this, &DbController::signal_openProject, m_worker, &DbWorker::slot_openProject);
	connect(this, &DbController::signal_closeProject, m_worker, &DbWorker::slot_closeProject);
	connect(this, &DbController::signal_deleteProject, m_worker, &DbWorker::slot_deleteProject);
	connect(this, &DbController::signal_upgradeProject, m_worker, &DbWorker::slot_upgradeProject);

	connect(this, &DbController::signal_createUser, m_worker, &DbWorker::slot_createUser);
	connect(this, &DbController::signal_updateUser, m_worker, &DbWorker::slot_updateUser);
	connect(this, &DbController::signal_getUserList, m_worker, &DbWorker::slot_getUserList);

	connect(this, &DbController::signal_getFileList, m_worker, &DbWorker::slot_getFileList);
	connect(this, &DbController::signal_addFiles, m_worker, &DbWorker::slot_addFiles);

	connect(this, &DbController::signal_getWorkcopy, m_worker, &DbWorker::slot_getWorkcopy);
	connect(this, &DbController::signal_setWorkcopy, m_worker, &DbWorker::slot_setWorkcopy);

	connect(this, &DbController::signal_checkIn, m_worker, &DbWorker::slot_checkIn);
	connect(this, &DbController::signal_checkOut, m_worker, &DbWorker::slot_checkOut);
	connect(this, &DbController::signal_undoChanges, m_worker, &DbWorker::slot_undoChanges);

	m_thread.start();
}

DbController::~DbController()
{
	m_thread.quit();
	m_thread.wait();
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

bool DbController::createProject(const QString& projectName, const QString& administratorPassword, QWidget* parentWidget)
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

bool DbController::openProject(const QString& projectName, const QString& username, const QString& password, QWidget* parentWidget)
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

	bool result = waitForComplete(parentWidget, tr("Openning project"));

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

bool DbController::deleteProject(const QString& projectName, const QString& password, QWidget* parentWidget)
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
	emit signal_deleteProject(projectName, password);

	bool result = waitForComplete(parentWidget, tr("Deleting project"));
	return result;
}

bool DbController::upgradeProject(const QString& projectName, const QString& password, QWidget* parentWidget)
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
	emit signal_upgradeProject(projectName, password);

	bool result = waitForComplete(parentWidget, tr("Upgrading project"));
	return result;
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

bool DbController::getFileList(std::vector<DbFileInfo>* files, int parentId, QWidget* parentWidget)
{
	return getFileList(files, parentId, QString(), parentWidget);
}

bool DbController::getFileList(std::vector<DbFileInfo>* files, int parentId, const QString& filter, QWidget* parentWidget)
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
	emit signal_getFileList(files, parentId, filter);

	bool result = waitForComplete(parentWidget, tr("Geting file list"));
	return result;
}

bool DbController::addFiles(std::vector<std::shared_ptr<DbFile>>* files, int parentId, QWidget* parentWidget)
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
	emit signal_addFiles(files, parentId);

	bool result = waitForComplete(parentWidget, tr("Adding files"));
	return result;
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
	emit signal_checkIn(&files, comment);

	ok = waitForComplete(parentWidget, tr("Checking in files"));
	return true;
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
		qDebug() << "DbStore: Another operation is in progress!";
		return false;
	}

	return m_progress.init();
}

// Must be called from the GUI thread
//
bool DbController::waitForComplete(QWidget* parentWidget, const QString& description)
{
	bool result = m_progress.run(parentWidget, description);
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

int DbController::afblFileId() const
{
	return m_worker->afblFileId();
}

int DbController::alFileId() const
{
	return m_worker->alFileId();
}

int DbController::hcFileId() const
{
	return m_worker->hcFileId();
}

int DbController::wvsFileId() const
{
	return m_worker->wvsFileId();
}

int DbController::dvsFileId() const
{
	return m_worker->dvsFileId();
}
