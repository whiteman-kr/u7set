#include "ConfigurationBuilder.h"

#include "../../lib/DbController.h"
#include "Builder/IssueLogger.h"
#include "../../lib/DeviceObject.h"
#include "Connection.h"
#include "../../lib/Crc.h"
#include "../lib/SignalProperties.h"

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
				SignalProperties* sp = new SignalProperties((*m_signalSet)[i]);
				return sp;

				//QObject* c = &(*m_signalSet)[i];
				//QQmlEngine::setObjectOwnership(c, QQmlEngine::ObjectOwnership::CppOwnership);
				//return c;
			}
		}
		return nullptr;
	}

	// ------------------------------------------------------------------------
	//
	//		ConfigurationBuilder
	//
	// ------------------------------------------------------------------------

	ConfigurationBuilder::ConfigurationBuilder(BuildWorkerThread* buildWorkerThread, DbController* db, Hardware::DeviceRoot* deviceRoot,
											   const std::vector<Hardware::DeviceModule*>& lmModules, LmDescriptionSet *lmDescriptions, SignalSet* signalSet,
											   Hardware::SubsystemStorage* subsystems, Hardware::OptoModuleStorage *opticModuleStorage,
											   IssueLogger *log, int buildNo, int changesetId, bool debug, QString projectName, QString userName):
		m_buildWorkerThread(buildWorkerThread),
		m_db(db),
		m_deviceRoot(deviceRoot),
		m_lmModules(lmModules),
		m_lmDescriptions(lmDescriptions),
		m_signalSet(signalSet),
		m_subsystems(subsystems),
		m_opticModuleStorage(opticModuleStorage),
		m_log(log),
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

		return;
	}

	ConfigurationBuilder::~ConfigurationBuilder()
	{
	}

	bool ConfigurationBuilder::build(BuildResultWriter &buildResultWriter)
	{
		if (db() == nullptr || log() == nullptr)
		{
			assert(db());
			assert(log());
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("%1: Fatal error, input parameter is nullptr!").arg(__FUNCTION__));
			return false;
		}

		//
		// Generate Module Confuiguration Binary File
		//
		LOG_MESSAGE(m_log, "");
		LOG_MESSAGE(m_log, tr("Generating modules configurations"));

		int subsystemsCount = m_subsystems->count();

		// Check if logic modules have unknown subsystems
		//
		for (auto it = m_lmModules.begin(); it != m_lmModules.end(); it++)
		{
			Hardware::DeviceModule* lm = *it;
			if (lm == nullptr)
			{
				assert(lm);
				return false;
			}

			if (lm->propertyExists("SubsystemID") == false)
			{
				m_log->errCFG3000("SubsystemID", lm->equipmentId());
				return false;
			}

			QString subsystemID = lm->propertyValue("SubsystemID").toString();

			bool subsystemFound = false;

			for (int i = 0; i < subsystemsCount; i++)
			{
				std::shared_ptr<Hardware::Subsystem> subsystem = m_subsystems->get(i);
				if (subsystem == nullptr)
				{
					assert(subsystem);
					return false;
				}

				if (subsystem->subsystemId() == subsystemID)
				{
					subsystemFound = true;
					break;
				}
			}

			if (subsystemFound == false)
			{
				m_log->errCFG3001(subsystemID, lm->equipmentId());
				return false;
			}
		}

		// Find Logic modules for each subsystem and execute configuration script for each subsystem
		//

		for (int i = 0; i < subsystemsCount; i++)
		{
			std::shared_ptr<Hardware::Subsystem> subsystem = m_subsystems->get(i);
			if (subsystem == nullptr)
			{
				assert(subsystem);
				return false;
			}

			std::vector<Hardware::DeviceModule*> subsystemModules;

			std::vector<LmDescription*> subsystemModulesDescriptions;

			for (auto it = m_lmModules.begin(); it != m_lmModules.end(); it++)
			{
				Hardware::DeviceModule* lm = *it;
				if (lm == nullptr)
				{
					assert(lm);
					return false;
				}

				if (lm->propertyExists("SubsystemID") == false)
				{
					m_log->errCFG3000("SubsystemID", lm->equipmentId());
					return false;
				}

				if (lm->propertyExists("LmDescriptionFile") == false)
				{
					m_log->errCFG3000("LmDescriptionFile", lm->equipmentId());
					return false;
				}

				QString subsystemID = lm->propertyValue("SubsystemID").toString();

				if (subsystemID == subsystem->subsystemId())
				{
					// Add a module for this subsystem
					//
					subsystemModules.push_back(lm);

					LmDescription* description = m_lmDescriptions->get(lm).get();

					if (description == nullptr)
					{
						m_log->errEQP6004(lm->equipmentIdTemplate(), LogicModuleSet::lmDescriptionFile(lm), lm->uuid());
						return false;
					}

					if (std::find(subsystemModulesDescriptions.begin(), subsystemModulesDescriptions.end(), description) == subsystemModulesDescriptions.end())
					{
						subsystemModulesDescriptions.push_back(description);
					}
				}
			}

			if (subsystemModules.empty() == true)
			{
				continue;
			}

			if (subsystemModulesDescriptions.empty() == true)
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("%1: Fatal error, Logic Modules descriptions for subsystem %2 is undefined!").arg(__FUNCTION__).arg(subsystem->caption()));
				return false;
			}

			for (LmDescription* logicModuleDescription : subsystemModulesDescriptions)
			{
				if (runConfigurationScriptFile(subsystemModules, logicModuleDescription) == false)
				{
					return false;
				}
			}
		}

		// Find all LM modules and save ssKey and channel information
		//

		std::sort(m_lmModules.begin(), m_lmModules.end(),
				  [](const Hardware::DeviceModule* a, const Hardware::DeviceModule* b) -> bool
		{
			return a->equipmentIdTemplate() < b->equipmentIdTemplate();
		});

		QStringList lmReport;
		lmReport << "Jumpers configuration for LM modules";

		for (Hardware::DeviceModule* m : m_lmModules)
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

		if (buildResultWriter.addFile("Reports", "lmJumpers.txt", lmReportData) == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Failed to save lmJumpers.txt file!"));
			return false;
		}

		return true;
	}

	bool ConfigurationBuilder::runConfigurationScriptFile(const std::vector<Hardware::DeviceModule*>& subsystemModules, LmDescription* logicModuleDescription)
	{
		bool ok = false;

		// Get script file from the project databse
		//
		std::vector<DbFileInfo> fileList;

		ok = db()->getFileList(&fileList, db()->mcFileId(), logicModuleDescription->configurationStringFile(), true, nullptr);

		if (ok == false || fileList.size() != 1)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined,
							   tr("Can't get file list and find module configuration description file '%1'").arg(logicModuleDescription->configurationStringFile()));
			return false;
		}

		std::shared_ptr<DbFile> scriptFile;
		ok = db()->getLatestVersion(fileList[0], &scriptFile, nullptr);

		if (ok == false || scriptFile == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined,
							   tr("Can't get module configuration description file %1").arg(logicModuleDescription->configurationStringFile()));
			return false;
		}

		QString contents = QString::fromLocal8Bit(scriptFile->data());

		// Attach objects
		//
		m_jsEngine.installExtensions(QJSEngine::ConsoleExtension);

		JsSignalSet jsSignalSet(m_signalSet);

		m_confCollection.init(m_projectName, m_userName, buildNo(), debug(), changesetId());

		QJSValue jsBuilder = m_jsEngine.newQObject(this);
		QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

		QJSValue jsRoot = m_jsEngine.newQObject(m_deviceRoot);
		QQmlEngine::setObjectOwnership(m_deviceRoot, QQmlEngine::CppOwnership);

		QJSValue jsLogicModules = m_jsEngine.newArray((int)subsystemModules.size());
		for (int i = 0; i < subsystemModules.size(); i++)
		{
			assert(jsLogicModules.isArray());

			QJSValue module = m_jsEngine.newQObject(subsystemModules[i]);
			QQmlEngine::setObjectOwnership(subsystemModules[i], QQmlEngine::CppOwnership);

			jsLogicModules.setProperty(i, module);
		}

		QJSValue jsConfCollection = m_jsEngine.newQObject(&m_confCollection);
		QQmlEngine::setObjectOwnership(&m_confCollection, QQmlEngine::CppOwnership);

		QJSValue jsLog = m_jsEngine.newQObject(m_log);
		QQmlEngine::setObjectOwnership(m_log, QQmlEngine::CppOwnership);

		QJSValue jsSignalSetObject = m_jsEngine.newQObject(&jsSignalSet);
		QQmlEngine::setObjectOwnership(&jsSignalSet, QQmlEngine::CppOwnership);

		QJSValue jsSubsystems = m_jsEngine.newQObject(m_subsystems);
		QQmlEngine::setObjectOwnership(m_subsystems, QQmlEngine::CppOwnership);

		QJSValue jsOpticModuleStorage = m_jsEngine.newQObject(m_opticModuleStorage);
		QQmlEngine::setObjectOwnership(m_opticModuleStorage, QQmlEngine::CppOwnership);

		QJSValue jsLogicModuleDescription = m_jsEngine.newQObject(logicModuleDescription);
		QQmlEngine::setObjectOwnership(logicModuleDescription, QQmlEngine::CppOwnership);

		// Run script
		//

		QJSValue jsEval = m_jsEngine.evaluate(contents);
		if (jsEval.isError() == true)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Module configuration script '%1' evaluation failed at line %2: %3").arg(logicModuleDescription->configurationStringFile()).arg(jsEval.property("lineNumber").toInt()).arg(jsEval.toString()));
			return false;
		}

		if (!m_jsEngine.globalObject().hasProperty("main"))
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Script has no \"main\" function"));
			return false;
		}

		if (!m_jsEngine.globalObject().property("main").isCallable())
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("\"main\" property of script is not callable"));
			return false;
		}

		QJSValueList args;

		args << jsBuilder;
		args << jsRoot;
		args << jsLogicModules;
		args << jsConfCollection;
		args << jsLog;
		args << jsSignalSetObject;
		args << jsSubsystems;
		args << jsOpticModuleStorage;
		args << jsLogicModuleDescription;

		QJSValue jsResult = m_jsEngine.globalObject().property("main").call(args);

		if (jsResult.isError() == true)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Uncaught exception while generating module configuration '%1': %2").arg(logicModuleDescription->configurationStringFile()).arg(jsResult.toString()));
			return false;
		}

		if (jsResult.toBool() == false)
		{
			return false;
		}

		return true;
	}


	bool ConfigurationBuilder::writeBinaryFiles(BuildResultWriter &buildResultWriter)
	{
		// Save confCollection items to binary files
		//
		for (auto i = m_confCollection.firmwares().begin(); i != m_confCollection.firmwares().end(); i++)
		{
			Hardware::ModuleFirmwareWriter& f = i->second;

			QByteArray data;

			if (f.save(data, m_log) == false)
			{
				return false;
			}

			if (f.subsysId().isEmpty())
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Failed to save module configuration output file, subsystemId is empty."));
				return false;
			}

			if (buildResultWriter.addFile(f.subsysId(), f.subsysId().toLower() + ".mcb", data) == nullptr)
			{
				return false;
			}

			if (buildResultWriter.addFile(f.subsysId(), f.subsysId().toLower() + ".mct", f.scriptLog()) == nullptr)
			{
				return false;
			}
		}

		return true;
	}

	int ConfigurationBuilder::jsBuildNo()
	{
		return buildNo();
	}

	bool ConfigurationBuilder::jsIsInterruptRequested()
	{
		if(m_buildWorkerThread == nullptr)
		{
			assert(m_buildWorkerThread);
			return false;
		}

		return m_buildWorkerThread->isInterruptRequested();
	}

	quint64 ConfigurationBuilder::getFirmwareUniqueId(const QString &subsystemID, int lmNumber)
	{
		return m_confCollection.getFirmwareUniqueId(subsystemID, lmNumber);
	}

	void ConfigurationBuilder::setGenericUniqueId(const QString& subsystemID, int lmNumber, quint64 genericUniqueId)
	{
		m_confCollection.setGenericUniqueId(subsystemID, lmNumber, genericUniqueId);
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
