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
	connect(this, &DbController::signal_upgradeProject, m_worker, &DbWorker::slot_upgradeProject);

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

bool DbController::upgradeProject(const QString& projectName, QWidget* parentWidget)
{
	// Check parameters
	//
	return false;
	/*if (projectName.isEmpty())
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
	emit signal_closeProject();

	bool result = waitForComplete(parentWidget, tr("Closing project"));

	if (result == true)
	{
		emit projectClosed();
	}

	return result;*/
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

void DbController::setCurrentUser(const DbUser& user)
{
	// Do we really need this function?
	assert(false);
}

DbProject DbController::currentProject() const
{
	assert(m_worker);
	return m_worker->currentProject();
}

void DbController::setCurrentProject(const DbProject& project)
{
	// Do we really need this function?
	assert(false);
}
