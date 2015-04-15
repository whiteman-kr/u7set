#include "Builder.h"
#include "ApplicationLogicBuilder.h"
#include "ConfigurationBuilder.h"

#include "../../include/DbController.h"
#include "../../include/OutputLog.h"
#include "../../include/DeviceObject.h"

#include "../../VFrame30/LogicScheme.h"
#include "../../VFrame30/VideoItemLink.h"
#include "../../VFrame30/HorzVertLinks.h"

#include "../Builder/ApplicationLogicCompiler.h"

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
			m_log->writeError(db.lastError(), false, true);
			m_log->writeError(tr("Opening project %1: error").arg(projectName()), true, false);
			return;
		}
		else
		{
			m_log->writeMessage(tr("Opening project %1: ok").arg(projectName()), true);
		}

		BuildResultWriter buildWriter;

#pragma message("################################ Load correct ChangesetID")
		buildWriter.start(&db, m_log, release(), 0 /* Load correct ChangesetID */);

		do
		{
			int lastChangesetId = 0;
			ok = db.lastChangesetId(&lastChangesetId);

			if (ok == false)
			{
				m_log->writeError(tr("lastChangesetId Error."), true, true);
				break;
			}

			bool isAnyCheckedOut = false;
			ok = db.isAnyCheckedOut(&isAnyCheckedOut);

			if (ok == false)
			{
				m_log->writeError(tr("isAnyCheckedOut Error."), true, true);
				QThread::currentThread()->requestInterruption();
				break;
			}

			if (release() == true && isAnyCheckedOut == true)
			{
				m_log->writeError(tr("There are some checked out objects. Please check in all objects before building release version."), true, true);
				QThread::currentThread()->requestInterruption();
				break;
			}

			//
			// Get Equipment from the database
			//
			m_log->writeMessage("", false);
			m_log->writeMessage(tr("Getting equipment"), true);

			Hardware::DeviceRoot deviceRoot;
			int rootFileId = db.hcFileId();
			deviceRoot.fileInfo().setFileId(rootFileId);

			bool ok = getEquipment(&db, &deviceRoot);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			if (ok == false)
			{
				m_log->writeError(tr("Error"), true, false);
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
			m_log->writeMessage("", false);
			m_log->writeMessage(tr("Expanding devices StrIds"), true);

			expandDeviceStrId(&deviceRoot);

			m_log->writeSuccess(tr("Ok"), true);

			//
			// Get signals from the database
			//
			SignalSetObject signalSetObject;
			signalSetObject.loadSignalsFromDb(&db);

			//
			// Compile Module configuration
			//
			m_log->writeMessage("", false);
			m_log->writeMessage(tr("Module configurations compilation"), true);

			ok = modulesConfiguration(&db, &deviceRoot, &signalSetObject, lastChangesetId, &buildWriter);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			if (ok == false)
			{
				m_log->writeError(tr("Error"), true, false);
				QThread::currentThread()->requestInterruption();
				break;
			}
			else
			{
				m_log->writeSuccess(tr("Ok"), true);
			}

			//
			// Build application logic
			//
			buildApplicationLogic(&db, lastChangesetId);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Compile application logic
			//
			compileApplicationLogic(&deviceRoot, &SignalSet(), &buildWriter);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}
		}
		while (false);

		buildWriter.finish();

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
			m_log->writeMessage(tr("Getting system %1...").arg(parent->caption()), false);
		}

		std::vector<DbFileInfo> files;

		bool ok = false;

		// Get file list with checked out files,
		// if this is release build, specific copies will be fetched later
		//
		ok = db->getFileList(&files, parent->fileInfo().fileId(), nullptr);

		if (ok == false)
		{
			m_log->writeError(tr("Cannot get equipment file list"), false, true);
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
				m_log->writeError(tr("Cannot get %1 instance.").arg(fi.fileName()), false, true);
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

	bool BuildWorkerThread::modulesConfiguration(DbController* db, Hardware::DeviceRoot *deviceRoot, SignalSetObject* signalSetObject, int changesetId, BuildResultWriter* buildWriter)
	{
		if (db == nullptr)
		{
			assert(false);
			return false;
		}

		ConfigurationBuilder cfgBuilder = {db, deviceRoot, signalSetObject, m_log, changesetId, debug(), projectName(), projectUserName(), buildWriter};

		bool result = cfgBuilder.build();

		return result;

	}

	bool BuildWorkerThread::buildApplicationLogic(DbController* db, int changesetId)
	{
		if (db == nullptr)
		{
			assert(false);
			return false;
		}

		m_log->writeMessage("", false);
		m_log->writeMessage(tr("Application Logic building"), true);

		ApplicationLogicBuilder alBuilder = {db, m_log, changesetId, debug()};

		bool result = alBuilder.build();

		if (result == false)
		{
			m_log->writeError(tr("Error"), true, false);
			QThread::currentThread()->requestInterruption();
		}
		else
		{
			m_log->writeSuccess(tr("Ok"), true);
		}

		return result;
	}


	bool BuildWorkerThread::compileApplicationLogic(Hardware::DeviceObject* equipment, SignalSet* signalSet, BuildResultWriter* buildResultWriter)
	{
		m_log->writeMessage("", false);
		m_log->writeMessage(tr("Application Logic compilation"), true);

		ApplicationLogicCompiler appLogicCompiler(equipment, signalSet, buildResultWriter, m_log);

		bool result = appLogicCompiler.run();

		if (result == false)
		{
			m_log->writeError(tr("Error"), true, false);
			QThread::currentThread()->requestInterruption();
		}
		else
		{
			m_log->writeSuccess(tr("Ok"), true);
		}

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
