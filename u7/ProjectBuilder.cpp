#include "ProjectBuilder.h"
#include "../include/DbController.h"
#include "../include/OutputLog.h"
#include "../include/DeviceObject.h"

#include <QThread>


void BuildWorkerThread::run()
{
	QThread::currentThread()->setTerminationEnabled(true);

	qDebug() << "Building started";

	QString str;
	bool ok = false;

	// Create database controller and open project
	//
	DbController db;

	db.disableProgress();

	db.setHost(serverIpAddress());
	db.setPort(serverPort());
	db.setServerUsername(serverUsername());
	db.setServerPassword(serverPassword());

	ok = db.openProject(projectName(), projectUserName(), projectUserPassword(), nullptr);

	if (ok == false)
	{
		m_log->writeError(db.lastError());
		m_log->writeError(tr("Opening project %1: error").arg(projectName()), true);
		return;
	}
	else
	{
		m_log->writeMessage(tr("Opening project %1: ok").arg(projectName()), true);
	}

	// Get Equipment from the database
	//
	m_log->writeMessage("");
	m_log->writeMessage(tr("Getting equipment"));

	Hardware::DeviceRoot deviceRoot;
	int rootFileId = db.hcFileId();
	deviceRoot.fileInfo().setFileId(rootFileId);

	ok = getEquipment(&db, &deviceRoot);

	if (ok == false)
	{
		m_log->writeError(tr("Getting equipment: error"), true);
		return;
	}
	else
	{
		m_log->writeMessage(tr("Getting equipment: ok"));
	}

	//---

/*	for (int i =0; i < 5; i++)
	{
		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			break;
		}

		QThread::yieldCurrentThread();

		QThread::sleep(1);
	}*/

	// Closing project and saying bye-bye!
	//
	ok = db.closeProject(nullptr);


	if (QThread::currentThread()->isInterruptionRequested() == true)
	{
		str = tr("Building project %1: canceled").arg(projectName());

		m_log->writeError(str, true);
		qDebug() << str;
	}
	else
	{
		str = tr("Building project %1: ok").arg(projectName());

		m_log->writeSuccess(str, true);
		qDebug() << str;

		emit resultReady(QString("Cool, we've done!"));
	}

	return;
}

bool BuildWorkerThread::getEquipment(DbController* db, Hardware::DeviceObject* parent)
{
	assert(db != nullptr);
	assert(db->isProjectOpened() == true);
	assert(parent != nullptr);

	std::vector<DbFileInfo> files;

	qDebug() << "before db->getFileList";

	bool ok = db->getFileList(&files, parent->fileInfo().fileId(), nullptr);

	qDebug() << "after db->getFileList";

	if (ok == false)
	{
		return false;
	}

/*	parent->deleteAllChildren();

	for (auto& fi : files)
	{
		std::shared_ptr<DbFile> file;

		ok = db->getLatestVersion(fi, &file, nullptr);

		if (file == false || ok == false)
		{
			return false;
		}

		Hardware::DeviceObject* object = Hardware::DeviceObject::Create(file->data());
		assert(object);

		if (object == nullptr)
		{
			return false;
		}

		object->setFileInfo(fi);

		std::shared_ptr<Hardware::DeviceObject> sp(object);

		parent->addChild(sp);

		// log
		//
		if (sp->deviceType() == Hardware::DeviceType::System)
		{
			m_log->writeMessage(tr("Getting system %1...").arg(sp->caption()));
		}
	}*/

	files.clear();

//	for (int i = 0 ; i < parent->childrenCount(); i++)
//	{
//		std::shared_ptr<Hardware::DeviceObject> child = parent->childSharedPtr(i);

//		ok = getEquipment(db, child.get());

//		if (ok == false)
//		{
//			return false;
//		}
//	}

	return true;
}

QString BuildWorkerThread::projectName() const
{
	QMutexLocker m(&m_mutex);
	return m_projectName;
}

void BuildWorkerThread::setProjectName(const QString& value)
{
	QMutexLocker m(&m_mutex);
	m_projectName = value;
}

QString BuildWorkerThread::serverIpAddress() const
{
	QMutexLocker m(&m_mutex);
	return m_serverIpAddress;
}

void BuildWorkerThread::setServerIpAddress(const QString& value)
{
	QMutexLocker m(&m_mutex);
	m_serverIpAddress = value;
}

int BuildWorkerThread::serverPort() const
{
	QMutexLocker m(&m_mutex);
	return m_serverPort;
}

void BuildWorkerThread::setServerPort(int value)
{
	QMutexLocker m(&m_mutex);
	m_serverPort = value;
}

QString BuildWorkerThread::serverUsername() const
{
	QMutexLocker m(&m_mutex);
	return m_serverUsername;
}

void BuildWorkerThread::setServerUsername(const QString& value)
{
	QMutexLocker m(&m_mutex);
	m_serverUsername = value;
}

QString BuildWorkerThread::serverPassword() const
{
	QMutexLocker m(&m_mutex);
	return m_serverPassword;
}

void BuildWorkerThread::setServerPassword(const QString& value)
{
	QMutexLocker m(&m_mutex);
	m_serverPassword = value;
}

void BuildWorkerThread::setOutputLog(OutputLog* value)
{
	QMutexLocker m(&m_mutex);
	m_log = value;
}

QString BuildWorkerThread::projectUserName() const
{
	QMutexLocker m(&m_mutex);
	return m_projectUserName;
}

void BuildWorkerThread::setProjectUserName(const QString& value)
{
	QMutexLocker m(&m_mutex);
	m_projectUserName = value;
}

QString BuildWorkerThread::projectUserPassword() const
{
	QMutexLocker m(&m_mutex);
	return m_projectUserPassword;
}

void BuildWorkerThread::setProjectUserPassword(const QString& value)
{
	QMutexLocker m(&m_mutex);
	m_projectUserPassword = value;
}


ProjectBuilder::ProjectBuilder(OutputLog* log) :
	m_log(log)
{
	assert(m_log != nullptr);

	m_thread = new BuildWorkerThread();
	m_thread->setObjectName(tr("BuildWorkerThread"));
	m_thread->setOutputLog(m_log);

	connect(m_thread, &BuildWorkerThread::resultReady, this, &ProjectBuilder::handleResults);

	connect(m_thread, &BuildWorkerThread::started, this, &ProjectBuilder::buildStarted);
	connect(m_thread, &BuildWorkerThread::finished, this, &ProjectBuilder::buildFinished);

	return;
}

ProjectBuilder::~ProjectBuilder()
{
	m_thread->requestInterruption();

	bool result = m_thread->wait(10000);		// Wait for 10 sec.

	if (result == false)
	{
		qDebug() << "Building thread was not finished.";
		m_thread->terminate();
	}

	delete m_thread;
	return;
}

bool ProjectBuilder::start(QString projectName, QString ipAddress, int port, QString serverUserName, QString serverPassword, QString projectUserName, QString projectUserPassword)
{
	assert(m_thread != nullptr);

	if (isRunning() == true)
	{
		assert(isRunning() == false);
		m_thread->wait(10000);
	}

	// Set params
	//

	m_thread->setProjectName(projectName);
	m_thread->setServerIpAddress(ipAddress);
	m_thread->setServerPort(port);
	m_thread->setServerUsername(serverUserName);
	m_thread->setServerPassword(serverPassword);
	m_thread->setProjectUserName(projectUserName);
	m_thread->setProjectUserPassword(projectUserPassword);

	// Ready? Go!
	//
	m_thread->start();
	return true;
}

void ProjectBuilder::stop()
{
	m_thread->requestInterruption();
}

bool ProjectBuilder::isRunning() const
{
	bool result = m_thread->isRunning();
	return result;
}

void ProjectBuilder::handleResults(QString result)
{
}

