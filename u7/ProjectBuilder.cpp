#include "ProjectBuilder.h"
#include <QJSEngine>
#include "../include/DbController.h"
#include "../include/OutputLog.h"
#include "../include/DeviceObject.h"

#include <QThread>


void BuildWorkerThread::run()
{
	QThread::currentThread()->setTerminationEnabled(true);

	qDebug() << "Building started";
	m_log->writeMessage("Building started");

	if (onlyCheckedIn() == false)
	{
		m_log->writeWarning(tr("WARNING: The workcopies of the checked out files will be compiled!"), true);
	}

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

	do
	{
		//
		// Get Equipment from the database
		//
		m_log->writeMessage("");
		m_log->writeMessage(tr("Getting equipment"));

		Hardware::DeviceRoot deviceRoot;
		int rootFileId = db.hcFileId();
		deviceRoot.fileInfo().setFileId(rootFileId);

		ok = getEquipment(&db, &deviceRoot);

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			break;
		}

		if (ok == false)
		{
			m_log->writeError(tr("Error"), true);
			break;
		}
		else
		{
			m_log->writeMessage(tr("Ok"));
		}

		//
		// Expand Devices StrId
		//
		m_log->writeMessage("");
		m_log->writeMessage(tr("Expanding devices StrIds"));

		expandDeviceStrId(&deviceRoot);

		m_log->writeMessage(tr("Ok"));

		//
		// Generate Module Confuiguration Binary File
		//
		m_log->writeMessage("");
		m_log->writeMessage(tr("Generating modules configurations"));

		ok = generateModulesConfigurations(&db, &deviceRoot);

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			break;
		}

		if (ok == false)
		{
			m_log->writeError(tr("Error"), true);
			break;
		}
		else
		{
			m_log->writeMessage(tr("Ok"));
		}

	}
	while (false);

	// Closing project and saying bye-bye!
	//
	ok = db.closeProject(nullptr);

	if (QThread::currentThread()->isInterruptionRequested() == true)
	{
		str = tr("Building project %1: canceled").arg(projectName());

		m_log->writeMessage("");
		m_log->writeError(str, true);
		qDebug() << str;
	}
	else
	{
		str = tr("Building project %1: ok").arg(projectName());

		m_log->writeMessage("");
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

	if (QThread::currentThread()->isInterruptionRequested() == true)
	{
		return false;
	}

	if (parent->deviceType() == Hardware::DeviceType::System)
	{
		m_log->writeMessage(tr("Getting system %1...").arg(parent->caption()));
	}

	std::vector<DbFileInfo> files;

	bool ok = false;

	if (onlyCheckedIn() == true)
	{
		assert(false);
	}
	else
	{
		ok = db->getFileList(&files, parent->fileInfo().fileId(), nullptr);
	}

	if (ok == false)
	{
		return false;
	}

	parent->deleteAllChildren();

	for (auto& fi : files)
	{
		std::shared_ptr<DbFile> file;

		if (onlyCheckedIn() == true)
		{
			assert(false);
		}
		else
		{
			ok = db->getLatestVersion(fi, &file, nullptr);
		}

		if (file == false || ok == false)
		{
			return false;
		}

		Hardware::DeviceObject* object = Hardware::DeviceObject::Create(file->data());

		if (object == nullptr)
		{
			return false;
		}
		else
		{
			assert(object);
		}

		object->setFileInfo(fi);

		std::shared_ptr<Hardware::DeviceObject> sp(object);

		parent->addChild(sp);
	}

	files.clear();

	for (int i = 0 ; i < parent->childrenCount(); i++)
	{
		std::shared_ptr<Hardware::DeviceObject> child = parent->childSharedPtr(i);

		ok = getEquipment(db, child.get());

		if (ok == false)
		{
			return false;
		}
	}

	return true;
}

bool BuildWorkerThread::expandDeviceStrId(Hardware::DeviceObject* device)
{
	if (device->parent() != nullptr)
	{
		QString strId = device->strId();

		strId.replace(QString("$(PARENT)"), device->parent()->strId(), Qt::CaseInsensitive);
		strId.replace(QString("$(PLACE)"), QString::number(device->place()).rightJustified(2, '0'), Qt::CaseInsensitive);

		device->setStrId(strId);
	}

	for (int i = 0; i < device->childrenCount(); i++)
	{
		expandDeviceStrId(device->child(i));
	}

	return true;
}

bool BuildWorkerThread::generateModulesConfigurations(DbController* db, const Hardware::DeviceObject* root)
{
	// !!! Read script from file, IT IS TEMPORARY, in future this script must be taken from the Project DB !!!!
	//
	m_log->writeWarning("Temoparary reading script from file, in future must be moved to DB!", true);

	QString fileName = "LogicModuleConfiguration.js";
	QFile scriptFile(fileName);

	if (!scriptFile.open(QIODevice::ReadOnly))
	{
		m_log->writeError(tr("Can't read file %1").arg(fileName));
		return false;
	}

	QTextStream stream(&scriptFile);
	QString contents = stream.readAll();
	scriptFile.close();

	// Attach objects
	//
	QJSEngine jsEngine;

	QJSValue jsLog = jsEngine.newQObject(m_log);
	QQmlEngine::setObjectOwnership(m_log, QQmlEngine::CppOwnership);

	// Run script
	//
	QJSValue jsEval = jsEngine.evaluate(contents, fileName);

	QJSValueList args;
	args << jsLog;

	QJSValue jsResult = jsEval.call(args);

	if (jsResult.isError() == true)
	{
		m_log->writeError(tr("Uncaught exception while generation module configuration: %1").arg(jsResult.toString()));
		return false;
	}

	qDebug() << jsResult.toInt();

	// Process results
	//
	return true;

	/*std::map<QString, std::shared_ptr<Hardware::McFirmware>> firmwares;

	bool ok = generateModulesConfigurations(db, root, &firmwares);
*/
	//return ok;
}

bool BuildWorkerThread::generateModulesConfigurations(
		DbController* db,
		const Hardware::DeviceObject* parent,
		std::map<QString, std::shared_ptr<Hardware::McFirmware>>* firmwares)
{
	assert(db != nullptr);
	assert(db->isProjectOpened() == true);
	assert(parent != nullptr);
	assert(parent->deviceType() < Hardware::DeviceType::Module);
	assert(firmwares != nullptr);

	if (QThread::currentThread()->isInterruptionRequested() == true)
	{
		return false;
	}

	if (parent->deviceType() == Hardware::DeviceType::System)
	{
		m_log->writeMessage(tr("System %1...").arg(parent->caption()));
	}

	for (int i = 0; i < parent->childrenCount(); i++)
	{
		Hardware::DeviceObject* child = parent->child(i);

		if (child->deviceType() > Hardware::DeviceType::Module)
		{
			continue;
		}

//		if (child->deviceType() < Hardware::DeviceType::Module)
//		{
//			generateModulesConfigurations(db, child);
//			continue;
//		}

//		// This is Module, if it has configuration process it.
//		//
//		assert(child->deviceType() == Hardware::DeviceType::Module);

//		Hardware::DeviceModule* module = dynamic_cast<Hardware::DeviceModule*>(child);
//		assert(module != nullptr);

//		if (module->moduleConfiguration().hasConfiguration() == false)
//		{
//			// Module does not have configuration, process the next child;
//			//
//			continue;
//		}

//		// Get the firmware by it's name or create new if it does not exists
//		//
//		const Hardware::ModuleConfiguration& moduleConfiguration = module->moduleConfiguration();

//		QString confFileName = module->confFirmwareName();
//		std::shared_ptr<Hardware::McFirmware> firmware;

//		try
//		{
//			firmware = firmwares->at(confFileName);
//		}
//		catch(std::out_of_range)
//		{
//			firmware = std::make_shared<Hardware::McFirmware>();

//			firmware->setName(confFileName);
//			firmware->setUartId(moduleConfiguration.uartId());

//			firmwares->insert(std::make_pair(confFileName, firmware));
//		}

//		QString error;

//		// Compile configuration to firmware
//		//
//		qDebug() << module->strId();

//		bool ok = module->compileConfiguration(firmware.get(), &error);

//		if (ok == false)
//		{
//			// Somthing went wrong.
//			//
//			m_log->writeError(error, false);
//			m_log->writeError(tr("Device StrId: %1, caption: %2, place: %3")
//							  .arg(module->strId())
//							  .arg(module->caption())
//							  .arg(module->place()), false);
//			return false;
//		}
	}

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

bool BuildWorkerThread::onlyCheckedIn() const
{
	return m_onlyCheckedIn;

}

void BuildWorkerThread::setOnlyCheckedIn(bool value)
{
	m_onlyCheckedIn = value;
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

bool ProjectBuilder::start(QString projectName,
						   QString ipAddress,
						   int port,
						   QString serverUserName,
						   QString serverPassword,
						   QString projectUserName,
						   QString projectUserPassword,
						   bool onlyCheckedIn)
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
	m_thread->setOnlyCheckedIn(onlyCheckedIn);

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

void ProjectBuilder::handleResults(QString /*result*/)
{
}

