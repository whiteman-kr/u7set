#include "ConfigurationBuilder.h"

#include "../../lib/DbController.h"
#include "Builder/IssueLogger.h"
#include "../../lib/DeviceObject.h"
#include "Connection.h"
#include "../../lib/Crc.h"

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

	QObject* JsSignalSet::getSignalByEquipmentID(const QString& equpmentID)
	{
		if (m_signalSet == nullptr)
		{
			assert(m_signalSet);
			return nullptr;
		}

		for (int i = 0; i < m_signalSet->count(); i++)
		{
			if ((*m_signalSet)[i].equipmentID() == equpmentID)
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

	ConfigurationBuilder::ConfigurationBuilder(BuildWorkerThread* buildWorkerThread, DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet,
											   Hardware::SubsystemStorage* subsystems, Hardware::OptoModuleStorage *opticModuleStorage,
											   IssueLogger *log, int buildNo, int changesetId, bool debug, QString projectName, QString userName,
											   BuildResultWriter* buildWriter):
		m_buildWorkerThread(buildWorkerThread),
		m_db(db),
		m_deviceRoot(deviceRoot),
		m_signalSet(signalSet),
		m_subsystems(subsystems),
		m_opticModuleStorage(opticModuleStorage),
		m_log(log),
		m_buildWriter(buildWriter),
		m_buildNo(buildNo),
		m_changesetId(changesetId),
		m_debug(debug),
		m_projectName(projectName),
		m_userName(userName)
	{
		assert(m_db);
		assert(m_deviceRoot);
		assert(m_signalSet);
		assert(m_subsystems);
		assert(m_opticModuleStorage);
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
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("%1: Fatal error, input parammeter is nullptr!").arg(__FUNCTION__));
			return false;
		}

		//
		// Generate Module Confuiguration Binary File
		//
		LOG_MESSAGE(m_log, "");
		LOG_MESSAGE(m_log, tr("Generating modules configurations"));

		bool ok = false;

		// Check if connections' identifiers (non-empty) exist in the database
		//
		/*for (int i = 0; i < m_connections->count(); i++)
		{
			std::shared_ptr<Hardware::Connection> connection = m_connections->get(i);

			std::vector<Hardware::DeviceObject*> list;

			if (connection->port1StrID().length() > 0)
			{
				list.clear();
				m_deviceRoot->findChildObjectsByMask(connection->port1StrID(), list);
				if (list.empty() == true)
				{
					LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
							  tr("No source port %1 was found for connection %2").arg(connection->port1StrID()).arg(connection->caption()));
					return false;
				}
			}

			if (connection->mode() == Hardware::OptoPort::Mode::Optical)
			{
				if (connection->port2StrID().length() > 0)
				{
					list.clear();
					m_deviceRoot->findChildObjectsByMask(connection->port2StrID(), list);
					if (list.empty() == true)
					{
						LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
								  tr("No target port %1 was found for connection %2").arg(connection->port2StrID()).arg(connection->caption()));
						return false;
					}
				}
			}
		}*/


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
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  tr("Can't get file list and find Module Configuration description file"));
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
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined,
					  tr("Can't get Module Configuration description file"));
			return false;
		}

		QString contents = QString::fromLocal8Bit(scriptFile->data());

		// Attach objects
		//
		QJSEngine jsEngine;

		//qmlRegisterType<MySliderItem>("com.mycompany.qmlcomponents", 1, 0, "Slider");


		JsSignalSet jsSignalSet(m_signalSet);

		Hardware::ModuleFirmwareCollection confCollection(m_projectName, m_userName, buildNo(), debug(), changesetId());

		QJSValue jsThread = jsEngine.newQObject(m_buildWorkerThread);
		QQmlEngine::setObjectOwnership(m_buildWorkerThread, QQmlEngine::CppOwnership);

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

		QJSValue jsOpticModuleStorage = jsEngine.newQObject(m_opticModuleStorage);
		QQmlEngine::setObjectOwnership(m_opticModuleStorage, QQmlEngine::CppOwnership);

		// Run script
		//
		QJSValue jsEval = jsEngine.evaluate(contents, "ModulesConfigurations.descr");
		if (jsEval.isError() == true)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("Module configuration script evaluation failed: %1").arg(jsEval.toString()));
			return false;
		}

		QJSValueList args;

		args << jsThread;
		args << jsRoot;
		args << jsConfCollection;
		args << jsLog;
		args << jsSignalSetObject;
		args << jsSubsystems;
		args << jsOpticModuleStorage;

		QJSValue jsResult = jsEval.call(args);

		if (jsResult.isError() == true)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("Uncaught exception while generating module configuration: %1").arg(jsResult.toString()));
			return false;
		}

		if (jsResult.toBool() == false)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("Module configuration generating failed!"));
			return false;
		}
		qDebug() << jsResult.toInt();

		// Find all LM modules and save ssKey and channel information
		//
		std::vector<Hardware::DeviceModule*> lmModules;
		findLmModules(m_deviceRoot, lmModules);
		std::sort(lmModules.begin(), lmModules.end(),
				  [](const Hardware::DeviceModule* a, const Hardware::DeviceModule* b) -> bool
				  {
					  return a->equipmentIdTemplate() < b->equipmentIdTemplate();
				  });

		QStringList lmReport;
		lmReport << "Jumpers configuration for LM modules";

		for (Hardware::DeviceModule* m : lmModules)
		{
			if (m->propertyExists("SubsystemID") == false)
			{
				lmReport << "No SubsystemID property found in " + m->equipmentIdTemplate();
				assert(false);
				continue;
			}

			if (m->propertyExists("LMNumber") == false)
			{
				lmReport << "No LMNumber property found in " + m->equipmentIdTemplate();
				assert(false);
				continue;
			}

			int ssKey = m_subsystems->ssKey(m->propertyValue("SubsystemID").toString());
			int lmNumber = m->propertyValue("LMNumber").toInt();

			lmReport << "\r\n";
			lmReport << "StrID: " + m->equipmentIdTemplate();
			lmReport << "Caption: " + m->caption();
			lmReport << "Place: " + QString::number(m->place());
			lmReport << "Subsystem ID: " + m->propertyValue("SubsystemID").toString();
			lmReport << "Subsystem code: " + QString::number(ssKey);
			lmReport << "LM Number: " + QString::number(lmNumber);

			quint16 jumpers = ssKey << 6;
			jumpers |= lmNumber;

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
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("Failed to save lmJumpers.txt file!"));
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
				Hardware::ModuleFirmwareWriter& f = i->second;

				QByteArray data;

				if (f.save(data, m_log) == false)
				{
					return false;
				}

				QString path = f.subsysId();
				QString fileName = f.caption();

				if (path.isEmpty())
				{
					LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("Failed to save module configuration output file, subsystemId is empty."));
					return false;
				}
				if (fileName.isEmpty())
				{
					LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("Failed to save module configuration output file, module type string is empty."));
					return false;
				}

				if (m_buildWriter->addFile(path, fileName + ".mcb", data) == false)
				{
					return false;
				}

				if (m_buildWriter->addFile(path, fileName + ".mct", f.log()) == false)
				{
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

	IssueLogger *ConfigurationBuilder::log() const
	{
		return m_log;
	}

	int ConfigurationBuilder::buildNo() const
	{
		return m_buildNo;
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
