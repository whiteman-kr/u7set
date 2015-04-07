#include "Builder.h"
#include "ApplicationLogicBuilder.h"

#include "../../include/DbController.h"
#include "../../include/OutputLog.h"
#include "../../include/DeviceObject.h"

#include "../../VFrame30/LogicScheme.h"
#include "../../VFrame30/VideoItemLink.h"
#include "../../VFrame30/HorzVertLinks.h"

namespace Builder
{
	// ------------------------------------------------------------------------
	//
	//		BuildWorkerThread
	//
	// ------------------------------------------------------------------------

	void BuildWorkerThread::run()
	{
		QThread::currentThread()->setTerminationEnabled(true);

		// moved to m_buildWriter.start
		//
		//qDebug() << "Building started";

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

#pragma message("Load correct ChangesetID")
		m_buildWriter.start(&db, m_log, release(), 0 /* Load correct ChangesetID */);

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
			m_log->writeMessage(tr("Application Logic compilation"), true);

			ok = applicationLogic(&db, lastChangesetId);

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

		m_buildWriter.finish();

		//moved to m_buildWriter.finish
		//
		// Closing project and saying bye-bye!
		//
		/*ok = db.closeProject(nullptr);

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
		}*/

		emit resultReady(QString("Cool, we've done!"));

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
		signalSetObject.loadSignalsFromDb(db);
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

	bool BuildWorkerThread::applicationLogic(DbController* db, int changesetId)
	{
		if (db == nullptr)
		{
			assert(false);
			return false;
		}

		ApplicationLogicBuilder alBuilder = {db, m_log, changesetId, debug()};

		bool result = alBuilder.build();

		return result;
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

	// ------------------------------------------------------------------------
	//
	//		SignalSetObject
	//
	// ------------------------------------------------------------------------

	void SignalSetObject::loadSignalsFromDb(DbController* db)
	{
		db->getSignals(&m_signalSet, nullptr);
	}

	QObject* SignalSetObject::getSignalByDeviceStrID(const QString& deviceStrID)
	{
		for (int i = 0; i < m_signalSet.count(); i++)
		{
			if (m_signalSet[i].deviceStrID() == deviceStrID)
			{
				QObject* c = &m_signalSet[i];
				QQmlEngine::setObjectOwnership(c, QQmlEngine::ObjectOwnership::CppOwnership);
				return c;
			}
		}
		return nullptr;
	}

	// ------------------------------------------------------------------------
	//
	//		Builder
	//
	// ------------------------------------------------------------------------


	Builder::Builder(OutputLog* log) :
		m_log(log)
	{
		assert(m_log != nullptr);

		m_thread = new BuildWorkerThread();
		m_thread->setObjectName(tr("BuildWorkerThread"));
		m_thread->setOutputLog(m_log);

		connect(m_thread, &BuildWorkerThread::resultReady, this, &Builder::handleResults);

		connect(m_thread, &BuildWorkerThread::started, this, &Builder::buildStarted);
		connect(m_thread, &BuildWorkerThread::finished, this, &Builder::buildFinished);

		return;
	}

	Builder::~Builder()
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

	bool Builder::start(QString projectName,
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

	void Builder::stop()
	{
		m_thread->requestInterruption();
	}

	bool Builder::isRunning() const
	{
		bool result = m_thread->isRunning();
		return result;
	}

	void Builder::handleResults(QString /*result*/)
	{
	}

}
