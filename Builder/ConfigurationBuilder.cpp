#include "ConfigurationBuilder.h"

#include "../DbLib/DbController.h"
#include "../HardwareLib/DeviceObject.h"
#include "../HardwareLib/ScriptDeviceObject.h"
#include "../UtilsLib/Crc.h"
#include "../lib/SignalProperties.h"
#include "../HardwareLib/Connection.h"
#include "IssueLogger.h"
#include "SignalSet.h"

#include <QQmlEngine>

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

	ConfigurationBuilder::ConfigurationBuilder(BuildWorkerThread* buildWorkerThread, Context* context):
		m_buildResultWriter(context->m_buildResultWriter.get()),
		m_buildWorkerThread(buildWorkerThread),
		m_db(&context->m_db),
		m_deviceRoot(context->m_equipmentSet->root().get()),
		m_fscModules(context->m_fscModules),
		m_lmDescriptions(context->m_fscDescriptions.get()),
		m_signalSet(context->m_signalSet.get()),
		m_subsystems(context->m_subsystems.get()),
		m_opticModuleStorage(context->m_opticModuleStorage.get()),
		m_log(context->m_log),
		m_generateExtraDebugInfo(context->generateExtraDebugInfo())
	{

		assert(m_db);
		assert(m_deviceRoot);
		assert(m_signalSet);
		assert(m_subsystems);
		assert(m_opticModuleStorage);
		assert(m_log);
		assert(m_buildResultWriter);

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
		for (auto it = m_fscModules.begin(); it != m_fscModules.end(); it++)
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

			for (auto it = m_fscModules.begin(); it != m_fscModules.end(); it++)
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

				if (lm->propertyExists("LMNumber") == false)
				{
					m_log->errCFG3000("LMNumber", lm->equipmentId());
					return false;
				}

				QString subsystemID = lm->propertyValue("SubsystemID").toString();

				if (subsystemID == subsystem->subsystemId())
				{
					// Check for unique LmNumber
					//
					int lmNumber = lm->propertyValue("LMNumber").toInt();

					for (auto slm : subsystemModules)
					{
						int sLmNumber = slm->propertyValue("LMNumber").toInt();

						if (sLmNumber == lmNumber)
						{
							m_log->errCFG3003(lmNumber, lm->equipmentId());
							return false;
						}
					}

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
				if (logicModuleDescription->flashMemory().m_configWriteBitstream == true)
				{
					if (runConfigurationScriptFile(subsystemModules, logicModuleDescription) == false)
					{
						return false;
					}
				}
			}
		}

		// Find all LM modules and save ssKey and channel information
		//

		std::sort(m_fscModules.begin(), m_fscModules.end(),
				  [](const Hardware::DeviceModule* a, const Hardware::DeviceModule* b) -> bool
		{
			return a->equipmentIdTemplate() < b->equipmentIdTemplate();
		});

		QStringList lmReport;
		lmReport << "Jumpers configuration for LM modules";

		for (Hardware::DeviceModule* m : m_fscModules)
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

			if (m->propertyExists("SubsystemChannel") == false)
			{
				lmReport << "No SubsystemChannel property found in " + m->equipmentIdTemplate();
				assert(false);
				continue;
			}

			int ssKey = m_subsystems->ssKey(m->propertyValue("SubsystemID").toString());
			int lmNumber = m->propertyValue("LMNumber").toInt();
			int channel = m->propertyValue("SubsystemChannel").toInt();

			Q_ASSERT(ssKey >= 0 && ssKey <= std::numeric_limits<quint16>::max());

			lmReport << "\r\n";
			lmReport << "Equipment ID: " + m->equipmentIdTemplate();
			lmReport << "Caption: " + m->caption();
			lmReport << "Place: " + QString::number(m->place());
			lmReport << "Subsystem ID: " + m->propertyValue("SubsystemID").toString();
			lmReport << "Subsystem Code: " + QString::number(ssKey);
			lmReport << "Subsystem Channel: " + E::valueToString<E::Channel>(channel);
			lmReport << "LM Number: " + QString::number(lmNumber);


			quint16 jumpers = static_cast<quint16>(ssKey) << 6;
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
			lmReportData.append(s.toUtf8());
			lmReportData.append(QChar::LineFeed);
		}

		if (m_buildResultWriter->addFile("Reports", "LmJumpers.txt", lmReportData) == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Failed to save LmJumpers.txt file!"));
			return false;
		}

		return true;
	}

	bool ConfigurationBuilder::writeDataFiles()
	{
		QStringList subsystemsList = m_buildResultWriter->firmwareWriter()->subsystems();

		// Save confCollection items to binary files
		//
		for (auto ss : subsystemsList)
		{
			const QByteArray& log = m_buildResultWriter->firmwareWriter()->scriptLog(ss);

			if (log.isEmpty() == false)
			{
				if (m_buildResultWriter->addFile(ss, ss.toLower() + ".mct", log) == nullptr)
				{
					return false;
				}
			}
		}

		if (m_generateExtraDebugInfo == true)
		{
			if (writeExtraDataFiles() == false)
			{
				return false;
			}
		}

		return true;
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

	DbController* ConfigurationBuilder::db()
	{
		return m_db;
	}

	IssueLogger *ConfigurationBuilder::log() const
	{
		return m_log;
	}

	bool ConfigurationBuilder::runConfigurationScriptFile(const std::vector<Hardware::DeviceModule*>& subsystemModules, LmDescription* lmDescription)
	{

		if (subsystemModules.empty() == true || lmDescription == nullptr)
		{
			assert(lmDescription);
			assert(subsystemModules.empty() == false);
			return false;
		}

		bool ok = false;

		// Get script file from the project databse
		//
		std::vector<DbFileInfo> fileList;

		ok = db()->getFileList(&fileList, DbDir::ModuleConfigurationDir, lmDescription->configurationStringFile(), true, nullptr);

		if (ok == false || fileList.size() != 1)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined,
							   tr("Can't get file list and find module configuration description file '%1'").arg(lmDescription->configurationStringFile()));
			return false;
		}

		std::shared_ptr<DbFile> scriptFile;
		ok = db()->getLatestVersion(fileList[0], &scriptFile, nullptr);

		if (ok == false || scriptFile == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined,
							   tr("Can't get module configuration description file %1").arg(lmDescription->configurationStringFile()));
			return false;
		}

		QString contents = QString::fromLocal8Bit(scriptFile->data());

		// Attach objects
		//

		std::unique_ptr<QJSEngine> jsEngine = std::make_unique<QJSEngine>();

		jsEngine->installExtensions(QJSEngine::ConsoleExtension);

		JsSignalSet jsSignalSet(m_signalSet);

		QJSValue jsBuilder = jsEngine->newQObject(this);
		QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

		QJSValue jsRoot = jsEngine->newQObject(new Hardware::ScriptDeviceObject{m_deviceRoot->sharedPtr()});

		QJSValue jsLogicModules = jsEngine->newArray((int)subsystemModules.size());
		for (int i = 0; i < subsystemModules.size(); i++)
		{
			Hardware::ScriptDeviceModule* m = new Hardware::ScriptDeviceModule{subsystemModules[i]->toModule()};
			QJSValue module = jsEngine->newQObject(m);

			jsLogicModules.setProperty(i, module);
		}

		int frameSize = lmDescription->flashMemory().m_configFramePayload;
		int frameCount = lmDescription->flashMemory().m_configFrameCount;

		QString subsysStrID = subsystemModules[0]->propertyValue("SubsystemID").toString();
		int subsysID = m_subsystems->ssKey(subsysStrID);

		int configUartId = lmDescription->flashMemory().m_configUartId;

		m_buildResultWriter->firmwareWriter()->createFirmware(subsysStrID,
										 subsysID,
										 configUartId,
										 "Configuration",
										 frameSize,
										 frameCount,
										 lmDescription->lmDescriptionFile(subsystemModules[0]),
										lmDescription->descriptionNumber());

		m_buildResultWriter->firmwareWriter()->setScriptFirmware(subsysStrID, configUartId);

		QJSValue jsFirmware = jsEngine->newQObject(m_buildResultWriter->firmwareWriter());
		QQmlEngine::setObjectOwnership(m_buildResultWriter->firmwareWriter(), QQmlEngine::CppOwnership);

		QJSValue jsLog = jsEngine->newQObject(m_log);
		QQmlEngine::setObjectOwnership(m_log, QQmlEngine::CppOwnership);

		QJSValue jsSignalSetObject = jsEngine->newQObject(&jsSignalSet);
		QQmlEngine::setObjectOwnership(&jsSignalSet, QQmlEngine::CppOwnership);

		QJSValue jsSubsystems = jsEngine->newQObject(m_subsystems);
		QQmlEngine::setObjectOwnership(m_subsystems, QQmlEngine::CppOwnership);

		QJSValue jsOpticModuleStorage = jsEngine->newQObject(m_opticModuleStorage);
		QQmlEngine::setObjectOwnership(m_opticModuleStorage, QQmlEngine::CppOwnership);

		QJSValue jsLogicModuleDescription = jsEngine->newQObject(lmDescription);
		QQmlEngine::setObjectOwnership(lmDescription, QQmlEngine::CppOwnership);

		// Run script
		//
		QJSValue jsEval = jsEngine->evaluate(contents);
		if (jsEval.isError() == true)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Module configuration script '%1' evaluation failed at line %2: %3").arg(lmDescription->configurationStringFile()).arg(jsEval.property("lineNumber").toInt()).arg(jsEval.toString()));
			return false;
		}

		if (!jsEngine->globalObject().hasProperty("main"))
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Script has no \"main\" function"));
			return false;
		}

		if (!jsEngine->globalObject().property("main").isCallable())
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("\"main\" property of script is not callable"));
			return false;
		}

		QJSValueList args;

		args << jsBuilder;
		args << jsRoot;
		args << jsLogicModules;
		args << jsFirmware;
		args << jsLog;
		args << jsSignalSetObject;
		args << jsSubsystems;
		args << jsOpticModuleStorage;
		args << jsLogicModuleDescription;

		QJSValue jsResult = jsEngine->globalObject().property("main").call(args);

		if (jsResult.isError() == true)
		{
			QString errorMessage = tr("Uncaught exception while generating module configuration '%1': %2, lineNumber: %3, Stack: %4, ")
									.arg(lmDescription->configurationStringFile())
									.arg(jsResult.toString())
									.arg(jsResult.property("lineNumber").toInt())
									.arg(jsResult.property("stack").toString());

			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, errorMessage);
			return false;
		}

		if (jsResult.toBool() == false)
		{
			return false;
		}

		return true;
	}

	bool ConfigurationBuilder::writeExtraDataFiles()
	{
		// Write equipment configuration to JSON

		QJsonObject jEquipment;

		writeDeviceObjectToJson(m_deviceRoot, jEquipment);

		QByteArray jEquipmentBytes = QJsonDocument(jEquipment).toJson();

		if (m_buildResultWriter->addFile("Reports", "Equipment.json", jEquipmentBytes) == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Failed to save Equipment.json file!"));
			return false;
		}

		//

		return true;
	}

	bool ConfigurationBuilder::writeDeviceObjectToJson(const Hardware::DeviceObject* object, QJsonObject& jParent)
	{
		if (object == nullptr)
		{
			Q_ASSERT(object);
			return false;
		}

		QJsonObject jObject;

		// Type

		jObject.insert(QLatin1String("className"), object->metaObject()->className());

		// Properties

		QJsonObject jProperties;

		for (const std::shared_ptr<Property>& sp : object->properties())
		{
			Property* p = sp.get();
			if (p == nullptr)
			{
				Q_ASSERT(p);
				return false;
			}

			if (p->caption() == QLatin1String("ConfigurationScript") ||
					p->caption() == QLatin1String("SpecificProperties") ||
					p->caption() == QLatin1String("SignalSpecificProperties") ||
					p->caption() == QLatin1String("EquipmentIDTemplate"))
			{
				continue;
			}

			QString value = p->value().toString();
			if (value.length() > 512)
			{
				value = tr("<Text, %1 symbols>").arg(value.length());
			}

			jProperties.insert(p->caption(), value);
		}

		if (jProperties.count() != 0)
		{
			jObject.insert(QLatin1String("objectProperties"), jProperties);
		}

		// Children

		QJsonObject jObjects;

		int childCount = object->childrenCount();
		for (int i = 0; i < childCount; i++)
		{
			writeDeviceObjectToJson(object->child(i).get(), jObjects);
		}

		if (jObjects.count() != 0)
		{
			jObject.insert(QLatin1String("objects"), jObjects);
		}

		// Append to parent

		if (object->isRoot())
		{
			jParent.insert(QLatin1String("root"), jObject);
		}
		else
		{
			jParent.insert(object->equipmentId(), jObject);
		}

		return true;

	}



}
