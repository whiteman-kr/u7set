#include "ConfigurationBuilder.h"

#include "../../include/DbController.h"
#include "../../include/OutputLog.h"
#include "../../include/DeviceObject.h"
#include "../../include/Crc.h"

namespace Builder
{

	// ------------------------------------------------------------------------
	//
	//		JsSignalSet
	//
	// ------------------------------------------------------------------------

	JsSignalSet::JsSignalSet(SignalSet* signalSet):
		m_signalSet(signalSet)
	{
		if (m_signalSet == nullptr)
		{
			assert(m_signalSet);
		}
	}

	QObject* JsSignalSet::getSignalByDeviceStrID(const QString& deviceStrID)
	{
		if (m_signalSet == nullptr)
		{
			assert(m_signalSet);
			return nullptr;
		}

		for (int i = 0; i < m_signalSet->count(); i++)
		{
			if ((*m_signalSet)[i].deviceStrID() == deviceStrID)
			{
				QObject* c = &(*m_signalSet)[i];
				QQmlEngine::setObjectOwnership(c, QQmlEngine::ObjectOwnership::CppOwnership);
				return c;
			}
		}
		return nullptr;
	}

	// ------------------------------------------------------------------------
	//
	//		ConfigurationBuilder
	//
	// ------------------------------------------------------------------------

	ConfigurationBuilder::ConfigurationBuilder(DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage* subsystems, OutputLog* log, int changesetId, bool debug, QString projectName, QString userName, BuildResultWriter* buildWriter):
		m_db(db),
		m_deviceRoot(deviceRoot),
		m_signalSet(signalSet),
		m_subsystems(subsystems),
		m_log(log),
		m_buildWriter(buildWriter),
		m_changesetId(changesetId),
		m_debug(debug),
		m_projectName(projectName),
		m_userName(userName)
	{
		assert(m_db);
		assert(m_deviceRoot);
		assert(m_signalSet);
		assert(m_subsystems);
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
			LOG_ERROR(m_log, tr("%1: Fatal error, input parammeter is nullptr!").arg(__FUNCTION__));
			return false;
		}

		//
		// Generate Module Confuiguration Binary File
		//
		LOG_MESSAGE(m_log, "");
		LOG_MESSAGE(m_log, tr("Generating modules configurations"));

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
			ok = db()->getFileList(&fileList, db()->mcFileId(), "ModulesConfigurations.descr", true, nullptr);
		}

		if (ok == false || fileList.size() != 1)
		{
			LOG_ERROR(m_log, tr("Can't get file list and find Module Configuration description file"));
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

		if (ok == false || scriptFile == nullptr)
		{
			LOG_ERROR(m_log, tr("Can't get Module Configuration description file"));
			return false;
		}

		QString contents = QString::fromLocal8Bit(scriptFile->data());

		// Attach objects
		//
		QJSEngine jsEngine;

		JsSignalSet jsSignalSet(m_signalSet);

		Hardware::ModuleFirmwareCollection confCollection(m_projectName, m_userName, m_changesetId);

		QJSValue jsRoot = jsEngine.newQObject(m_deviceRoot);
		QQmlEngine::setObjectOwnership(m_deviceRoot, QQmlEngine::CppOwnership);

		QJSValue jsConfCollection = jsEngine.newQObject(&confCollection);
		QQmlEngine::setObjectOwnership(&confCollection, QQmlEngine::CppOwnership);

		QJSValue jsLog = jsEngine.newQObject(m_log);
		QQmlEngine::setObjectOwnership(m_log, QQmlEngine::CppOwnership);

		QJSValue jsSignalSetObject = jsEngine.newQObject(&jsSignalSet);
		QQmlEngine::setObjectOwnership(&jsSignalSet, QQmlEngine::CppOwnership);

		QJSValue jsSubsystems = jsEngine.newQObject(m_subsystems);
		QQmlEngine::setObjectOwnership(m_subsystems, QQmlEngine::CppOwnership);


		// Run script
		//
		QJSValue jsEval = jsEngine.evaluate(contents, "ModulesConfigurations.descr");
		if (jsEval.isError() == true)
		{
			LOG_ERROR(m_log, tr("Module configuration script evaluation failed: %1").arg(jsEval.toString()));
			return false;
		}

		QJSValueList args;

		args << jsRoot;
		args << jsConfCollection;
		args << jsLog;
		args << jsSignalSetObject;
		args << jsSubsystems;

		QJSValue jsResult = jsEval.call(args);

		if (jsResult.isError() == true)
		{
			LOG_ERROR(m_log, tr("Uncaught exception while generating module configuration: %1").arg(jsResult.toString()));
			return false;
		}

		if (jsResult.toBool() == false)
		{
			LOG_ERROR(m_log, tr("Module configuration generating failed!"));
			return false;
		}
		qDebug() << jsResult.toInt();

		// Find all LM modules and save ssKey and channel information
		//
		std::vector<Hardware::DeviceModule*> lmModules;
		findLmModules(m_deviceRoot, lmModules);

		QStringList lmReport;
		lmReport << "Jumpers configuration for LM modules";

		for (Hardware::DeviceModule* m : lmModules)
		{
			int ssKey = m_subsystems->ssKey(m->subSysID());

			lmReport << "\r\n";
			lmReport << "StrID: " + m->strId();
			lmReport << "Caption: " + m->caption();
			lmReport << "Place: " + QString::number(m->place());
			lmReport << "Subsystem ID: " + m->subSysID();
			lmReport << "Subsystem code: " + QString::number(ssKey);
			lmReport << "Channel: " + QString::number(m->channel());

			quint16 jumpers = ssKey << 6;
			jumpers |= m->channel();

            quint16 crc4 = Crc::crc4(jumpers);
			jumpers |= (crc4 << 12);

			lmReport << "Jumpers configuration (HEX): 0x" + QString::number(jumpers, 16);

			QString jumpersHex = QString::number(jumpers, 2).rightJustified(16, '0');
			jumpersHex.insert(4, ' ');
			jumpersHex.insert(9, ' ');
			jumpersHex.insert(14, ' ');
			lmReport << "Jumpers configuration (BIN): " + jumpersHex;
		}

		QByteArray lmReportData;
		for (QString s : lmReport)
		{
			lmReportData.append(s + "\r\n");
		}

		if (m_buildWriter->addFile("Reports", "lmJumpers.txt", lmReportData) == false)
		{
			LOG_ERROR(m_log, tr("Failed to save lmJumpers.txt file!"));
			return false;
		}

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
				Hardware::ModuleFirmware& f = i->second;

				QByteArray data;

				QString errorMsg;
				if (f.save(data, &errorMsg) == false)
				{
					LOG_ERROR(m_log, errorMsg);
					return false;
				}

				QString path = f.subsysId();
				QString fileName = f.caption();

				if (path.isEmpty())
				{
					LOG_ERROR(m_log, tr("Failed to save module configuration output file, subsystemId is empty."));
					return false;
				}
				if (fileName.isEmpty())
				{
					LOG_ERROR(m_log, tr("Failed to save module configuration output file, module type string is empty."));
					return false;
				}

				if (m_buildWriter->addFile(path, fileName + ".mcb", data) == false)
				{
					LOG_ERROR(m_log, tr("Failed to save module configuration output file for") + f.subsysId() + ", " + f.caption() + "!");
					return false;
				}

                if (m_buildWriter->addFile(path, fileName + ".mct", f.log()) == false)
                {
                    LOG_ERROR(m_log, tr("Failed to save module configuration output log file for") + f.subsysId() + ", " + f.caption() + "!");
                    return false;
                }
            }
		}

		return true;
	}

	void ConfigurationBuilder::findLmModules(Hardware::DeviceObject* object, std::vector<Hardware::DeviceModule *> &modules)
	{
		if (object == nullptr)
		{
			assert(object);
			return;
		}

		for (int i = 0; i < object->childrenCount(); i++)
		{
			Hardware::DeviceObject* child = object->child(i);

			if (child->deviceType() == Hardware::DeviceType::Module)
			{
				Hardware::DeviceModule* module = dynamic_cast<Hardware::DeviceModule*>(child);

				if (module->moduleFamily() == Hardware::DeviceModule::LM)
				{
					modules.push_back(module);
				}
			}

			findLmModules(child, modules);
		}

		return;
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
