#include "BuildWorkerThread.h"
#include "Parser.h"
#include "ConfigurationBuilder.h"
#include "ApplicationLogicCompiler.h"
#include "SoftwareCfgGenerator.h"
#include "AppDataServiceCfgGenerator.h"
#include "DiagDataServiceCfgGenerator.h"
#include "MonitorCfgGenerator.h"
#include "TuningServiceCfgGenerator.h"
#include "TuningClientCfgGenerator.h"
#include "ConfigurationServiceCfgGenerator.h"
#include "ArchivingServiceCfgGenerator.h"
#include "MetrologyCfgGenerator.h"
#include "TestClientCfgGenerator.h"
#include "../Simulator/Simulator.h"

namespace Builder
{
	//
	//		BuildWorkerThread
	//
	BuildWorkerThread::BuildWorkerThread(QObject* parent) :
		QThread(parent)
	{
		setObjectName("BuildWorkerThread");
	}

	void BuildWorkerThread::run()
	{
		m_context = std::make_unique<Context>(m_log, buildOutputPath(), expertMode());
		std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
			{
				this->m_context.reset();		// this will release m_context on leaving run()
			});

		m_context->m_progress = 0;

		assert(m_context->m_log);

		QThread::currentThread()->setTerminationEnabled(true);

        QElapsedTimer buildTime;
		buildTime.start();

		bool ok = false;

		// Start logging to output string, this string will be written as file to build output
		//
		m_context->m_log->startStrLogging();
		m_context->m_log->clearItemsIssues();

		// Log softaware version
		//
		LOG_MESSAGE(m_context->m_log, qApp->applicationName() + " v" + qApp->applicationVersion());
		LOG_MESSAGE(m_context->m_log, tr("Started at: ") + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"));

		// Create database controller and open project
		//
		m_context->m_db.disableProgress();

		m_context->m_db.setHost(serverIpAddress());
		m_context->m_db.setPort(serverPort());
		m_context->m_db.setServerUsername(serverUsername());
		m_context->m_db.setServerPassword(serverPassword());

		ok = m_context->m_db.openProject(projectName(), projectUserName(), projectUserPassword(), nullptr);

		if (ok == false)
		{
			// Opening project %1 error (%2).
			//
			m_context->m_log->errPDB2006(projectName(), m_context->m_db.lastError());
			return;
		}
		else
		{
			LOG_MESSAGE(m_context->m_log, tr("Opening project %1: ok").arg(projectName()));
		}

		// Get project properties
		//
		ok = getProjectProperties();

		// --
		//
		m_context->m_buildResultWriter = std::make_shared<BuildResultWriter>();
		m_context->m_buildResultWriter->start(buildOutputPath(), &m_context->m_db, m_context->m_log, 0 /* Load correct ChangesetID */);

		do
		{
			// --
			//
			ok = m_context->m_db.lastChangesetId(&m_context->m_lastChangesetId);

			if (ok == false)
			{
				LOG_ERROR_OBSOLETE(m_context->m_log, Builder::IssueType::NotDefined, tr("lastChangesetId Error."));
				break;
			}

			int to_do_thre_are_two_places_in_build_checking_checked_out_objects;
			int checkedOutCount = 0;
			ok = m_context->m_db.isAnyCheckedOut(&checkedOutCount);
			if (ok == false)
			{
				LOG_ERROR_OBSOLETE(m_context->m_log, Builder::IssueType::NotDefined, tr("isAnyCheckedOut Error."));
				break;
			}

			const BuildInfo& bi = m_context->m_buildResultWriter->buildInfo();
			m_context->m_buildResultWriter->firmwareWriter()->setProjectInfo(bi.project, bi.user, bi.id, bi.changeset);

			//
			// Get Equipment from the database
			//
			if (ok = getEquipment();
				ok == false)
			{
				break;
			}

			m_context->m_progress += 5;			// Total progress 5

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Loading BusTypes
			//
			if (ok = loadBusses();
				ok == false )
			{
				LOG_ERROR_OBSOLETE(m_context->m_log, Builder::IssueType::NotDefined, tr("Can't load BusTypes files"));
			}

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Builder::SignalSet
			//
			m_context->m_signalSet = std::make_shared<SignalSet>(m_context->m_busSet.get(), m_context->m_buildResultWriter, m_context->m_log);

			if (m_context->m_signalSet->prepareBusses() == false)
			{
				break;
			}

			if (loadSignals(m_context->m_signalSet.get(), m_context->m_equipmentSet.get()) == false)
			{
				break;
			}

			m_context->m_progress += 10;		// Total progress 15

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Find all LM Modules and load their descriptions
			//
			if (ok = loadLmDescriptions();
				ok == false)
			{
				break;
			}

			m_context->m_progress += 5;			// Total progress 20

			//
			// Loading subsystems
			//
			if (ok = loadSubsystems();
				ok == false)
			{
				break;
			}
			else
			{
				LOG_SUCCESS(m_context->m_log, tr("Ok"));
			}

			//
			// Loading connections
			//
			if (bool loadConnectionsOk = loadConnections();
				loadConnectionsOk == false)
			{
				break;
			}

			//
			// Check that all files (and from that theirs SchemaIds) in $root$/Schema are unique
			//
			ok = checkRootSchemasUniquesIds(&m_context->m_db);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Parse application logic
			//
			if (bool parseOk = parseApplicationLogic();
				parseOk == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//m_context->m_progress is set inside parseApplicationLogic(); up to 25%
			//m_context->m_progress += 25;		// Total progress 45

			//
			// Save LogicModule Descriptions
			//
			ok = saveLogicModuleDescriptions();

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Compile application logic
			//
			m_context->m_tuningDataStorage = std::make_shared<Tuning::TuningDataStorage>();
			m_context->m_comparatorSet = std::make_shared<ComparatorSet>();

			ok = compileApplicationLogic();

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			m_context->m_progress += 10;		// Total progress 55

			//
			// Tuning parameters
			//
			LOG_EMPTY_LINE(m_context->m_log);
			LOG_MESSAGE(m_context->m_log, tr("Tuning parameters compilation"));

			::Builder::TuningBuilder tuningBuilder(m_context.get());

			ok = tuningBuilder.build();

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Compile Module configuration
			//
			LOG_EMPTY_LINE(m_context->m_log);
			LOG_MESSAGE(m_context->m_log, tr("Module configurations compilation"));

			ConfigurationBuilder cfgBuilder(this, m_context.get());

			ok = cfgBuilder.build();

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			generateModulesInformation(*m_context->m_buildResultWriter, m_context->m_lmAndBvbModules);

			ok &= generateLmsUniqueIDs(m_context.get());

			ok &= writeLogicModulesInfoXml(m_context.get());

			ok &= buildSoftwareList(m_context.get());

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			m_context->m_progress += 10;		// Total progress 65

			//
			// Load Sim Profiles
			//
			loadSimProfiles();

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Generate MATS software configurations
			//
			ok = generateSoftwareConfiguration();

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			m_context->m_progress += 10;		// Total progress 75

			//
			// Write logic, configuration and tuning binary files
			//
			if (m_log->errorCount() == 0)
			{
				ok = writeBinaryFiles(*m_context->m_buildResultWriter);

				if (ok == false ||
						QThread::currentThread()->isInterruptionRequested() == true)
				{
					break;
				}
			}

			//

			ok = cfgBuilder.writeDataFiles();

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			m_context->m_progress += 10;		// Total progress 85

			//
			// Write Firmware Statistics
			//
			ok = writeFirmwareStatistics(*m_context->m_buildResultWriter);

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			m_context->m_progress += 5;		// Total progress 90

			//
			// Run simulator-based script tests
			//
			ok = runSimTests();

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			m_context->m_progress += 5;		// Total progress 95

			//
			// Ok
			//
			LOG_SUCCESS(m_context->m_log, tr("Ok"));
		}
		while (false);

		m_context->m_buildResultWriter->finish();

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			// The build was cancelled.
			//
			m_context->m_log->errCMN0016();
			m_context->m_log->clear();		// Log can contain thouthands of messages, if it some kind iof "same ids" error
		}

		m_context->m_db.closeProject(nullptr);

		// Display build time
		//
		qint64 buildEllapsed = buildTime.elapsed() / 1000;
		qint64 durationSecs = buildEllapsed % 60;
		qint64 durationMins = buildEllapsed / 60;
		LOG_MESSAGE(m_context->m_log, QString("Build time: %1 minute(s) %2 second(s)").arg(durationMins).arg(durationSecs));

		// We've done, exit
		//
		qDebug("Leave BuildWorkerThread::run()");

		m_context->m_progress += 5;		// Total progress 100

		// QThread::finished will be emitted, it should be counted as reasultReady
		//

		return;
	}

	bool BuildWorkerThread::getProjectProperties()
	{
		assert(m_context->m_db.isProjectOpened() == true);

		bool ok = m_context->m_db.getProjectProperties(&m_context->m_projectProperties, nullptr);

		if (ok == false)
		{
			m_context->m_log->errPDB2020();
		}

		// --
		//
		std::vector<int> suppressWarnings = m_context->m_projectProperties.suppressWarnings();

		if (suppressWarnings.empty() == false)
		{
			QString suppressWarningsMessage;
			for (int w : suppressWarnings)
			{
				if (suppressWarningsMessage.isEmpty() == false)
				{
					suppressWarningsMessage += QStringLiteral(", ");
				}

				suppressWarningsMessage += QString::number(w);
			}

			LOG_MESSAGE(m_context->m_log, tr("Suppressed warnings: %1").arg(suppressWarningsMessage));
		}

		m_context->m_log->setSupressIssues(suppressWarnings);

		return ok;
	}

	bool BuildWorkerThread::getEquipment()
	{
		LOG_EMPTY_LINE(m_context->m_log);
		LOG_MESSAGE(m_context->m_log, tr("Getting equipment"));

		std::shared_ptr<Hardware::DeviceRoot> deviceRoot = std::make_shared<Hardware::DeviceRoot>();

		int rootFileId = m_context->m_db.systemFileId(DbDir::HardwareConfigurationDir);
		auto fio = std::make_shared<DbFileInfo>(rootFileId);

		deviceRoot->setData(fio);

		if (bool ok = getEquipment(deviceRoot.get());
			ok == false)
		{
			return false;
		}
		else
		{
			LOG_SUCCESS(m_context->m_log, tr("Ok"));
		}

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			return true;
		}

		//
		// Expand Devices StrId
		//
		LOG_EMPTY_LINE(m_context->m_log);
		LOG_MESSAGE(m_context->m_log, tr("Expanding devices StrIds"));

		if (bool ok = expandDeviceStrId(deviceRoot.get());
			ok == false)
		{
			return false;
		}

		LOG_SUCCESS(m_context->m_log, tr("Ok"));

		m_context->m_equipmentSet = std::make_shared<Hardware::EquipmentSet>(deviceRoot);

		deviceRoot.reset();		// Use equipmentSet.root() instead

		//
		// Check same Uuids and same StrIds
		//
		LOG_EMPTY_LINE(m_context->m_log);
		LOG_MESSAGE(m_context->m_log, tr("Checking for same Uuids and StrIds"));

		if (bool ok = checkUuidAndStrId(m_context->m_equipmentSet->root().get());
			ok == false)
		{
			return false;
		}

		//
		// Check child restirictions
		//
		if (bool ok = checkChildRestrictions(m_context->m_equipmentSet->root());
			ok == false)
		{
			return false;
		}

		LOG_SUCCESS(m_context->m_log, tr("Ok"));

		return true;
	}

	bool BuildWorkerThread::getEquipment(Hardware::DeviceObject* parent)
	{
		assert(m_context->m_db.isProjectOpened() == true);
		assert(parent != nullptr);

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			return false;
		}

		// --
		//
		std::vector<DbFileInfo> files;
		bool ok = false;

		// --
		//
		int parentFileId = -1;

		if (const DbFileInfo* parentFileInfo = parent->data();
			parentFileInfo == nullptr)
		{
			Q_ASSERT(parentFileInfo);
			return false;
		}
		else
		{
			parentFileId = parentFileInfo->fileId();
		}

		// Get file list with checked out files,
		//
		ok = m_context->m_db.getFileList(&files, parentFileId, true, nullptr);

		if (ok == false)
		{
			LOG_ERROR_OBSOLETE(m_context->m_log, Builder::IssueType::NotDefined, tr("Cannot get equipment file list"));
			return false;
		}

		parent->deleteAllChildren();

		for (DbFileInfo& fi : files)
		{
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			if (fi.action() == VcsItemAction::Deleted)		// File is deleted
			{
				continue;
			}

			LOG_MESSAGE(m_context->m_log, tr("Getting equipment object, fileid: %1, details: %2").arg(fi.fileId()).arg(fi.details()));

			std::shared_ptr<Hardware::DeviceObject> device;
			ok = m_context->m_db.getDeviceTreeLatestVersion(fi, &device, nullptr);

			if (ok == false ||
				device.get() == nullptr)
			{
				LOG_ERROR_OBSOLETE(m_context->m_log, "", tr("Failed to load equipment, fileid: %1").arg(fi.fileId()));
				continue;
			}

			parent->addChild(device);
		}

		return true;
	}

	void BuildWorkerThread::findFSCConfigurationModules(Hardware::DeviceObject* object, std::vector<Hardware::DeviceModule*>* out) const
	{
		if (object == nullptr ||
			out == nullptr)
		{
			assert(object);
			assert(out);
			return;
		}

		for (int i = 0; i < object->childrenCount(); i++)
		{
			Hardware::DeviceObject* child = object->child(i).get();

			if (child->deviceType() == Hardware::DeviceType::Module)
			{
				Hardware::DeviceModule* module = dynamic_cast<Hardware::DeviceModule*>(child);

				if (module->isFSCConfigurationModule() == true)
				{
					out->push_back(module);
				}
			}

			if (child->deviceType() < Hardware::DeviceType::Module)
			{
				findFSCConfigurationModules(child, out);
			}
		}

		return;
	}

	void BuildWorkerThread::findModulesByFamily(Hardware::DeviceObject* object, std::vector<Hardware::DeviceModule*>* out, Hardware::DeviceModule::FamilyType family) const
	{
		if (object == nullptr ||
			out == nullptr)
		{
			assert(object);
			assert(out);
			return;
		}

		for (int i = 0; i < object->childrenCount(); i++)
		{
			Hardware::DeviceObject* child = object->child(i).get();

			if (child->deviceType() == Hardware::DeviceType::Module)
			{
				Hardware::DeviceModule* module = dynamic_cast<Hardware::DeviceModule*>(child);

				if (module->moduleFamily() == family)
				{
					out->push_back(module);
				}
			}

			if (static_cast<int>(child->deviceType()) < static_cast<int>(Hardware::DeviceType::Module))
			{
				findModulesByFamily(child, out, family);
			}
		}

		return;

	}

	bool BuildWorkerThread::expandDeviceStrId(Hardware::DeviceObject* device)
	{
		if (device == nullptr)
		{
			assert(device != nullptr);
			return false;
		}

		device->expandEquipmentId();

		return true;
	}

	bool BuildWorkerThread::loadSubsystems()
	{
		LOG_EMPTY_LINE(m_context->m_log);
		LOG_MESSAGE(m_context->m_log, tr("Loading subsystems..."));

		QString errorCode;

		m_context->m_subsystems = std::make_shared<Hardware::SubsystemStorage>();

		if (bool ok = m_context->m_subsystems->load(&m_context->m_db, errorCode);
			ok == false)
		{
			LOG_ERROR_OBSOLETE(m_context->m_log, Builder::IssueType::NotDefined, tr("Can't load subsystems file"));
			if (errorCode.isEmpty() == false)
			{
				LOG_ERROR_OBSOLETE(m_context->m_log, Builder::IssueType::NotDefined, errorCode);
				return false;
			}
		}

		// Check if subsystems have same id or sskey
		//
		std::set<QString> ids;
		std::set<int> sskeys;

		bool result = true;

		for (std::shared_ptr<Hardware::Subsystem> subsystem : m_context->m_subsystems->subsystems())
		{
			assert(subsystem);

			if (ids.count(subsystem->subsystemId()) > 0)
			{
				// Duplicate SubsystemID
				//
				result = false;
				m_context->m_log->errEQP6005(subsystem->subsystemId());
			}
			else
			{
				ids.insert(subsystem->subsystemId());
			}

			if (sskeys.count(subsystem->key()) > 0)
			{
				// Duplicate ssKey
				//
				result = false;
				m_context->m_log->errEQP6006(subsystem->key());
			}
			else
			{
				sskeys.insert(subsystem->key());
			}
		}

		// Check if all LMs in subsystem have the same version and LmDescriptionFile
		//
		for (std::shared_ptr<Hardware::Subsystem> subsystem : m_context->m_subsystems->subsystems())
		{
			assert(subsystem);

			Hardware::DeviceModule::FamilyType moduleFamily = Hardware::DeviceModule::FamilyType::OTHER;
			int moduleVersion = -1;
			QString LmDescriptionFile;

			for (const Hardware::DeviceModule* lm : m_context->m_lmAndBvbModules)
			{
				assert(lm);
				assert(lm->isFSCConfigurationModule() == true);

				auto lmSubsystemIdProp = lm->propertyByCaption(Hardware::PropertyNames::lmSubsystemID);
				if (lmSubsystemIdProp == nullptr)
				{
					m_context->m_log->errCFG3000(Hardware::PropertyNames::lmSubsystemID, lm->equipmentIdTemplate());
					return false;
				}

				if (lmSubsystemIdProp->value() != subsystem->subsystemId())
				{
					continue;
				}

				// Check moduleFamily
				//
				if (moduleFamily == Hardware::DeviceModule::FamilyType::OTHER)
				{
					moduleFamily = lm->moduleFamily();			// Init start value
				}
				else
				{
					if (moduleFamily != lm->moduleFamily())
					{
						result = false;
						m_context->m_log->errEQP6007(subsystem->subsystemId());
						continue;
					}
				}

				// Check moduleVersion
				//
				if (moduleVersion == -1)
				{
					moduleVersion = lm->moduleVersion();			// Init start value
				}
				else
				{
					if (moduleVersion != lm->moduleVersion())
					{
						result = false;
						m_context->m_log->errEQP6007(subsystem->subsystemId());
						continue;
					}
				}

				// Check LmDescriptionFile
				//
				auto LmDescriptionFileProp = lm->propertyByCaption(Hardware::PropertyNames::lmDescriptionFile);
				if (LmDescriptionFileProp == nullptr)
				{
					m_context->m_log->errCFG3000(Hardware::PropertyNames::lmDescriptionFile, lm->equipmentIdTemplate());
					return false;
				}

				if (LmDescriptionFile.isEmpty() == true)
				{
					LmDescriptionFile = LmDescriptionFileProp->value().toString();	// Init start value
				}
				else
				{
					if (LmDescriptionFile != LmDescriptionFileProp->value().toString())
					{
						result = false;
						m_context->m_log->errEQP6007(subsystem->subsystemId());
						continue;
					}
				}
			}
		}

		return result;
	}

	bool BuildWorkerThread::checkUuidAndStrId(Hardware::DeviceObject* root)
	{
		if (root == nullptr)
		{
			assert(root);
			return false;
		}

		std::map<QUuid, Hardware::DeviceObject*> uuidMap;
		std::map<QString, Hardware::DeviceObject*> strIdMap;

		// Recursive function
		//

		bool ok = checkUuidAndStrIdWorker(root, uuidMap, strIdMap);

		return ok;
	}



	bool BuildWorkerThread::checkUuidAndStrIdWorker(Hardware::DeviceObject* device,
									 std::map<QUuid, Hardware::DeviceObject*>& uuidMap,
									 std::map<QString, Hardware::DeviceObject*>& strIdMap)
	{
		if (device == nullptr)
		{
			assert(device);
			return false;
		}

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			return false;
		}

		// Check for the same Uuid and StrID
		//
		auto foundSameUuid = uuidMap.find(device->uuid());
		auto foundSameStrId = strIdMap.find(device->equipmentIdTemplate());

		bool ok = true;

		if (foundSameUuid != uuidMap.end())
		{
			// Two or more equipment objects have the same Uuid '%1' (Object1 '%2', Object2 '%3').
			//
			m_context->m_log->errEQP6002(device->uuid(), device->equipmentId(), foundSameUuid->second->equipmentId());
			ok = false;
		}
		else
		{
			uuidMap[device->uuid()] = device;
		}

		if (foundSameStrId != strIdMap.end())
		{
			// Two or more equipment objects have the same StrID '%1'.
			//
			m_context->m_log->errEQP6001(device->equipmentId(), device->uuid(), foundSameStrId->second->uuid()) ;
			ok = false;
		}
		else
		{
			strIdMap[device->equipmentIdTemplate()] = device;
		}


		if (device->isModule())
		{
			Hardware::DeviceModule* module = (Hardware::DeviceModule*)device;

			if (module->moduleFamily() == Hardware::DeviceModule::FamilyType::LM && module->place() != 0)
			{
				m_context->m_log->errEQP6009(module->equipmentIdTemplate(), module->uuid());
				ok = false;
				return ok;
			}
		}

		// Check property Place, must not be -1
		//
		if (device->place() < 0 && device->isRoot() == false)
		{
			// Property Place is less then 0 (Equipment object '%1').
			//
			m_context->m_log->errEQP6000(device->equipmentIdTemplate(), device->uuid());
			ok = false;
		}

		// --
		//
		int childCount = device->childrenCount();

		for (int i = 0; i < childCount; i++)
		{
			ok &= checkUuidAndStrIdWorker(device->child(i).get(), uuidMap, strIdMap);
		}

		return ok;
	}


	bool BuildWorkerThread::checkChildRestrictions(std::shared_ptr<Hardware::DeviceObject> root)
	{
		if (root == nullptr)
		{
			assert(root);
			return false;
		}

		// Recursive function
		//
		bool ok = checkChildRestrictionsWorker(root);

		return ok;
	}

	bool BuildWorkerThread::checkChildRestrictionsWorker(std::shared_ptr<Hardware::DeviceObject> device)
	{
		assert(device != nullptr);

		QString errorMessage;

		int childrenCount = device->childrenCount();

		for (int i = 0; i < childrenCount; i++)
		{
			auto child = device->child(i);

			if (child == nullptr)
			{
				assert(child);
				return false;
			}

			bool allowed = device->checkChild(child, &errorMessage);

			if (allowed == false)
			{
				if (errorMessage.isEmpty() == false)
				{
					m_context->m_log->errINT1001(errorMessage);
				}

				m_context->m_log->errEQP6008(device->equipmentId(), child->equipmentId(), child->place());
				return false;
			}

			if (checkChildRestrictionsWorker(child) == false)
			{
				return false;
			}
		}

		return true;
	}

	bool BuildWorkerThread::loadSignals(SignalSet* signalSet, Hardware::EquipmentSet* equipment)
	{
		IssueLogger* log = m_context->m_log;

		TEST_PTR_RETURN_FALSE(log);
		TEST_PTR_LOG_RETURN_FALSE(signalSet, log);
		TEST_PTR_LOG_RETURN_FALSE(equipment, log);

		LOG_EMPTY_LINE(log);

		LOG_MESSAGE(log, tr("Loading application signals"));

		bool result = m_context->m_db.getSignals(signalSet, true, nullptr);

		if (result == false)
		{
			// Load signals from the project database error
			//
			m_context->m_log->errPDB2003();
			return false;
		}

		signalSet->findAndRemoveExcludedFromBuildSignals();

		signalSet->cacheSpecPropValues();

		result = signalSet->expandTemplates(equipment);

		RETURN_IF_FALSE(result);

		result = signalSet->checkSignals();

		RETURN_IF_FALSE(result);

		//QStringList

		signalSet->buildID2IndexMap();

		result = signalSet->bindSignalsToLMs(equipment);

		if (result == false)
		{
			return false;
		}

		signalSet->initCalculatedSignalsProperties();

		LOG_SUCCESS(log, tr("Ok"));

		return true;
	}

	bool BuildWorkerThread::loadBusses()
	{
		m_context->m_busSet = std::make_shared<VFrame30::BusSet>();

		// Get Busses
		//
		std::vector<DbFileInfo> fileList;

		bool ok = m_context->m_db.getFileList(&fileList, DbDir::BusTypesDir, Db::File::BusFileExtension, true, nullptr);
		if (ok == false)
		{
			return false;
		}

		if (fileList.empty() == true)
		{
			return true;
		}

		// Get Busses latest version from the DB
		//
		std::vector<std::shared_ptr<DbFile>> files;

		ok = m_context->m_db.getLatestVersion(fileList, &files, nullptr);
		if (ok == false)
		{
			return false;
		}

		// Parse files, create actual Busses
		//
		std::vector<VFrame30::Bus> busses;
		busses.reserve(files.size());

		for (const std::shared_ptr<DbFile>& f : files)
		{
			if (f->deleted() == true ||
				f->action() == VcsItemAction::Deleted)
			{
				continue;
			}

			VFrame30::Bus bus;
			ok = bus.Load(f->data());
			if (ok == false)
			{
				return false;
			}

			busses.push_back(bus);
		}

		m_context->m_busSet->setBusses(busses);

		return true;
	}


	bool BuildWorkerThread::loadLmDescriptions()
	{
		LOG_EMPTY_LINE(m_context->m_log);
		LOG_MESSAGE(m_context->m_log, tr("Loading LogicModule descriptions..."));

		m_context->m_lmDescriptions = std::make_shared<LmDescriptionSet>();
		m_context->m_fscDescriptions = std::make_shared<LmDescriptionSet>();

		findModulesByFamily(m_context->m_equipmentSet->root().get(), &m_context->m_lmModules, Hardware::DeviceModule::FamilyType::LM);

		findModulesByFamily(m_context->m_equipmentSet->root().get(), &m_context->m_lmAndBvbModules, Hardware::DeviceModule::FamilyType::LM);
		findModulesByFamily(m_context->m_equipmentSet->root().get(), &m_context->m_lmAndBvbModules, Hardware::DeviceModule::FamilyType::BVB);

		bool ok = true;
		for (Hardware::DeviceModule* lm : m_context->m_lmAndBvbModules)
		{
			ok &= loadLogicModuleDescription(lm, m_context->m_lmDescriptions.get());
		}

		if (ok == false)
		{
			return false;
		}

		findFSCConfigurationModules(m_context->m_equipmentSet->root().get(), &m_context->m_fscModules);

		ok = true;
		for (Hardware::DeviceModule* lm : m_context->m_fscModules)
		{
			ok &= loadLogicModuleDescription(lm, m_context->m_fscDescriptions.get());
		}

		if (ok == false)
		{
			return false;
		}
		else
		{
			LOG_SUCCESS(m_context->m_log, tr("Ok"));
		}

		return true;
	}

	bool BuildWorkerThread::loadLogicModuleDescription(Hardware::DeviceModule* logicModule, LmDescriptionSet* lmDescriptions)
	{
		if (logicModule == nullptr ||
			lmDescriptions == nullptr)
		{
			assert(logicModule);
			assert(lmDescriptions);
			return false;
		}

		// Get LmDescriptionFile property value
		//
		auto lmDescriptionFileProp = logicModule->propertyByCaption(Hardware::PropertyNames::lmDescriptionFile);

		if (lmDescriptionFileProp == nullptr)
		{
			m_context->m_log->errCFG3000(Hardware::PropertyNames::lmDescriptionFile, logicModule->equipmentIdTemplate());
			return false;
		}

		QString lmDescriptionFile = lmDescriptionFileProp->value().toString();

		if (lmDescriptionFile.isEmpty() == true)
		{
			m_context->m_log->errEQP6020(logicModule->equipmentIdTemplate(), logicModule->uuid());
			return false;
		}

		// Has file already been loaded?
		//
		if (lmDescriptions->has(lmDescriptionFile) == true)
		{
			return true;
		}

		// Load file
		//
		bool ok = lmDescriptions->loadFile(m_context->m_log, &m_context->m_db, logicModule->equipmentIdTemplate(), lmDescriptionFile);

		return ok;
	}

	bool BuildWorkerThread::loadConnections()
	{
		QString errorMessage;

		m_context->m_connections = std::make_shared<Hardware::ConnectionStorage>(&m_context->m_db);

		if (bool ok = m_context->m_connections->load(&errorMessage);
			ok == false)
		{
			LOG_ERROR_OBSOLETE(m_context->m_log, Builder::IssueType::NotDefined, tr("Can't load connections file: "));
			if (errorMessage.isEmpty() == false)
			{
				LOG_ERROR_OBSOLETE(m_context->m_log, Builder::IssueType::NotDefined, errorMessage);
				return false;
			}
		}

		m_context->m_opticModuleStorage = std::make_shared<Hardware::OptoModuleStorage>(m_context.get());
		bool res = m_context->m_opticModuleStorage->init();

		return res;
	}

	// Check that all files (and from that theirs SchemaIds) in $root$/Schema are unique
	//
	bool BuildWorkerThread::checkRootSchemasUniquesIds(DbController* db)
	{
		if (db == nullptr)
		{
			assert(db);
			return false;
		}

		LOG_EMPTY_LINE(m_context->m_log);
		LOG_MESSAGE(m_context->m_log, tr("Checking uniqueness SchemaIDs..."));

		DbFileTree fileTree;

		if (bool ok = db->getFileListTree(&fileTree, DbDir::SchemasDir, true, nullptr);
			ok == false)
		{
			m_context->m_log->errPDB2001(db->systemFileId(DbDir::SchemasDir), "", db->lastError());
			return false;
		}

		// --
		//
		const std::map<int, std::shared_ptr<DbFileInfo>>& files = fileTree.files();

		QString strAlFileExtension{Db::File::AlFileExtension};
		QString strUfbFileExtension{Db::File::UfbFileExtension};
		QString strMvsFileExtension{Db::File::MvsFileExtension};
		QString strDvsFileExtension{Db::File::DvsFileExtension};

		bool success = true;

		std::set<QString> schemaIds;
		for (auto& [fileId, file] : files)
		{
			if (file->isFolder() == true)
			{
				continue;
			}

			QString fileExt = file->extension();

			if (fileExt.compare(strAlFileExtension, Qt::CaseInsensitive) != 0 &&
				fileExt.compare(strUfbFileExtension, Qt::CaseInsensitive) != 0 &&
				fileExt.compare(strMvsFileExtension, Qt::CaseInsensitive) != 0 &&
				fileExt.compare(strDvsFileExtension, Qt::CaseInsensitive) != 0)
			{
				continue;
			}

			//--
			//
			VFrame30::SchemaDetails details;

			if (bool ok = details.parseDetails(file->details());
				ok == false)
			{
				m_context->m_log->errALP4024(file->fileName(), file->details());
				continue;
			}

			if (schemaIds.count(details.m_schemaId) != 0)
			{
				m_context->m_log->errALP4025(details.m_schemaId);
				success = false;
			}
			else
			{
				schemaIds.insert(details.m_schemaId);
			}
		}

		if (success == true)
		{
			LOG_SUCCESS(m_context->m_log, tr("Ok"));
		}

		return success;
	}

	bool BuildWorkerThread::parseApplicationLogic()
	{
		m_context->m_appLogicData = std::make_shared<AppLogicData>();

		LOG_EMPTY_LINE(m_context->m_log);
		LOG_MESSAGE(m_context->m_log, tr("Application Logic parsing..."));

		Parser parser(m_context.get());
		parser.parse();

		bool result = m_context->m_log->errorCount() == 0;
		if (result == false)
		{
		}
		else
		{
			LOG_SUCCESS(m_context->m_log, tr("Ok"));
		}

		emit runOrderReady(parser.runOrder());

		return result;
	}

	bool BuildWorkerThread::saveLogicModuleDescriptions()
	{
		assert(m_context->m_buildResultWriter);

		LOG_MESSAGE(m_context->m_log, "Saving LogicModule's descriptions");

		QStringList lmFiles = m_context->m_lmDescriptions->fileList();

		for (QString fileName : lmFiles)
		{
			auto file = m_context->m_lmDescriptions->rowFile(fileName);

			if (file.second == false)
			{
				m_context->m_log->errINT1000(tr("File %1 present in LmDescriptionSet but cannot be found it's raw version. ") + Q_FUNC_INFO);
				return false;
			}

			BuildFile* buildFile = m_context->m_buildResultWriter->addFile(QLatin1String("LmDescriptions"), fileName, file.first, false);
			assert(buildFile);
			Q_UNUSED(buildFile);
		}

		return true;
	}

	bool BuildWorkerThread::compileApplicationLogic()
	{
		LOG_EMPTY_LINE(m_context->m_log);
		LOG_MESSAGE(m_context->m_log, tr("Application Logic compilation"));

		ApplicationLogicCompiler appLogicCompiler(m_context.get());

		bool result = appLogicCompiler.run();

		LOG_EMPTY_LINE(m_context->m_log);

		if (result == false)
		{
			LOG_MESSAGE(m_context->m_log, tr("Application Logic compilation was finished with errors"));
		}
		else
		{
			LOG_SUCCESS(m_context->m_log, tr("Application Logic compilation was succesfully finished"));
		}

		return result;
	}

	bool BuildWorkerThread::loadSimProfiles()
	{
		Context* context = m_context.get();
		DbController& db = context->m_db;
		IssueLogger* log = context->m_log;

		LOG_EMPTY_LINE(log);
		LOG_MESSAGE(log, QString(tr("Loading Sim Profiles...")));

		m_context->m_simProfiles.clear();
		QByteArray fileContent;

		std::vector<DbFileInfo> fileList;

		bool ok = db.getFileList(&fileList, DbDir::EtcDir, Db::File::SimProfilesFileName, true, nullptr);
		if (ok == false)
		{
			log->errPDB2001(db.systemFileId(DbDir::EtcDir), Db::File::SimProfilesFileName, db.lastError());
			return false;
		}

		if (ok == true && fileList.size() == 1)
		{
			std::shared_ptr<DbFile> file;

			ok = db.getLatestVersion(fileList[0], &file, nullptr);

			if (ok == true)
			{
				QString errorMessage;
				fileContent = file->data();

				ok = m_context->m_simProfiles.load(fileContent, &errorMessage);

				if (ok == false)
				{
					log->errCMN0010(fileList[0].fileName());
					return false;
				}
			}
			else
			{
				log->errPDB2002(fileList[0].fileId(), fileList[0].fileName(), db.lastError());
				return false;
			}
		}

		// Save file to build
		//
		BuildFile* outFile = context->m_buildResultWriter->addFile(Directory::COMMON, Db::File::SimProfilesFileName, fileContent, false);
		if (outFile == nullptr)
		{
			return false;
		}

		/*
		// -- EXAMPLE OF USING Sim::SimProfiles: --
		//

		// Enumerate all profiles
		//
		const QStringList profiles = m_context->m_simProfiles.profiles();
		for (QString profileName : profiles)
		{
			Sim::Profile& profile = m_context->m_simProfiles.profile(profileName);
			Q_ASSERT(profileName == profile.profileName);

			// Enumerate objects in the profile
			//
			const QStringList objectList = profile.equipment();
			bool allAppliedSuccesfully = true;

			for (QString equipmentId : objectList)
			{
				// Get device object from the EquipmentSet
				//
				std::shared_ptr<Hardware::DeviceObject> object = m_context->m_equipmentSet->deviceObject(equipmentId);
				if (object == nullptr)
				{
					m_log->errEQP6011(equipmentId, QString("appling %1 SimProfile").arg(profileName));
					allAppliedSuccesfully = false;
					continue;
				}

				Q_ASSERT(object->equipmentId() == equipmentId);

				// Apply profile properties to DeviceObject.
				// DeviceObject's state is saved here, later it can be restored with Sim::Profile::restoreObjects();
				//
				QString errorMessage;

				bool applyOk = profile.applyToObject(equipmentId, object, &errorMessage);
				if (applyOk == false)
				{
					m_log->errEQP6030(profileName, errorMessage);
					allAppliedSuccesfully = false;
					continue;
				}
			}

			if (allAppliedSuccesfully == false)
			{
				continue;
			}

			// At this point we have m_context->m_equipmentSet with applied profile
			// Do settings generation here
			// ....
			//

			// Restore EquipmentSet state
			//
			profile.restoreObjects();
		}

		// -- END OF EXAMPLE--
		//
		*/

		// --
		//
		return true;
	}

	bool BuildWorkerThread::generateSoftwareConfiguration()
	{
		SoftwareCfgGenerator::clearStaticData();

		Context* context = m_context.get();

		bool result = true;

		LOG_EMPTY_LINE(context->m_log);
		LOG_MESSAGE(context->m_log, QString(tr("MATS configuration generation...")))
		LOG_EMPTY_LINE(context->m_log);

		result = checkProfiles();

		RETURN_IF_FALSE(result);

		result &= SoftwareCfgGenerator::generalSoftwareCfgGeneration(context);

		RETURN_IF_FALSE(result);

		std::map<QString, std::shared_ptr<SoftwareCfgGenerator>> swCfgGens;

		// create SoftwareCfgGenerators and generate "Default" profile settings
		//

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString("Software settings generation for profile: %1").
							arg(SettingsProfile::DEFAULT));

		for(auto p : m_context->m_software)
		{
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;;
			}

			Hardware::Software* software = p.second;

			if (software == nullptr	|| software->isSoftware() == false)
			{
				Q_ASSERT(false);
				LOG_INTERNAL_ERROR(m_log);
				result = false;
				continue;
			}

			std::shared_ptr<SoftwareCfgGenerator> swCfgGen;

			switch(software->softwareType())
			{
			case E::SoftwareType::AppDataService:
				swCfgGen = std::make_shared<AppDataServiceCfgGenerator>(context, software);
				break;

			case E::SoftwareType::DiagDataService:
				swCfgGen = std::make_shared<DiagDataServiceCfgGenerator>(context, software);
				break;

			case E::SoftwareType::Monitor:
				swCfgGen = std::make_shared<MonitorCfgGenerator>(context, software);
				break;

			case E::SoftwareType::TuningService:
				swCfgGen = std::make_shared<TuningServiceCfgGenerator>(context, software);
				break;

			case E::SoftwareType::TuningClient:
				swCfgGen = std::make_shared<TuningClientCfgGenerator>(context, software);
				break;

			case E::SoftwareType::ConfigurationService:
				swCfgGen = std::make_shared<ConfigurationServiceCfgGenerator>(context, software);
				break;

			case E::SoftwareType::ArchiveService:
				swCfgGen = std::make_shared<ArchivingServiceCfgGenerator>(context, software);
				break;

			case E::SoftwareType::Metrology:
				swCfgGen = std::make_shared<MetrologyCfgGenerator>(context, software);
				break;

			case E::SoftwareType::TestClient:
				swCfgGen = std::make_shared<TestClientCfgGenerator>(context, software);
				break;

			default:
				m_context->m_log->errEQP6100(software->equipmentIdTemplate(), software->uuid());
				result = false;
			}

			if (swCfgGen != nullptr)
			{
				swCfgGens.insert({software->equipmentIdTemplate(), swCfgGen});
				result &= swCfgGen->createSettingsProfile(SettingsProfile::DEFAULT);
			}
		}

		LOG_EMPTY_LINE(m_log);

		QStringList profileIDs = context->m_simProfiles.profiles();

		for(const QString& profileID : profileIDs)
		{
			if (profileID.isEmpty() == true || profileID == SettingsProfile::DEFAULT)
			{
				continue;
			}

			LOG_MESSAGE(m_log, QString("Software settings generation for profile: %1").
								arg(profileID));

			Sim::Profile& profile = context->m_simProfiles.profile(profileID);

			QStringList profileEquipmentIDs = profile.equipment();

			for(const QString& profileEquipmentID : profileEquipmentIDs)
			{
				std::shared_ptr<Hardware::DeviceObject> deviceObject = context->m_equipmentSet->deviceObject(profileEquipmentID);

				if (deviceObject == nullptr)
				{
					Q_ASSERT(false);	// this error should be detected early in checkProfiles()
					result = false;
					break;
				}

				QString errMsg;

				bool res = profile.applyToObject(profileEquipmentID, deviceObject, &errMsg);

				if (res == false)
				{
					LOG_INTERNAL_ERROR_MSG(m_log, errMsg);
					result = false;
					break;
				}
			}

			BREAK_IF_FALSE(result);

			for(auto p : swCfgGens)
			{
				result &= p.second->createSettingsProfile(profileID);
			}

			BREAK_IF_FALSE(result);

			profile.restoreObjects();

			LOG_EMPTY_LINE(m_log);
		}

		RETURN_IF_FALSE(result);

		// Software.xml file generation
		//
		QByteArray softwareXmlData;
		QXmlStreamWriter softwareXml(&softwareXmlData);

		softwareXml.setAutoFormatting(true);
		softwareXml.writeStartDocument();
		softwareXml.writeStartElement(XmlElement::SOFTWARE_ITEMS);	// <SoftwareItems>

		context->m_buildResultWriter->buildInfo().writeToXml(softwareXml);

		for(auto p : swCfgGens)
		{
			std::shared_ptr<SoftwareCfgGenerator> swCfgGen = p.second;

			// Writing settings of current software to Software.xml
			//
			swCfgGen->writeSoftwareSection(softwareXml, false);		// <Software>

			result &= swCfgGen->getSettingsXml(softwareXml);

			softwareXml.writeEndElement();							// </Software>
		}

		softwareXml.writeEndElement();								// </SoftwareItems>

		context->m_buildResultWriter->addFile(Directory::COMMON, File::SOFTWARE_XML, softwareXmlData);

		//
		// Software items configuration generation
		//

		for(auto p : swCfgGens)
		{
			std::shared_ptr<SoftwareCfgGenerator> swCfgGen = p.second;

			// Creating Configuration.xml for current software
			//
			result &= swCfgGen->createConfigurationXml();

			// First step of software configuration generation
			//
			LOG_MESSAGE(m_log, QString(tr("Configuration generation for software item (step 1): %1")).
						arg(swCfgGen->equipmentID()));

			result &= swCfgGen->generateConfigurationStep1();
		}

		for(auto p : swCfgGens)
		{
			std::shared_ptr<SoftwareCfgGenerator> swCfgGen = p.second;

			// Second step of software configuration generation
			//
			LOG_MESSAGE(m_log, QString(tr("Configuration generation for software item (step 2): %1")).
						arg(swCfgGen->equipmentID()));

			result &= swCfgGen->generateConfigurationStep2();
		}

		result &= context->m_buildResultWriter->writeConfigurationXmlFiles();

		LOG_EMPTY_LINE(context->m_log);

		if (result == false)
		{
			LOG_MESSAGE(context->m_log, tr("Sofware configuration generation was finished with errors"));
		}
		else
		{
			LOG_SUCCESS(context->m_log, tr("Sofware configuration generation was succesfully finished"));
		}

		SoftwareCfgGenerator::clearStaticData();

		return result;
	}

	bool BuildWorkerThread::checkProfiles()
	{
		Context* context = m_context.get();

		if (context->m_simProfiles.isEmpty() == true)
		{
			return true;
		}

		LOG_MESSAGE(m_log, QString("Software settings profiles checking..."));

		bool result = true;

		QStringList profileIDs = context->m_simProfiles.profiles();

		for(const QString& profileID : profileIDs)
		{
			Sim::Profile& profile = context->m_simProfiles.profile(profileID);

			QStringList profileEquipmentIDs = profile.equipment();

			for(const QString& profileEquipmentID : profileEquipmentIDs)
			{
				std::shared_ptr<Hardware::DeviceObject> deviceObject = context->m_equipmentSet->deviceObject(profileEquipmentID);

				if (deviceObject == nullptr)
				{
					// Equipment object %1 is not found (Settings profile - %2).
					//
					m_log->errCFG3044(profileEquipmentID,profileID);
					result = false;
					continue;
				}

				const Sim::ProfileProperties& pp = profile.properties(profileEquipmentID);

				for(auto p : pp.properties)
				{
					QString propertyName = p.first;

					if (deviceObject->propertyExists(propertyName) == false)
					{
						// Property %1.%2 is not found (Settings profile - %3).
						//
						m_log->errCFG3045(profileEquipmentID, propertyName, profileID);
						result = false;
						continue;
					}
				}
			}
		}

		if (result == true)
		{
			LOG_SUCCESS(m_log, QString("Ok"));
		}

		LOG_EMPTY_LINE(m_log);

		return result;
	}

	bool BuildWorkerThread::writeFirmwareStatistics(BuildResultWriter& buildResultWriter)
	{
		bool result = true;

		result &= buildResultWriter.writeFirmwareStatistics();

		return result;
	}

	bool BuildWorkerThread::writeBinaryFiles(BuildResultWriter &buildResultWriter)
	{
		bool result = true;

		result &= buildResultWriter.writeBinaryFiles();

		return result;
	}

	void BuildWorkerThread::generateModulesInformation(BuildResultWriter& buildWriter,
							   const std::vector<Hardware::DeviceModule *>& lmModules)
	{
		for (auto it = lmModules.begin(); it != lmModules.end(); it++)
		{
			Hardware::DeviceModule* lm = *it;

			if (lm == nullptr)
			{
				assert(lm);
				continue;
			}

			QString subsysID = lm->propertyValue("SubsystemID").toString();
			if (subsysID.isEmpty())
			{
				assert(false);
				continue;
			}

			bool ok = false;
			int lmNumber = lm->propertyValue("LMNumber").toInt(&ok);
			if (ok == false)
			{
				assert(false);
				continue;
			}

			QVariant channelVariant = lm->propertyValue("SubsystemChannel");
			if (channelVariant.canConvert<E::Channel>() == false)
			{
				assert(false);
				continue;
			}
			E::Channel subsystemChannel = channelVariant.value<E::Channel>();

			Hardware::ModuleFirmwareWriter* firmwareWriter = buildWriter.firmwareWriter();

			Hardware::ModuleFirmware& moduleFirmware = firmwareWriter->firmware(subsysID, &ok);
			if (ok == false)
			{
				// No module firmware exists for this module, maybe it
				continue;
			}

			moduleFirmware.addLogicModuleInfo(lm->equipmentId(), subsysID, lmNumber, subsystemChannel, lm->moduleFamily(), lm->customModuleFamily(), lm->moduleVersion(), lm->moduleType());
		}
	}

	bool BuildWorkerThread::generateLmsUniqueIDs(Context* context)
	{
		TEST_PTR_RETURN_FALSE(context);
		TEST_PTR_RETURN_FALSE(context->m_buildResultWriter);

		BuildResultWriter& buildWriter = *context->m_buildResultWriter.get();

		IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);

		const std::vector<Hardware::DeviceModule *>& lmModules = m_context->m_lmModules;
		std::map<QString, quint64>& lmsUniqueIDs = m_context->m_lmsUniqueIDs;

		lmsUniqueIDs.clear();

		bool result = true;

		//
		// Count Unique ID for this compilation
		//

		for (auto it = lmModules.begin(); it != lmModules.end(); it++)
		{
			Hardware::DeviceModule* lm = *it;

			if (lm == nullptr)
			{
				Q_ASSERT(lm);
				result = false;
				continue;
			}

			QString subsysID = lm->propertyValue(EquipmentPropNames::SUBSYSTEM_ID).toString();
			if (subsysID.isEmpty())
			{
				Q_ASSERT(false);
				result = false;
				continue;
			}

			bool ok = false;
			int lmNumber = lm->propertyValue(EquipmentPropNames::LM_NUMBER).toInt(&ok);
			if (ok == false)
			{
				Q_ASSERT(false);
				result = false;
				continue;
			}

			Hardware::ModuleFirmwareWriter* firmwareWriter = buildWriter.firmwareWriter();

			quint64 genericUniqueId = 0;
			bool first = true;

			Hardware::ModuleFirmware& moduleFirmware = firmwareWriter->firmware(subsysID, &ok);
			if (ok == false)
			{
				Q_ASSERT(ok);
				continue;
			}

			std::vector<UartPair> uarts = moduleFirmware.uartList();

			for (auto fi : uarts)
			{
				int uartId = fi.first;

				quint64 uniqueID = firmwareWriter->uniqueID(subsysID, uartId, lmNumber, &ok);
				if (ok == false)
				{
					log->errINT1001(tr("UniqueID is not found for Subsystem='%1', UartID='%2', LmNumber='%3'. ").arg(subsysID).arg(uartId).arg(lmNumber) + Q_FUNC_INFO);
					result = false;
					continue;
				}

				if (first == true)
				{
					first = false;
					genericUniqueId = uniqueID;
				}
				else
				{
					genericUniqueId ^= uniqueID;
				}
			}

			firmwareWriter->setGenericUniqueId(subsysID, lmNumber, genericUniqueId);

			lmsUniqueIDs.insert({ lm->equipmentIdTemplate(), genericUniqueId });
		}

		if (result == false)
		{
			LOG_INTERNAL_ERROR_MSG(log, "Can't generate LMs unique IDs");
		}

		return result;
	}

	bool BuildWorkerThread::writeLogicModulesInfoXml(Context* context)
	{
		TEST_PTR_RETURN_FALSE(context);
		TEST_PTR_RETURN_FALSE(context->m_buildResultWriter);

		LogicModulesInfoWriter writer(*context);

		writer.fill();

		QByteArray xmlData;

		writer.save(&xmlData);

		bool result = true;

		BuildFile* file = context->m_buildResultWriter->addFile(Directory::COMMON, File::LOGIC_MODULES_XML, "", "", xmlData);

		if (file == nullptr)
		{
			result = false;
		}

		return result;
	}

	bool BuildWorkerThread::buildSoftwareList(Context* context)
	{
		TEST_PTR_RETURN_FALSE(context);

		IssueLogger* log = context->m_log;

		TEST_PTR_RETURN_FALSE(log);

		Hardware::EquipmentSet* equipment = context->m_equipmentSet.get();

		TEST_PTR_LOG_RETURN_FALSE(equipment, log);

		context->m_software.clear();

		bool result = true;

		equipmentWalker(equipment->root().get(), [context, &result](Hardware::DeviceObject* currentDevice)
			{
				if (currentDevice == nullptr)
				{
					Q_ASSERT(false);
					result = false;
					return;
				}

				if (currentDevice->isSoftware() == false)
				{
					return;
				}

				Hardware::Software* software = currentDevice->toSoftware().get();

				if (software == nullptr)
				{
					Q_ASSERT(false);
					result = false;
					return;
				}

				context->m_software.insert( { software->equipmentId(), software} );
			}
		);

		if (result == true)
		{
			LOG_MESSAGE(log, QString(tr("Software list building... OK")));
		}
		else
		{
			LOG_INTERNAL_ERROR_MSG(log, QString(tr("Can't build software list")));
		}

		return result;
	}

	bool BuildWorkerThread::runSimTests()
	{
		Q_ASSERT(m_context);

		if (m_context->m_projectProperties.runSimTestsOnBuild() == false)
		{
			return true;
		}

		LOG_MESSAGE(m_context->m_log, tr(""));
		LOG_MESSAGE(m_context->m_log, tr("Run simulator-based tests..."));

		DbController& db = m_context->m_db;

		// Get test scripts
		//
		DbFileTree scriptFilesTree;
		int simTestsFileId = db.systemFileId(DbDir::SimTestsDir);

		bool ok = db.getFileListTree(&scriptFilesTree, simTestsFileId, true, nullptr);
		if (ok == false)
		{
			m_context->m_log->errPDB2001(simTestsFileId, "", db.lastError());
			return false;
		}

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			return true;
		}

		std::vector<DbFileInfo> fileInfos = scriptFilesTree.toVector(true);

		auto filter = [](const DbFileInfo& fi) -> bool
		{
			return fi.isFolder() || fi.fileName().endsWith(".js") == false || fi.deleted() == true;
		};

		fileInfos.erase(std::remove_if(fileInfos.begin(),
									   fileInfos.end(),
									   filter),
						fileInfos.end());

		if (fileInfos.empty() == true)
		{
			return true;
		}

		std::vector<std::shared_ptr<DbFile>> files;

		ok = db.getLatestVersion(fileInfos, &files, nullptr);
		if (ok == false)
		{
			m_context->m_log->errPDB2001(simTestsFileId, "", db.lastError());
			return false;
		}

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			return true;
		}

		// Run tests
		//
		std::vector<Sim::SimScriptItem> testScripts;
		testScripts.reserve(files.size());

		for (const std::shared_ptr<DbFile>& f : files)
		{
			testScripts.emplace_back(Sim::SimScriptItem{f->data(), scriptFilesTree.filePath(f->fileId()) + "/" + f->fileName()});
		}

		class SimLogger : public ILogFile
		{
			OutputLog* m_log = nullptr;

		public:
			SimLogger(OutputLog* log) : m_log(log) {}

			virtual bool writeAlert(const QString& text) override
			{
				m_log->writeError(text); return true;
			};
			virtual bool writeError(const QString& text) override
			{
				m_log->writeError(text); return true;
			};
			virtual bool writeWarning(const QString& text) override
			{
				m_log->writeWarning0(text); return true;
			};
			virtual bool writeMessage(const QString& text) override
			{
				m_log->writeMessage(text); return true;
			};
			virtual bool writeText(const QString& text) override
			{
				m_log->writeMessage(text); return true;
			};
		};

		SimLogger simLogger(m_log);
		QString buildPath = m_context->m_buildResultWriter->fullOutputPathes()[0];

		Sim::Simulator simulator{&simLogger, nullptr};

		ok = simulator.load(buildPath);
		if (ok == false)
		{
			return false;
		}

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			return true;
		}

		qint64 timeout = m_context->m_projectProperties.simTestsTimeout();

		// Add modules to simulation
		//
		simulator.control().setRunList({});

		// Run scripts
		//
		ok = simulator.runScripts(testScripts, timeout);
		if (ok == false)
		{
			return false;
		}

		ok = simulator.waitScript(timeout < 0 ? ULONG_MAX : static_cast<unsigned long>(timeout));
		if (ok == false)
		{
			return false;
		}

		ok = simulator.scriptResult();

		return ok;
	}

	QString BuildWorkerThread::projectName() const
	{
		return m_projectName;
	}

	void BuildWorkerThread::setProjectName(QString value)
	{
		m_projectName = value;
	}

	QString BuildWorkerThread::serverIpAddress() const
	{
		return m_serverIpAddress;
	}

	void BuildWorkerThread::setServerIpAddress(QString value)
	{
		m_serverIpAddress = value;
	}

	int BuildWorkerThread::serverPort() const
	{
		return m_serverPort;
	}

	void BuildWorkerThread::setServerPort(int value)
	{
		m_serverPort = value;
	}

	QString BuildWorkerThread::serverUsername() const
	{
		return m_serverUsername;
	}

	void BuildWorkerThread::setServerUsername(QString value)
	{
		m_serverUsername = value;
	}

	QString BuildWorkerThread::serverPassword() const
	{
		return m_serverPassword;
	}

	void BuildWorkerThread::setServerPassword(QString value)
	{
		m_serverPassword = value;
	}

	void BuildWorkerThread::setIssueLog(IssueLogger* value)
	{
		m_log = value;
	}

	QString BuildWorkerThread::projectUserName() const
	{
		return m_projectUserName;
	}

	void BuildWorkerThread::setProjectUserName(QString value)
	{
		m_projectUserName = value;
	}

	QString BuildWorkerThread::projectUserPassword() const
	{
		return m_projectUserPassword;
	}

	void BuildWorkerThread::setProjectUserPassword(QString value)
	{
		m_projectUserPassword = value;
	}

	QString BuildWorkerThread::buildOutputPath() const
	{
		return m_buildOutputPath;
	}

	void BuildWorkerThread::setBuildOutputPath(QString value)
	{
		m_buildOutputPath = value;
	}

	bool BuildWorkerThread::expertMode() const
	{
		return m_expertMode;
	}

	void BuildWorkerThread::setExpertMode(bool value)
	{
		m_expertMode = value;
	}

	bool BuildWorkerThread::isInterruptRequested()
	{
		return QThread::currentThread()->isInterruptionRequested();
	}

	int BuildWorkerThread::progress() const
	{
		if (m_context != nullptr)
		{
			return m_context->m_progress;
		}
		else
		{
			return 0;
		}
	}
}
