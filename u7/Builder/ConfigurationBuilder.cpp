#include "ConfigurationBuilder.h"

#include "../../include/DbController.h"
#include "../../include/OutputLog.h"
#include "../../include/DeviceObject.h"

namespace Builder
{

	ConfigurationBuilder::ConfigurationBuilder(DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSetObject* signalSetObject, OutputLog* log, int changesetId, bool debug, QString projectName, QString userName, BuildResultWriter* buildWriter):
		m_db(db),
		m_deviceRoot(deviceRoot),
		m_signalSetObject(signalSetObject),
		m_log(log),
		m_changesetId(changesetId),
		m_debug(debug),
		m_projectName(projectName),
		m_userName(userName),
		m_buildWriter(buildWriter)
	{
		assert(m_db);
		assert(m_deviceRoot);
		assert(m_signalSetObject);
		assert(m_log);
		assert(m_buildWriter);

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
		// Generate Module Confuiguration Binary File
		//
		m_log->writeMessage("", false);
		m_log->writeMessage(tr("Generating modules configurations"), true);

		bool ok = false;


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

		QJSValue jsSignalSet = jsEngine.newQObject(m_signalSetObject);
		QQmlEngine::setObjectOwnership(m_signalSetObject, QQmlEngine::CppOwnership);

		QJSValue jsRoot = jsEngine.newQObject(m_deviceRoot);
		QQmlEngine::setObjectOwnership(m_deviceRoot, QQmlEngine::CppOwnership);

		Hardware::ModuleConfCollection confCollection(m_projectName, m_userName, m_changesetId);

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
			for (auto i = confCollection.firmwares().begin(); i != confCollection.firmwares().end(); i++)
			{
				const Hardware::ModuleConfFirmware& f = i->second;

				QByteArray data;
				f.save(data);

				if (m_buildWriter->addFile(f.subsysId(), f.type() + ".mcb", data) == false)
				{
					m_log->writeError(tr("Failed to save module configuration output file for") + f.subsysId() + ", " + f.type() + "!", false, true);
					return false;
				}
			}
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
