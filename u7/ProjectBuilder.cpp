#include "ProjectBuilder.h"
#include <QJSEngine>
#include "../include/DbController.h"
#include "../include/OutputLog.h"
#include "../include/DeviceObject.h"

#include "../VFrame30/LogicScheme.h"
#include "../VFrame30/VideoItemLink.h"
#include "../VFrame30/HorzVertLinks.h"

#include <QThread>
#include <QUuid>


void BuildWorkerThread::run()
{
	QThread::currentThread()->setTerminationEnabled(true);

	qDebug() << "Building started";

	if (debug() == true)
	{
		m_log->writeWarning(tr("DEBUG Building started"), true);
		m_log->writeWarning(tr("WARNING: The workcopies of the checked out files will be compiled!"), true);
	}
	else
	{
		m_log->writeMessage(tr("RELEASE Building started"), true);
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		m_log->writeError(tr("RELEASE BUILD IS UNDER CONSTRACTION!"), true);
		return;
	}

	bool ok = false;
	QString str;

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
		int lastChangesetId = 0;
		ok = db.lastChangesetId(&lastChangesetId);

		if (ok == false)
		{
			m_log->writeError(tr("lastChangesetId Error."), true);
			break;
		}

		bool isAnyCheckedOut = false;
		ok = db.isAnyCheckedOut(&isAnyCheckedOut);

		if (ok == false)
		{
			m_log->writeError(tr("isAnyCheckedOut Error."), true);
			QThread::currentThread()->requestInterruption();
			break;
		}

		if (release() == true && isAnyCheckedOut == true)
		{
			m_log->writeError(tr("There are some checked out objects. Please check in all objects before building release version."), true);
			QThread::currentThread()->requestInterruption();
			break;
		}

		//
		// Get Equipment from the database
		//
		m_log->writeMessage("");
		m_log->writeMessage(tr("Getting equipment"), true);

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
			QThread::currentThread()->requestInterruption();
			break;
		}
		else
		{
			m_log->writeSuccess(tr("Ok"), true);
		}

		//
		// Expand Devices StrId
		//
		m_log->writeMessage("");
		m_log->writeMessage(tr("Expanding devices StrIds"), true);

		expandDeviceStrId(&deviceRoot);

		m_log->writeSuccess(tr("Ok"), true);

		//
		// Generate Module Confuiguration Binary File
		//
		m_log->writeMessage("");
		m_log->writeMessage(tr("Generating modules configurations"), true);

		ok = generateModulesConfigurations(&db, &deviceRoot);

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			break;
		}

		if (ok == false)
		{
			m_log->writeError(tr("Error"), true);
			QThread::currentThread()->requestInterruption();
			break;
		}
		else
		{
			m_log->writeSuccess(tr("Ok"), true);
		}


		//
		// Compile application logic
		//
		m_log->writeMessage("");
		m_log->writeMessage(tr("Application Ligic compilation"), true);

		ok = applicationLogic(&db);

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			break;
		}

		if (ok == false)
		{
			m_log->writeError(tr("Error"), true);
			QThread::currentThread()->requestInterruption();
			break;
		}
		else
		{
			m_log->writeSuccess(tr("Ok"), true);
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

	// Get file list with checked out files,
	// if this is release build, specific copies will be fetched later
	//
	ok = db->getFileList(&files, parent->fileInfo().fileId(), nullptr);

	if (ok == false)
	{
		m_log->writeError(tr("Cannot get equipment file list"));
		return false;
	}

	if (release() == true)
	{
		// filter some files, which are not checkedin?
		assert(false);
	}
	else
	{
	}

	parent->deleteAllChildren();

	for (auto& fi : files)
	{
		std::shared_ptr<DbFile> file;

		if (release() == true)
		{
			assert(false);
		}
		else
		{
			ok = db->getLatestVersion(fi, &file, nullptr);
		}

		if (file == false || ok == false)
		{
			m_log->writeError(tr("Cannot get %1 instance.").arg(fi.fileName()));
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

bool BuildWorkerThread::generateModulesConfigurations(DbController* db, Hardware::DeviceObject* root)
{
	if (db == nullptr || root == nullptr)
	{
		assert(db);
		assert(root);
		m_log->writeError(tr("Fatal error, input parammeter is nullptr!"), true);
		return false;
	}

	// Get script file from the project databse
	//
	bool ok = false;
	std::vector<DbFileInfo> fileList;

	if (release() == true)
	{
		assert(false);
	}
	else
	{
		ok = db->getFileList(&fileList, db->mcFileId(), "ModulesConfigurations.descr", nullptr);
	}

	if (ok == false || fileList.size() != 1)
	{
		m_log->writeError(tr("Can't get file list and find Module Configuration description file"), true);
		return false;
	}

	std::shared_ptr<DbFile> scriptFile;

	if (release() == true)
	{
		assert(false);
	}
	else
	{
		ok = db->getLatestVersion(fileList[0], &scriptFile, nullptr);
	}

	if (ok == false || scriptFile == false)
	{
		m_log->writeError(tr("Can't get Module Configuration description file"), true);
		return false;
	}

	QString contents = QString::fromLocal8Bit(scriptFile->data());

	// Attach objects
	//
	QJSEngine jsEngine;

	QJSValue jsLog = jsEngine.newQObject(m_log);
	QQmlEngine::setObjectOwnership(m_log, QQmlEngine::CppOwnership);

	SignalSetObject signalSetObject;
	db->getSignals(signalSetObject.getSignalSet(), nullptr);
	QJSValue jsSignalSet = jsEngine.newQObject(&signalSetObject);
	QQmlEngine::setObjectOwnership(&signalSetObject, QQmlEngine::CppOwnership);

	QJSValue jsRoot = jsEngine.newQObject(root);
	QQmlEngine::setObjectOwnership(root, QQmlEngine::CppOwnership);

	Hardware::ModuleConfCollection confCollection;

	QJSValue jsConfCollection = jsEngine.newQObject(&confCollection);
	QQmlEngine::setObjectOwnership(&confCollection, QQmlEngine::CppOwnership);

	// Run script
	//
	QJSValue jsEval = jsEngine.evaluate(contents, "ModulesConfigurations.descr");
    if (jsEval.isError() == true)
    {
        m_log->writeError(tr("Module configuration script evaluation failed: %1").arg(jsEval.toString()));
        return false;
    }

	QJSValueList args;

	args << jsRoot;
	args << jsConfCollection;
	args << jsLog;
	args << jsSignalSet;

	QJSValue jsResult = jsEval.call(args);

	if (jsResult.isError() == true)
	{
        m_log->writeError(tr("Uncaught exception while generating module configuration: %1").arg(jsResult.toString()));
		return false;
	}

    if (jsResult.toBool() == false)
    {
        m_log->writeError(tr("Module configuration generating failed!"));
        return false;
    }
    qDebug() << jsResult.toInt();

    // Save confCollection items to binary files
	//
	if (release() == true)
	{
		assert(false);
	}
	else
	{
		if (confCollection.save(projectName(), projectUserName()) == false)
		{
			m_log->writeError(tr("Failed to save module configuration binary files!"));
			return false;
		}
	}

	return true;
}

bool BuildWorkerThread::applicationLogic(DbController* db)
{
	if (db == nullptr)
	{
		assert(false);
		return false;
	}

	// Get Application Logic
	//
	std::vector<std::shared_ptr<VFrame30::LogicScheme>> schemes;

	bool ok = loadApplicationLogicFiles(db, &schemes);

	if (ok == false)
	{
		return ok;
	}

	if (schemes.empty() == true)
	{
		m_log->writeMessage(tr("There is no appliction logic files in the project."));
		return true;
	}

	// Compile application logic
	//
	m_log->writeMessage(tr("Compiling..."));

	for (std::shared_ptr<VFrame30::LogicScheme> scheme : schemes)
	{
		m_log->writeMessage(scheme->caption());

		ok = compileApplicationLogicScheme(scheme.get());

		if (ok == false)
		{
			return false;
		}
	}

	return true;
}

bool BuildWorkerThread::loadApplicationLogicFiles(DbController* db, std::vector<std::shared_ptr<VFrame30::LogicScheme>>* out)
{
	if (out == nullptr)
	{
		assert(out);
		return false;
	}

	out->clear();

	// Get application logic file list from the DB
	//
	bool ok = false;
	std::vector<DbFileInfo> applicationLogicFileList;

	if (release() == true)
	{
		assert(false);
	}
	else
	{
		ok = db->getFileList(&applicationLogicFileList, db->alFileId(), "%.als", nullptr);
	}

	if (ok == false)
	{
		m_log->writeError(tr("Cannot get application logic file list."));
		return false;
	}

	if (applicationLogicFileList.empty() == true)
	{
		return true;		// it is not a error
	}

	out->reserve(applicationLogicFileList.size());

	// Get file data and read it
	//
	for (DbFileInfo& fi : applicationLogicFileList)
	{
		m_log->writeMessage(tr("Loading %1").arg(fi.fileName()));

		std::shared_ptr<DbFile> file;

		if (release() == true)
		{
			assert(false);			// get specific files
		}
		else
		{
			ok = db->getLatestVersion(fi, &file, false);
		}

		if (ok == false)
		{
			m_log->writeError(tr("Cannot get application logic file instances."), true);
			return false;
		}

		// Read Appliaction logic files
		//
		std::shared_ptr<VFrame30::LogicScheme> ls(dynamic_cast<VFrame30::LogicScheme*>(VFrame30::Scheme::Create(file.get()->data())));

		if (ls == nullptr)
		{
			assert(ls != nullptr);
			m_log->writeError(tr("File loading error."), true);
			return false;
		}

		// Add LogicScheme to result
		//
		out->push_back(ls);
	}

	return true;
}

bool BuildWorkerThread::compileApplicationLogicScheme(VFrame30::LogicScheme* logicScheme)
{
	if (logicScheme == nullptr)
	{
		assert(false);
		return false;
	}

	// --
	logicScheme->BuildFblConnectionMap();

	// Find layer for compilation
	//
	bool layerFound = false;
	bool ok = false;

	for (std::shared_ptr<VFrame30::SchemeLayer> l : logicScheme->Layers)
	{
		qDebug() << Q_FUNC_INFO << " WARNING!!!! Compiling ALL layers, in future compile just l->compile() LAYER!!!!";

		//if (l->compile() == true)
		{
			layerFound = true;
			ok = compileApplicationLogicLayer(logicScheme, l.get());

			if (ok == false)
			{
				return false;
			}
		}
	}

	if (layerFound == false)
	{
		m_log->writeError(tr("There is no compile layer in the scheme."));
		return false;
	}

	return true;
}

bool BuildWorkerThread::compileApplicationLogicLayer(VFrame30::LogicScheme* logicScheme, VFrame30::SchemeLayer* layer)
{
	if (logicScheme == nullptr || layer == nullptr)
	{
		assert(logicScheme);
		assert(layer);
		return false;
	}

	// Enum all links and get all horzlinks è vertlinks
	//
	VFrame30::CHorzVertLinks horzVertLinks;

	for (auto item = layer->Items.begin(); item != layer->Items.end(); ++item)
	{
		VFrame30::VideoItemLink* link = dynamic_cast<VFrame30::VideoItemLink*>(item->get());

		if (link != nullptr)
		{
			const std::list<VFrame30::VideoItemPoint>& pointList = link->GetPointList();

			if (pointList.size() < 2)
			{
				assert(pointList.size() >= 2);
				continue;
			}

			// Decompose link on different parts and put them to horzlinks and vertlinks
			//
			horzVertLinks.AddLinks(pointList, link->guid());
		}
	}

	// Enum all vert and horz links and compose branches
	//
	std::list<std::set<QUuid>> branches;	// This list contains full branches

	for (auto item = layer->Items.begin(); item != layer->Items.end(); ++item)
	{
		VFrame30::VideoItemLink* link = dynamic_cast<VFrame30::VideoItemLink*>(item->get());

		if (link == nullptr)
		{
			continue;
		}

		const std::list<VFrame30::VideoItemPoint>& pointList = link->GetPointList();

		if (pointList.size() < 2)
		{
			assert(pointList.size() >= 2);
			continue;
		}

		// Check if end points on some link
		//
		std::list<QUuid> videoItemsUnderFrontPoint = horzVertLinks.getVideoItemsUnderPoint(pointList.front(), link->guid());
		std::list<QUuid> videoItemsUnderBackPoint = horzVertLinks.getVideoItemsUnderPoint(pointList.back(), link->guid());

		// Find item branch, if branch does not exists, make a new brach
		//
		auto foundBranch = std::find_if(branches.begin(), branches.end(),
			[link](const std::set<QUuid>& b)
			{
				auto foundBranch = b.find(link->guid());
				return foundBranch != b.end();
			});

		if (foundBranch == branches.end())
		{
			std::set<QUuid> newBranch;
			newBranch.insert(link->guid());

			branches.push_front(newBranch);

			foundBranch = branches.begin();
		}

		// Add to foundBranch everything from  videoItemsUnderFrontPoint, videoItemsUnderBackPoint
		//
		for (QUuid& q : videoItemsUnderFrontPoint)
		{
			foundBranch->insert(q);
		}

		for (QUuid& q : videoItemsUnderBackPoint)
		{
			foundBranch->insert(q);
		}
	}

	// DEBUG
	//
	qDebug() << "";
	qDebug() << "Branches before joining";

	for (std::set<QUuid>& b : branches)
	{
		qDebug() << "--";
		for (const QUuid& q : b)
		{
			qDebug() << q;
		}
	}

	qDebug() << "";


	// branches can contain same items,
	// all such branches must be united
	//
	bool wasJoining = false;	// if branch was joinedto other branch, then process currentBranch one more time

	for (auto& currentBranch = branches.begin();
		 currentBranch != branches.end();
		 wasJoining ? void() : ++currentBranch)
	{
		wasJoining = false;

		// currentBranch is std::set<QUuid>
		//

		// Take each id from the currentBranch,
		// try to find such branch where this id is present,
		// and join this branch to currentBranch
		//
		for (auto& id = currentBranch->begin(); id != currentBranch->end(); ++id)
		{
			auto subBranch = currentBranch;
			++subBranch;
			for (; subBranch != branches.end();)
			{
				if (std::find(subBranch->begin(), subBranch->end(), *id) != subBranch->end())
				{
					// Join found branch to currentBranch
					//
					for (auto& subBranchId : *subBranch)
					{
						currentBranch->insert(subBranchId);
					}

					// Delete subBrach, make tmp to delete it after interator increment
					//
					auto tmp = subBranch;
					++subBranch;

					branches.erase(tmp);

					wasJoining = true;
					continue;
				}

				++subBranch;
			}
		}

	}

	// DEBUG
	//
	qDebug() << "";
	qDebug() << "Branches after joining";

	for (std::set<QUuid>& b : branches)
	{
		qDebug() << "--";
		for (const QUuid& q : b)
		{
			qDebug() << q;
		}
	}

	qDebug() << "";


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

bool BuildWorkerThread::debug() const
{
	return m_debug;
}

void BuildWorkerThread::setDebug(bool value)
{
	m_debug = value;
}

bool BuildWorkerThread::release() const
{
	return !m_debug;
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
						   bool debug)
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
	m_thread->setDebug(debug);

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


