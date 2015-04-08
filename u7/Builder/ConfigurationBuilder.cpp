#include "ConfigurationBuilder.h"

#include "../../include/DbController.h"
#include "../../include/OutputLog.h"
#include "../../include/DeviceObject.h"

#include "Builder.h"

namespace Builder
{

	ConfigurationBuilder::ConfigurationBuilder(DbController* db, OutputLog* log,
		int changesetId, bool debug, QString projectName, QString userName) :
		m_db(db),
		m_log(log),
		m_changesetId(changesetId),
		m_debug(debug),
		m_projectName(projectName),
		m_userName(userName)
	{
		assert(m_db);
		assert(m_log);

		return;
	}

	ConfigurationBuilder::~ConfigurationBuilder()
	{
	}

	bool ConfigurationBuilder::build()
	{
		if (db() == nullptr || log() == nullptr)
		{
			assert(db());
			assert(log());
			m_log->writeError(tr("%1: Fatal error, input parammeter is nullptr!").arg(__FUNCTION__), true,true);
			return false;
		}

		//
		// Get Equipment from the database
		//
		m_log->writeMessage("", false);
		m_log->writeMessage(tr("Getting equipment"), true);

		Hardware::DeviceRoot deviceRoot;
		int rootFileId = db()->hcFileId();
		deviceRoot.fileInfo().setFileId(rootFileId);

		bool ok = getEquipment(db(), &deviceRoot);

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			return false;
		}

		if (ok == false)
		{
			m_log->writeError(tr("Error"), true, false);
			QThread::currentThread()->requestInterruption();
			return false;
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
		// Generate Module Confuiguration Binary File
		//
		m_log->writeMessage("", false);
		m_log->writeMessage(tr("Generating modules configurations"), true);


		// Get script file from the project databse
		//
		std::vector<DbFileInfo> fileList;

		if (release() == true)
		{
			assert(false);
		}
		else
		{
			ok = db()->getFileList(&fileList, db()->mcFileId(), "ModulesConfigurations.descr", nullptr);
		}

		if (ok == false || fileList.size() != 1)
		{
			m_log->writeError(tr("Can't get file list and find Module Configuration description file"), false, true);
			return false;
		}

		std::shared_ptr<DbFile> scriptFile;

		if (release() == true)
		{
			assert(false);
		}
		else
		{
			ok = db()->getLatestVersion(fileList[0], &scriptFile, nullptr);
		}

		if (ok == false || scriptFile == false)
		{
			m_log->writeError(tr("Can't get Module Configuration description file"), false, true);
			return false;
		}

		QString contents = QString::fromLocal8Bit(scriptFile->data());

		// Attach objects
		//
		QJSEngine jsEngine;

		QJSValue jsLog = jsEngine.newQObject(m_log);
		QQmlEngine::setObjectOwnership(m_log, QQmlEngine::CppOwnership);

		SignalSetObject signalSetObject;
		signalSetObject.loadSignalsFromDb(db());
		QJSValue jsSignalSet = jsEngine.newQObject(&signalSetObject);
		QQmlEngine::setObjectOwnership(&signalSetObject, QQmlEngine::CppOwnership);

		QJSValue jsRoot = jsEngine.newQObject(&deviceRoot);
		QQmlEngine::setObjectOwnership(&deviceRoot, QQmlEngine::CppOwnership);

		Hardware::ModuleConfCollection confCollection;

		QJSValue jsConfCollection = jsEngine.newQObject(&confCollection);
		QQmlEngine::setObjectOwnership(&confCollection, QQmlEngine::CppOwnership);

		// Run script
		//
		QJSValue jsEval = jsEngine.evaluate(contents, "ModulesConfigurations.descr");
		if (jsEval.isError() == true)
		{
			m_log->writeError(tr("Module configuration script evaluation failed: %1").arg(jsEval.toString()), false, true);
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
			m_log->writeError(tr("Uncaught exception while generating module configuration: %1").arg(jsResult.toString()), false, true);
			return false;
		}

		if (jsResult.toBool() == false)
		{
			m_log->writeError(tr("Module configuration generating failed!"), false, false);
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
			if (confCollection.save(m_projectName, m_userName) == false)
			{
				m_log->writeError(tr("Failed to save module configuration binary files!"), false, true);
				return false;
			}
		}

		return true;
	}

	bool ConfigurationBuilder::getEquipment(DbController* db, Hardware::DeviceObject* parent)
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

	bool ConfigurationBuilder::expandDeviceStrId(Hardware::DeviceObject* device)
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

	DbController* ConfigurationBuilder::db()
	{
		return m_db;
	}

	OutputLog* ConfigurationBuilder::log() const
	{
		return m_log;
	}

	int ConfigurationBuilder::changesetId() const
	{
		return m_changesetId;
	}

	bool ConfigurationBuilder::debug() const
	{
		return m_debug;
	}

	bool ConfigurationBuilder::release() const
	{
		return !debug();
	}

}
