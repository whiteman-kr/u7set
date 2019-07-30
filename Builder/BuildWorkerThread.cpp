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
		m_context = std::make_unique<Context>(m_log, buildOutputPath(), debug(), expertMode());
		std::shared_ptr<int*> progressCompleted(nullptr, [this](void*)
			{
				this->m_context.reset();		// this will release m_context on leaving run()
			});

		assert(m_context->m_log);

		QThread::currentThread()->setTerminationEnabled(true);

		QTime buildTime;
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
		m_context->m_buildResultWriter = std::make_shared<BuildResultWriter>(buildOutputPath());
		m_context->m_buildResultWriter->start(&m_context->m_db, m_context->m_log, release(), 0 /* Load correct ChangesetID */);

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

			int checkedOutCount = 0;
			ok = m_context->m_db.isAnyCheckedOut(&checkedOutCount);

			if (ok == false)
			{
				LOG_ERROR_OBSOLETE(m_context->m_log, Builder::IssueType::NotDefined, tr("isAnyCheckedOut Error."));
				break;
			}

			if (release() == true && checkedOutCount > 0)
			{
				LOG_ERROR_OBSOLETE(m_context->m_log, Builder::IssueType::NotDefined,
						  tr("There are some checked out objects (%1). Please check in all objects before building release version.").arg(checkedOutCount));
				break;
			}

			const BuildInfo& bi = m_context->m_buildResultWriter->buildInfo();
			m_context->m_buildResultWriter->firmwareWriter()->setProjectInfo(bi.project, bi.user, bi.id, bi.release == false, bi.changeset);

			//
			// Get Equipment from the database
			//
			if (ok = getEquipment();
				ok == false)
			{
				break;
			}

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
			if (bool ok = loadConnections();
				ok == false)
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
			if (bool pareseOk = parseApplicationLogic();
				pareseOk == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

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
			m_context->m_comparatorStorage = std::make_shared<ComparatorStorage>();

			ok = compileApplicationLogic();

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

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

			LmsUniqueIdMap lmsUniqueIdMap;
			generateLmsUniqueID(*m_context->m_buildResultWriter, m_context->m_lmModules, lmsUniqueIdMap);

			//
			// Generate MATS software configurations
			//
			ok = generateSoftwareConfiguration(lmsUniqueIdMap);

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

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

			//
			// Write Firmware Statistics
			//

			ok = writeFirmwareStatistics(*m_context->m_buildResultWriter);

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

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
		int buildEllapsed = buildTime.elapsed() / 1000;
		int durationSecs = buildEllapsed % 60;
		int durationMins = buildEllapsed / 60;
		LOG_MESSAGE(m_context->m_log, QString("Build time: %1 minute(s) %2 second(s)").arg(durationMins).arg(durationSecs));

		// We've done, exit
		//
		qDebug("Leave BuildWorkerThread::run()");

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

		std::shared_ptr<Hardware::DeviceObject> deviceRoot = std::make_shared<Hardware::DeviceRoot>();

		int rootFileId = m_context->m_db.hcFileId();
		deviceRoot->fileInfo().setFileId(rootFileId);

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

		if (bool ok = checkUuidAndStrId(m_context->m_equipmentSet->root());
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

		// Get file list with checked out files,
		// if this is release build, specific copies will be fetched later
		//
		ok = m_context->m_db.getFileList(&files, parent->fileInfo().fileId(), true, nullptr);

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
			Hardware::DeviceObject* child = object->child(i);

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
			Hardware::DeviceObject* child = object->child(i);

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
			ok &= checkUuidAndStrIdWorker(device->child(i), uuidMap, strIdMap);
		}

		return ok;
	}


	bool BuildWorkerThread::checkChildRestrictions(Hardware::DeviceObject* root)
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

	bool BuildWorkerThread::checkChildRestrictionsWorker(Hardware::DeviceObject* device)
	{
		assert(device != nullptr);

		QString errorMessage;

		int childrenCount = device->childrenCount();

		for (int i = 0; i < childrenCount; i++)
		{
			Hardware::DeviceObject* child = device->child(i);

			if (child == nullptr)
			{
				assert(child);
				return false;
			}

			bool allowed = device->checkChild(child, &errorMessage);

			if (allowed == false)
			{
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
		if (signalSet == nullptr ||
			equipment == nullptr)
		{
			assert(false);
			return false;
		}

		LOG_EMPTY_LINE(m_context->m_log);

		LOG_MESSAGE(m_context->m_log, tr("Loading application signals"));

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

		result = signalSet->checkSignals();

		if (result == false)
		{
			return false;
		}

		//QStringList

		signalSet->buildID2IndexMap();

		result = signalSet->bindSignalsToLMs(equipment);

		if (result == false)
		{
			return false;
		}

		signalSet->initCalculatedSignalsProperties();

		LOG_SUCCESS(m_context->m_log, tr("Ok"));

		return true;
	}

	bool BuildWorkerThread::loadBusses()
	{
		m_context->m_busSet = std::make_shared<VFrame30::BusSet>();

		// Get Busses
		//
		std::vector<DbFileInfo> fileList;

		bool ok = m_context->m_db.getFileList(&fileList, m_context->m_db.busTypesFileId(), Db::File::BusFileExtension, true, nullptr);
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

		findModulesByFamily(m_context->m_equipmentSet->root(), &m_context->m_lmModules, Hardware::DeviceModule::FamilyType::LM);

		findModulesByFamily(m_context->m_equipmentSet->root(), &m_context->m_lmAndBvbModules, Hardware::DeviceModule::FamilyType::LM);
		findModulesByFamily(m_context->m_equipmentSet->root(), &m_context->m_lmAndBvbModules, Hardware::DeviceModule::FamilyType::BVB);

		bool ok = true;
		for (Hardware::DeviceModule* lm : m_context->m_lmAndBvbModules)
		{
			ok &= loadLogicModuleDescription(lm, m_context->m_lmDescriptions.get());
		}

		if (ok == false)
		{
			return false;
		}

		findFSCConfigurationModules(m_context->m_equipmentSet->root(), &m_context->m_fscModules);

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

		m_context->m_opticModuleStorage = std::make_shared<Hardware::OptoModuleStorage>(m_context->m_equipmentSet.get(),
																						m_context->m_fscDescriptions.get(),
																						m_context->m_connections.get(),
																						m_context->m_log);
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

		if (bool ok = db->getFileListTree(&fileTree, db->schemaFileId(), true, nullptr);
			ok == false)
		{
			m_context->m_log->errPDB2001(db->schemaFileId(), "", db->lastError());
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


	bool BuildWorkerThread::generateSoftwareConfiguration(const LmsUniqueIdMap& lmsUniqueIdMap)
	{
		Context* context = m_context.get();

		bool result = true;

		LOG_EMPTY_LINE(context->m_log);
		LOG_MESSAGE(context->m_log, QString(tr("MATS configuration generation...")))
		LOG_EMPTY_LINE(context->m_log);

		result &= SoftwareCfgGenerator::generalSoftwareCfgGeneration(context);

		equipmentWalker(context->m_equipmentSet->root(),
			[this, lmsUniqueIdMap, context, &result](Hardware::DeviceObject* currentDevice)
			{
				if (QThread::currentThread()->isInterruptionRequested() == true)
				{
					return;
				}

				if (currentDevice->isSoftware() == false)
				{
					return;
				}

				Hardware::Software* software = dynamic_cast<Hardware::Software*>(currentDevice);

				if (software == nullptr)
				{
					assert(false);
					return;
				}

				SoftwareCfgGenerator* softwareCfgGenerator = nullptr;

				switch(software->type())
				{
				case E::SoftwareType::AppDataService:
					softwareCfgGenerator = new AppDataServiceCfgGenerator(context, software, lmsUniqueIdMap);
					break;

				case E::SoftwareType::DiagDataService:
					softwareCfgGenerator = new DiagDataServiceCfgGenerator(context, software);
					break;

				case E::SoftwareType::Monitor:
					softwareCfgGenerator = new MonitorCfgGenerator(context, software);
					break;

				case E::SoftwareType::TuningService:
					softwareCfgGenerator = new TuningServiceCfgGenerator(context, software, lmsUniqueIdMap);
					break;

				case E::SoftwareType::TuningClient:
					softwareCfgGenerator = new TuningClientCfgGenerator(context, software);
					break;

				case E::SoftwareType::ConfigurationService:
					softwareCfgGenerator = new ConfigurationServiceCfgGenerator(context, software);
					break;

				case E::SoftwareType::ArchiveService:
					softwareCfgGenerator = new ArchivingServiceCfgGenerator(context, software);
					break;

				case E::SoftwareType::Metrology:
					softwareCfgGenerator = new MetrologyCfgGenerator(context, software);
					break;

				case E::SoftwareType::TestClient:
					softwareCfgGenerator = new TestClientCfgGenerator(context, software);
					break;

				default:
					m_context->m_log->errEQP6100(software->equipmentIdTemplate(), software->uuid());
					result = false;
				}

				if (softwareCfgGenerator != nullptr)
				{
					result &= softwareCfgGenerator->run();

					delete softwareCfgGenerator;
				}
			}
		);

		context->m_buildResultWriter->writeConfigurationXmlFiles();

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

	void BuildWorkerThread::generateLmsUniqueID(BuildResultWriter& buildWriter,
												const std::vector<Hardware::DeviceModule *>& lmModules,
												LmsUniqueIdMap &lmsUniqueIdMap)
	{
		lmsUniqueIdMap.clear();

		//
		// Count Unique ID for this compilation
		//

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

			Hardware::ModuleFirmwareWriter* firmwareWriter = buildWriter.firmwareWriter();

			quint64 genericUniqueId = 0;
			bool first = true;

			Hardware::ModuleFirmware& moduleFirmware = firmwareWriter->firmware(subsysID, &ok);
			if (ok == false)
			{
				assert(ok);
				continue;
			}

			std::vector<UartPair> uarts = moduleFirmware.uartList();

			for (auto fi : uarts)
			{
				int uartId = fi.first;

				if (first == true)
				{
					first = false;
					genericUniqueId = firmwareWriter->uniqueID(subsysID, uartId, lmNumber);
				}
				else
				{
					genericUniqueId ^= firmwareWriter->uniqueID(subsysID, uartId, lmNumber);
				}
			}

			firmwareWriter->setGenericUniqueId(subsysID, lmNumber, genericUniqueId);

			lmsUniqueIdMap.insert(lm->equipmentIdTemplate(), genericUniqueId);
		}
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

	DbProjectProperties BuildWorkerThread::projectProperties() const
	{
		return m_projectProperties;
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
}
