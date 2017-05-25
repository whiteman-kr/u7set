#include "Builder.h"
#include "Parser.h"
#include "TuningBuilder.h"
#include "ConfigurationBuilder.h"
#include "Subsystem.h"
#include "GlobalMessanger.h"

#include "../../lib/DbController.h"
#include "../../lib/DeviceObject.h"

#include "../../VFrame30/LogicSchema.h"
#include "../../VFrame30/SchemaItemLink.h"
#include "../../VFrame30/HorzVertLinks.h"

#include "ApplicationLogicCompiler.h"
#include "SoftwareCfgGenerator.h"
#include "AppDataServiceCfgGenerator.h"
#include "DiagDataServiceCfgGenerator.h"
#include "MonitorCfgGenerator.h"
#include "TuningServiceCfgGenerator.h"
#include "ArchivingServiceCfgGenerator.h"
#include "TuningClientCfgGenerator.h"
#include "MetrologyCfgGenerator.h"
#include "IssueLogger.h"

#include <functional>
#include <set>

#include <QBuffer>
#include <QtConcurrent/QtConcurrent>



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

		// Start logging to output string, this string will be written as file to build output
		//
		m_log->startStrLogging();
		m_log->clearItemsIssues();

		GlobalMessanger::instance()->clearBuildSchemaIssues();
		GlobalMessanger::instance()->clearSchemaItemRunOrder();

		// Log softaware version
		//
		LOG_MESSAGE(m_log, qApp->applicationName() + " v" + qApp->applicationVersion());

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
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, db.lastError());
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Opening project %1: error").arg(projectName()));
			return;
		}
		else
		{
			LOG_MESSAGE(m_log, tr("Opening project %1: ok").arg(projectName()));
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
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("lastChangesetId Error."));
				break;
			}

			int checkedOutCount = 0;
			ok = db.isAnyCheckedOut(&checkedOutCount);

			if (ok == false)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("isAnyCheckedOut Error."));
				break;
			}

			if (release() == true && checkedOutCount > 0)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  tr("There are some checked out objects (%1). Please check in all objects before building release version.").arg(checkedOutCount));
				break;
			}

			//
			// Get Equipment from the database
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Getting equipment"));

			std::shared_ptr<Hardware::DeviceObject> deviceRoot = std::make_shared<Hardware::DeviceRoot>();

			int rootFileId = db.hcFileId();
			deviceRoot->fileInfo().setFileId(rootFileId);

			bool ok = getEquipment(&db, deviceRoot.get());

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			if (ok == false)
			{
				break;
			}
			else
			{
				LOG_SUCCESS(m_log, tr("Ok"));
			}

			//
			// Expand Devices StrId
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Expanding devices StrIds"));

			expandDeviceStrId(deviceRoot.get());

			LOG_SUCCESS(m_log, tr("Ok"));

			Hardware::EquipmentSet equipmentSet(deviceRoot);
			deviceRoot.reset();		// Use equipmentSet.root() instead

			//
			// Check same Uuids and same StrIds
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Checking for same Uuids and StrIds"));

			ok = checkUuidAndStrId(equipmentSet.root());

			if (ok == false)
			{
				break;
			}

            //
            // Check child restirictions
            //

            ok = checkChildRestrictions(equipmentSet.root());

            if (ok == false)
            {
                break;
            }

			LOG_SUCCESS(m_log, tr("Ok"));

			//
			// SignalSet
			//
			SignalSet signalSet;

			if (loadSignals(&db, &signalSet, equipmentSet) == false)
			{
				break;
			}

			//
			// Find all LM Modules and load their descriptions
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Loading LogicModule descriptions..."));

			std::vector<Hardware::DeviceModule*> lmModules;
			findLmModules(equipmentSet.root(), &lmModules);

			LmDescriptionSet lmDescriptions;

			ok = true;
			for (Hardware::DeviceModule* lm : lmModules)
			{
				ok &= loadLogicModuleDescription(&db, lm, &lmDescriptions);
			}

			if (ok == false)
			{
				break;
			}
			else
			{
				LOG_SUCCESS(m_log, tr("Ok"));
			}

			//
			// Loading subsystems
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Loading subsystems..."));

			Hardware::SubsystemStorage subsystems;

			ok = loadSubsystems(db, lmModules, &subsystems);

			if (ok == false)
			{
				break;
			}
			else
			{
				LOG_SUCCESS(m_log, tr("Ok"));
			}

			//
			// Loading connections
			//
			QString errorCode;

            Hardware::ConnectionStorage connections(&db, nullptr);

            ok = connections.load();

			if (ok == false)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Can't load connections file"));
				if (errorCode.isEmpty() == false)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, errorCode);
					break;
				}
			}

			Hardware::OptoModuleStorage opticModuleStorage(&equipmentSet, &lmDescriptions, m_log);

			//
			// Parse application logic
			//
			AppLogicData appLogicData;

			ok = parseApplicationLogic(&db, &appLogicData, lmDescriptions, &equipmentSet, &signalSet, lastChangesetId);

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Compile application logic
			//
			Tuning::TuningDataStorage tuningDataStorage;
			ComparatorStorage comparatorStorage;

			ok = compileApplicationLogic(&subsystems, &equipmentSet, &opticModuleStorage, &connections, &signalSet, &lmDescriptions, &appLogicData, &tuningDataStorage, &comparatorStorage, &buildWriter);

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Tuning parameters
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Tuning parameters compilation"));

			TuningBuilder tuningBuilder(&db, equipmentSet.root(), &signalSet, &subsystems, &tuningDataStorage, m_log,
                                        buildWriter.buildInfo().id, lastChangesetId, debug(), projectName(), projectUserName(), lmModules, &lmDescriptions);

			ok = tuningBuilder.build();

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Compile Module configuration
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Module configurations compilation"));

            ConfigurationBuilder cfgBuilder(this, &db, equipmentSet.root(), lmModules, &lmDescriptions, &signalSet, &subsystems, &opticModuleStorage, m_log,
                                               buildWriter.buildInfo().id, lastChangesetId, debug(), projectName(), projectUserName());

			ok = cfgBuilder.build(buildWriter);


			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			LmsUniqueIdMap lmsUniqueIdMap;

			generateLmsUniqueID(buildWriter, tuningBuilder, cfgBuilder, lmModules, lmsUniqueIdMap);

			//
			// Generate SCADA software configurations
			//
			ok = generateSoftwareConfiguration(&db, &subsystems, &equipmentSet, &signalSet, &tuningDataStorage, lmsUniqueIdMap, &buildWriter);

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Write logic, configuration and tuning binary files
			//

			ok = writeBinaryFiles(buildWriter);

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			ok = tuningBuilder.writeBinaryFiles(buildWriter);

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			ok = cfgBuilder.writeBinaryFiles(buildWriter);

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			LOG_SUCCESS(m_log, tr("Ok"));
		}
		while (false);

		buildWriter.finish();

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			// The build was cancelled.
			//
			m_log->errCMN0016();

			m_log->clear();		// Log can contain thouthands of messages, if it some kind iof "same ids" error
		}

		db.closeProject(nullptr);

		// Set Shceme Items Issues to GlobalMessanger
		//
		BuildIssues m_buildIssues;
		m_log->swapItemsIssues(&m_buildIssues);

		GlobalMessanger::instance()->swapSchemaIssues(&m_buildIssues);

		// We've done, exit
		//
		emit resultReady(QString("Cool, we've done!"));

		qDebug("Leave BuildWorkerThread::run()");

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

		// --
		//
		std::vector<DbFileInfo> files;

		bool ok = false;

		// Get file list with checked out files,
		// if this is release build, specific copies will be fetched later
		//
		ok = db->getFileList(&files, parent->fileInfo().fileId(), true, nullptr);

		if (ok == false)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Cannot get equipment file list"));
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

			LOG_MESSAGE(m_log, tr("Getting equipment object, fileid: %1, details: %2").arg(fi.fileId()).arg(fi.details()));

			std::shared_ptr<Hardware::DeviceObject> device;
			ok = db->getDeviceTreeLatestVersion(fi, &device, nullptr);

			if (ok == false ||
				device.get() == nullptr)
			{
				LOG_ERROR_OBSOLETE(m_log, "", tr("Failed to load equipment, fileid: %1").arg(fi.fileId()));
				continue;
			}

			parent->addChild(device);
		}

		return true;
	}

	void BuildWorkerThread::findLmModules(Hardware::DeviceObject* object, std::vector<Hardware::DeviceModule*>* out) const
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

				if (module->isLogicModule() == true)
				{
					out->push_back(module);
				}
			}

			if (child->deviceType() < Hardware::DeviceType::Module)
			{
				findLmModules(child, out);
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

	bool BuildWorkerThread::loadSubsystems(DbController& db, const std::vector<Hardware::DeviceModule*>& logicMoudles, Hardware::SubsystemStorage* subsystems)
	{
		if (subsystems == nullptr)
		{
			assert(subsystems);
			return false;
		}

		QString errorCode;

		bool ok = subsystems->load(&db, errorCode);

		if (ok == false)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Can't load subsystems file"));
			if (errorCode.isEmpty() == false)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, errorCode);
				return false;
			}
		}

		// Check if subsystems have same id or sskey
		//
		std::set<QString> ids;
		std::set<int> sskeys;

		bool result = true;

		for (std::shared_ptr<Hardware::Subsystem> subsystem : subsystems->subsystems())
		{
			assert(subsystem);

			if (ids.count(subsystem->subsystemId()) > 0)
			{
				// Duplicate SubsystemID
				//
				result = false;
				m_log->errEQP6005(subsystem->subsystemId());
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
				m_log->errEQP6006(subsystem->key());
			}
			else
			{
				sskeys.insert(subsystem->key());
			}
		}

		// Check if all LMs in subsystem have the same version and LmDescriptionFile
		//
		for (std::shared_ptr<Hardware::Subsystem> subsystem : subsystems->subsystems())
		{
			assert(subsystem);

			Hardware::DeviceModule::FamilyType moduleFamily = Hardware::DeviceModule::FamilyType::OTHER;
			int moduleVersion = -1;
			QString LmDescriptionFile;

			for (const Hardware::DeviceModule* lm : logicMoudles)
			{
				assert(lm);
				assert(lm->isLogicModule() == true);

				auto lmSubsystemIdProp = lm->propertyByCaption(Hardware::PropertyNames::lmSubsystemID);
				if (lmSubsystemIdProp == nullptr)
				{
					m_log->errCFG3000(Hardware::PropertyNames::lmSubsystemID, lm->equipmentIdTemplate());
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
						m_log->errEQP6007(subsystem->subsystemId());
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
						m_log->errEQP6007(subsystem->subsystemId());
					}
				}

				// Check LmDescriptionFile
				//
				auto LmDescriptionFileProp = lm->propertyByCaption(Hardware::PropertyNames::lmDescriptionFile);
				if (LmDescriptionFileProp == nullptr)
				{
					m_log->errCFG3000(Hardware::PropertyNames::lmDescriptionFile, lm->equipmentIdTemplate());
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
						m_log->errEQP6007(subsystem->subsystemId());
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
			m_log->errEQP6002(device->uuid(), device->equipmentId(), foundSameUuid->second->equipmentId());
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
			m_log->errEQP6001(device->equipmentId(), device->uuid(), foundSameStrId->second->uuid()) ;
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
				m_log->errEQP6009(module->equipmentIdTemplate(), module->uuid());
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
			m_log->errEQP6000(device->equipmentIdTemplate(), device->uuid());
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
                m_log->errEQP6008(device->equipmentId(), child->equipmentId(), child->place());
                return false;
            }

            if (checkChildRestrictionsWorker(child) == false)
            {
                return false;
            }
        }

        return true;
    }


    bool BuildWorkerThread::loadSignals(DbController* db, SignalSet* signalSet, Hardware::EquipmentSet& equipment)
	{
		if (db == nullptr ||
			signalSet == nullptr)
		{
			assert(false);
			return false;
		}

		LOG_EMPTY_LINE(m_log);

		LOG_MESSAGE(m_log, tr("Loading application logic signals"));

		bool result = db->getSignals(signalSet, nullptr);

		if (result == false)
		{
			// Load signals from the project database error
			//
			m_log->errPDB2003();
			return false;
		}

		result = db->getUnits(Signal::unitList.get(), nullptr);

		if (result == false)
		{
			// Load units from the project database error
			//
			m_log->errPDB2004();
			return false;
		}

		LOG_MESSAGE(m_log, QString(tr("Checking application signals")));

		// Check some signals's properties and init Signal::lm property
		//
		result = checkAppSignals(*signalSet, equipment);

		if (result == false)
		{
			return false;
		}

		LOG_SUCCESS(m_log, tr("Ok"));

		signalSet->buildID2IndexMap();

		return true;
	}


	bool BuildWorkerThread::checkAppSignals(SignalSet& signalSet, Hardware::EquipmentSet& equipment)
	{
		bool result = true;

		int signalCount = signalSet.count();

		if (signalCount == 0)
		{
			return true;
		}

		QHash<QString, int> appSignalIDMap;
		QHash<QString, int> customAppSignalIDMap;

		for(int i = 0; i < signalCount; i++)
		{
			Signal& s = signalSet[i];

			// check AppSignalID
			//
			if (appSignalIDMap.contains(s.appSignalID()) == true)
			{
				// Application signal identifier '%1' is not unique.
				//
				m_log->errALC5016(s.appSignalID());
				result = false;
				continue;
			}
			else
			{
				appSignalIDMap.insert(s.appSignalID(), i);
			}

			// check CustomAppSignalID
			//
			if (customAppSignalIDMap.contains(s.customAppSignalID()) == true)
			{
				// Custom application signal identifier '%1' is not unique.
				//
				m_log->errALC5017(s.customAppSignalID());
				result = false;
				continue;
			}
			else
			{
				customAppSignalIDMap.insert(s.customAppSignalID(), i);
			}

			// check EquipmentID
			//
			s.setLm(nullptr);

			if (s.equipmentID().isEmpty() == true)
			{
				// Application signal '%1' is not bound to any device object.
				//
				m_log->wrnALC5012(s.appSignalID());
			}
			else
			{
				std::shared_ptr<Hardware::DeviceObject> deviceObjectShared = equipment.deviceObjectSharedPointer(s.equipmentID());

				if (deviceObjectShared == nullptr)
				{
					// Application signal '%1' is bound to unknown device object '%2'.
					//
					m_log->errALC5013(s.appSignalID(), s.equipmentID());
					result = false;
					continue;
				}

				Hardware::DeviceObject* deviceObject = deviceObjectShared.get();

				if (deviceObject->isModule() == true)
				{
					// device is Module
					//
					Hardware::DeviceModule* module = deviceObject->toModule();

					if (module == nullptr)
					{
						assert(false);
						continue;
					}

					if (module->isLogicModule())
					{
						s.setLm(std::dynamic_pointer_cast<Hardware::DeviceModule>(deviceObjectShared));
					}
					else
					{
						// The signal '%1' can be bind to Logic Module or Equipment Signal.
						//
						m_log->errALC5031(s.appSignalID());
						result = false;
						continue;
					}
				}
				else
				{
					if (deviceObject->isSignal())
					{
						// device is Signal
						//
						Hardware::DeviceChassis* chassis = const_cast<Hardware::DeviceChassis*>(deviceObject->getParentChassis());

						if (chassis == nullptr)
						{
							assert(false);
							continue;
						}

						std::shared_ptr<Hardware::DeviceModule> lm = chassis->getLogicModuleSharedPointer();

						if (lm != nullptr)
						{
							s.setLm(lm);
						}
						else
						{
							// Can't find logic module associated with signal '%1' (no LM in chassis '%2').
							//
							m_log->errALC5033(s.appSignalID(), chassis->equipmentId());
							result = false;
							continue;
						}
					}
					else
					{
						// The signal '%1' can be bind to Logic Module or Equipment Signal.
						//
						m_log->errALC5031(s.appSignalID());
						result = false;
						continue;
					}
				}
			}

			// check other signal properties
			//
			if (s.isDiscrete())
			{
				if (s.dataSize() != 1)
				{
					// Discrete signal '%1' must have DataSize equal to 1.
					//
					m_log->errALC5014(s.appSignalID());
					result = false;
				}
			}
			else
			{
				assert(s.isAnalog() == true);

				if (s.dataSize() != 32)
				{
					// Analog signal '%1' must have DataSize equal to 32.
					//
					m_log->errALC5015(s.appSignalID());
					result = false;
				}
			}
		}

		return result;
	}


	bool BuildWorkerThread::loadLogicModuleDescription(DbController* db, Hardware::DeviceModule* logicModule, LmDescriptionSet* lmDescriptions)
	{
		if (db == nullptr ||
			logicModule == nullptr ||
			lmDescriptions == nullptr)
		{
			assert(db);
			assert(logicModule);
			assert(lmDescriptions);
			return false;
		}

		// Get LmDescriptionFile property value
		//
		auto lmDescriptionFileProp = logicModule->propertyByCaption(Hardware::PropertyNames::lmDescriptionFile);

		if (lmDescriptionFileProp == nullptr)
		{
			m_log->errCFG3000(Hardware::PropertyNames::lmDescriptionFile, logicModule->equipmentIdTemplate());
			return false;
		}

		QString lmDescriptionFile = lmDescriptionFileProp->value().toString();

		if (lmDescriptionFile.isEmpty() == true)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QObject::tr("Property LmDescriptionFile is empty. LogicModule %1").arg(logicModule->equipmentIdTemplate()));
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
		bool ok = lmDescriptions->loadFile(m_log, db, logicModule->equipmentIdTemplate(), lmDescriptionFile);

		return ok;
	}


	bool BuildWorkerThread::parseApplicationLogic(DbController* db,
												  AppLogicData* appLogicData,
												  LmDescriptionSet& lmDescriptions,
												  Hardware::EquipmentSet* equipment,
												  SignalSet* signalSet,
												  int changesetId)
	{
		if (db == nullptr ||
			appLogicData == nullptr ||
			equipment == nullptr ||
			signalSet == nullptr)
		{
			assert(db);
			assert(appLogicData);
			assert(equipment);
			assert(signalSet);
			return false;
		}

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, tr("Application Logic parsing..."));

		Parser alPareser(db, m_log, appLogicData, &lmDescriptions, equipment, signalSet, changesetId, debug());

		bool result = alPareser.parse();

		if (result == false)
		{
		}
		else
		{
			LOG_SUCCESS(m_log, tr("Ok"));
		}

		return result;
	}


	bool BuildWorkerThread::compileApplicationLogic(Hardware::SubsystemStorage* subsystems,
													Hardware::EquipmentSet* equipmentSet,
													Hardware::OptoModuleStorage* optoModuleStorage,
													Hardware::ConnectionStorage* connections,
													SignalSet* signalSet,
													LmDescriptionSet* lmDescriptions,
													AppLogicData* appLogicData,
													Tuning::TuningDataStorage* tuningDataStorage,
													ComparatorStorage* comparatorStorage,
													BuildResultWriter* buildResultWriter)
	{
		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, tr("Application Logic compilation"));

		ApplicationLogicCompiler appLogicCompiler(subsystems,
												  equipmentSet,
												  optoModuleStorage,
												  connections,
												  signalSet,
												  lmDescriptions,
												  appLogicData,
												  tuningDataStorage,
												  comparatorStorage,
												  buildResultWriter,
												  m_log);

		bool result = appLogicCompiler.run();

		LOG_EMPTY_LINE(m_log);

		if (result == false)
		{
			LOG_MESSAGE(m_log, tr("Application Logic compilation was finished with errors"));
		}
		else
		{
			LOG_SUCCESS(m_log, tr("Application Logic compilation was succesfully finished"));
		}

		return result;
	}


	bool BuildWorkerThread::generateSoftwareConfiguration(DbController *db,
															Hardware::SubsystemStorage* subsystems,
															Hardware::EquipmentSet* equipment,
															SignalSet* signalSet,
															Tuning::TuningDataStorage* tuningDataStorage,
															const LmsUniqueIdMap& lmsUniqueIdMap,
															BuildResultWriter* buildResultWriter)
	{
		bool result = true;

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("OMS and Tuning software configuration generation...")))
		LOG_EMPTY_LINE(m_log);

		result &= SoftwareCfgGenerator::generalSoftwareCfgGeneration(db, signalSet, equipment, buildResultWriter);

		equipmentWalker(equipment->root(),
			[this, &db, &subsystems, &signalSet, &buildResultWriter, &equipment, &tuningDataStorage, lmsUniqueIdMap, &result](Hardware::DeviceObject* currentDevice)
			{
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
					softwareCfgGenerator = new AppDataServiceCfgGenerator(db, subsystems, software, signalSet, equipment, buildResultWriter);
					break;

				case E::SoftwareType::DiagDataService:
					softwareCfgGenerator = new DiagDataServiceCfgGenerator(db, software, signalSet, equipment, buildResultWriter);
					break;

				case E::SoftwareType::Monitor:
					softwareCfgGenerator = new MonitorCfgGenerator(db, software, signalSet, equipment, buildResultWriter);
					break;

				case E::SoftwareType::TuningService:
					softwareCfgGenerator = new TuningServiceCfgGenerator(db, subsystems, software, signalSet, equipment, tuningDataStorage, lmsUniqueIdMap, buildResultWriter);
					break;

				case E::SoftwareType::TuningClient:
					softwareCfgGenerator = new TuningClientCfgGenerator(db, subsystems, software, signalSet, equipment, buildResultWriter);
					break;

				case E::SoftwareType::ConfigurationService:
					break;

				case E::SoftwareType::ArchiveService:
					softwareCfgGenerator = new ArchivingServiceCfgGenerator(db, software, signalSet, equipment, buildResultWriter);
					break;

                case E::SoftwareType::Metrology:
                    softwareCfgGenerator = new MetrologyCfgGenerator(db, subsystems, software, signalSet, equipment, buildResultWriter);
                    break;

				default:
					m_log->errEQP6100(software->equipmentIdTemplate(), software->uuid());
					result = false;
				}

				if (softwareCfgGenerator != nullptr)
				{
					result &= softwareCfgGenerator->run();

					delete softwareCfgGenerator;
				}
			}
		);

		buildResultWriter->writeConfigurationXmlFiles();

		LOG_EMPTY_LINE(m_log);

		if (result == false)
		{
			LOG_MESSAGE(m_log, tr("Sofware configuration generation was finished with errors"));
		}
		else
		{
			LOG_SUCCESS(m_log, tr("Sofware configuration generation was succesfully finished"));
		}

		return result;
	}


	bool BuildWorkerThread::writeBinaryFiles(BuildResultWriter &buildResultWriter)
	{
		bool result = true;

		result &= buildResultWriter.writeMultichannelFiles();

		return result;
	}


	void BuildWorkerThread::generateLmsUniqueID(BuildResultWriter& buildWriter,
												TuningBuilder& tuningBuilder,
												ConfigurationBuilder& cfgBuilder,
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

			quint64 appUniqueId = buildWriter.getAppUniqueId(subsysID, lmNumber);
			quint64 tunUniqueId = tuningBuilder.getFirmwareUniqueId(subsysID, lmNumber);
			quint64 cfgUniqueId = cfgBuilder.getFirmwareUniqueId(subsysID, lmNumber);

			quint64 genericUniqueId = appUniqueId ^ tunUniqueId ^ cfgUniqueId;

			buildWriter.setGenericUniqueId(subsysID, lmNumber, genericUniqueId);
			tuningBuilder.setGenericUniqueId(subsysID, lmNumber, genericUniqueId);
			cfgBuilder.setGenericUniqueId(subsysID, lmNumber, genericUniqueId);

			lmsUniqueIdMap.insert(lm->equipmentIdTemplate(), genericUniqueId);
		}
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

	void BuildWorkerThread::setIssueLog(IssueLogger* value)
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

	bool BuildWorkerThread::isInterruptRequested()
	{
		return QThread::currentThread()->isInterruptionRequested();
	}

	//
	//
	//		LmDescriptionSet
	//
	bool LmDescriptionSet::has(QString fileName) const
	{
		return m_lmDescriptions.count(fileName) > 0;
	}

	bool LmDescriptionSet::loadFile(IssueLogger* log, DbController* db, QString objectId, QString fileName)
	{
		assert(log);
		assert(db);
		assert(objectId.isEmpty() == false);
		assert(fileName.isEmpty() == false);

		std::vector<DbFileInfo> fileList;

		bool result = db->getFileList(&fileList, db->afblFileId(), fileName, true, nullptr);
		if (result == false)
		{
			log->errPDB2001(db->afblFileId(), fileName, db->lastError());
			return false;
		}

		if (fileList.size() != 1)
		{
			log->errEQP6004(objectId, fileName, QUuid());
			return false;
		}

		// Get description file from the DB
		//
		std::shared_ptr<DbFile> file;
		result = db->getLatestVersion(fileList[0], &file, nullptr);
		if (result == false)
		{
			log->errPDB2002(fileList[0].fileId(), fileList[0].fileName(), db->lastError());
			return false;
		}

		// Parse file
		//
		QString parseErrorMessage;

		std::shared_ptr<LogicModule> lmd = std::make_shared<LogicModule>();

		result = lmd->load(file->data(), &parseErrorMessage);

		if (result == false)
		{
			QString errorMsg = QString("Cannot parse file %1. Error message: %2").arg(fileName).arg(parseErrorMessage);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined, errorMsg);
			return false;
		}

		add(fileName, lmd);

		return true;
	}

	void LmDescriptionSet::add(QString fileName, std::shared_ptr<LogicModule> lm)
	{
		assert(lm);
		m_lmDescriptions[fileName] = lm;
	}

	std::shared_ptr<LogicModule> LmDescriptionSet::get(QString fileName) const
	{
		auto it = m_lmDescriptions.find(fileName);

		if (it == std::end(m_lmDescriptions))
		{
			return std::shared_ptr<LogicModule>();
		}
		else
		{
			return it->second;
		}
	}

	std::shared_ptr<LogicModule> LmDescriptionSet::get(QString fileName)
	{
		auto it = m_lmDescriptions.find(fileName);

		if (it == std::end(m_lmDescriptions))
		{
			return std::shared_ptr<LogicModule>();
		}
		else
		{
			return it->second;
		}
	}

	std::shared_ptr<LogicModule> LmDescriptionSet::get(const Hardware::DeviceModule* logicModule) const
	{
		if (logicModule == nullptr ||
			logicModule->isLogicModule() == false)
		{
			assert(logicModule);
			assert(logicModule->isLogicModule());
			return std::shared_ptr<LogicModule>();
		}

		auto lmDescriptionFileProp = logicModule->propertyByCaption(Hardware::PropertyNames::lmDescriptionFile);
		if (lmDescriptionFileProp == nullptr)
		{
			assert(lmDescriptionFileProp);
			return std::shared_ptr<LogicModule>();
		}

		QString lmDescriptionFile = lmDescriptionFileProp->value().toString();
		if (lmDescriptionFile.isEmpty() == true)
		{
			assert(lmDescriptionFile.isEmpty() == false);
			return std::shared_ptr<LogicModule>();
		}

		return get(lmDescriptionFile);
	}

	std::shared_ptr<LogicModule> LmDescriptionSet::get(Hardware::DeviceModule* logicModule)
	{
		if (logicModule == nullptr ||
			logicModule->isLogicModule() == false)
		{
			assert(logicModule);
			assert(logicModule->isLogicModule());
			return std::shared_ptr<LogicModule>();
		}

		auto lmDescriptionFileProp = logicModule->propertyByCaption(Hardware::PropertyNames::lmDescriptionFile);
		if (lmDescriptionFileProp == nullptr)
		{
			assert(lmDescriptionFileProp);
			return std::shared_ptr<LogicModule>();
		}

		QString lmDescriptionFile = lmDescriptionFileProp->value().toString();
		if (lmDescriptionFile.isEmpty() == true)
		{
			assert(lmDescriptionFile.isEmpty() == false);
			return std::shared_ptr<LogicModule>();
		}

		return get(lmDescriptionFile);
	}

	QString LmDescriptionSet::lmDescriptionFile(const Hardware::DeviceModule* logicModule)
	{
		assert(logicModule);
		assert(logicModule->isLogicModule());
		return LogicModule::lmDescriptionFile(logicModule);
	}

	// ------------------------------------------------------------------------
	//
	//		Builder
	//
	// ------------------------------------------------------------------------


	Builder::Builder(IssueLogger* log) :
		m_log(log)
	{
		assert(m_log != nullptr);

		m_thread = new BuildWorkerThread();
		m_thread->setObjectName(tr("BuildWorkerThread"));
		m_thread->setIssueLog(m_log);

		connect(m_thread, &BuildWorkerThread::resultReady, this, &Builder::handleResults);

		connect(m_thread, &BuildWorkerThread::started, GlobalMessanger::instance(), &GlobalMessanger::buildStarted);
		connect(m_thread, &BuildWorkerThread::finished, GlobalMessanger::instance(), &GlobalMessanger::buildFinished);

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

