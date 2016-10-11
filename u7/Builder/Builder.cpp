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
#include "IssueLogger.h"

#include <QBuffer>
#include <QtConcurrent/QtConcurrent>
#include <functional>



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

			bool isAnyCheckedOut = false;
			ok = db.isAnyCheckedOut(&isAnyCheckedOut);

			if (ok == false)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("isAnyCheckedOut Error."));
				break;
			}

			if (release() == true && isAnyCheckedOut == true)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  tr("There are some checked out objects. Please check in all objects before building release version."));
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
			// Loading AFB elements
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Loading AFB elements"));

			Afb::AfbElementCollection afbCollection;

			ok = loadAfbl(&db, &afbCollection);

			if (ok == false)
			{
				break;
			}
			else
			{
				LOG_SUCCESS(m_log, tr("Ok"));
			}

			LOG_MESSAGE(m_log, tr("%1 elements loaded.").arg(afbCollection.elements().size()));

			//
			// Loading subsystems
			//

			Hardware::SubsystemStorage subsystems;

			QString errorCode;
			ok = subsystems.load(&db, errorCode);

			if (ok == false)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Can't load subsystems file"));
				if (errorCode.isEmpty() == false)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, errorCode);
					break;
				}
			}

			//
			// Loading connections
			//

			Hardware::ConnectionStorage connections;

			ok = connections.load(&db, errorCode);

			if (ok == false)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Can't load connections file"));
				if (errorCode.isEmpty() == false)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, errorCode);
					break;
				}
			}

			Hardware::OptoModuleStorage opticModuleStorage(&equipmentSet, m_log);

			//
			// Parse application logic
			//
			AppLogicData appLogicData;

			ok = parseApplicationLogic(&db, &appLogicData, &afbCollection, &equipmentSet, &signalSet, lastChangesetId);

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			Tuning::TuningDataStorage tuningDataStorage;

			//
			// Compile application logic
			//
			ok = compileApplicationLogic(&subsystems, &equipmentSet, &opticModuleStorage, &connections, &signalSet, &afbCollection, &appLogicData, &tuningDataStorage, &buildWriter);

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Generate SCADA software configurations
			//
			ok = generateSoftwareConfiguration(&db, &subsystems, &equipmentSet, &signalSet, &tuningDataStorage, &buildWriter);

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

			ok = modulesConfiguration(this, &db, equipmentSet.root(), &signalSet, &subsystems, &opticModuleStorage, lastChangesetId, &buildWriter);

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

			ok = tuningParameters(&db, equipmentSet.root(), &signalSet, &subsystems, &tuningDataStorage, lastChangesetId, &buildWriter);

			if (ok == false ||
				QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			LOG_SUCCESS(m_log, tr("Ok"));
		}
		while (false);

		buildWriter.finish();

		db.closeProject(nullptr);

		// Set Shceme Items Issues to GlobalMessanger
		//
		BuildIssues m_buildIssues;
		m_log->swapItemsIssues(&m_buildIssues);

		GlobalMessanger::instance()->swapSchemaIssues(&m_buildIssues);

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			m_log->clear();		// Log can contain thouthands of messages, if it some kind iof "same ids" error
			LOG_ERROR_OBSOLETE(m_log, "", tr("The build was canceled."));
		}

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

		if (release() == true)
		{
			// filter some files, which are not checkedin?
			assert(false);
		}
		else
		{
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

			if (release() == true)
			{
				assert(false);
			}
			else
			{
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
		}

		return true;
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

					if (module->isLM())
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


	bool BuildWorkerThread::loadAfbl(DbController* db, Afb::AfbElementCollection* afbCollection)
	{
		if (db == nullptr ||
			afbCollection == nullptr)
		{
			assert(db);
			assert(afbCollection);
			return false;
		}

		bool result = true;

		// Get file list from the DB
		//
		std::vector<DbFileInfo> files;

		if (db->getFileList(&files, db->afblFileId(), "afb", true, nullptr) == false)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QObject::tr("Cannot get application functional block file list."));
			return false;
		}

		// Get files from the DB
		//
		std::vector<std::shared_ptr<Afb::AfbElement>> afbs;
		afbs.reserve(files.size());

		for (DbFileInfo& fi : files)
		{
			if (fi.action() == VcsItemAction::Deleted)		// File is deleted
			{
				qDebug() << "Skip file " << fi.fileId() << ", " << fi.fileName() << ", it was marked as deleted";
				continue;
			}

			std::shared_ptr<DbFile> f;

			if (db->getLatestVersion(fi, &f, nullptr) == false)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  QObject::tr("Getting the latest version of the file %1 failed.").arg(fi.fileName()));
				result = false;
				continue;
			}

			std::shared_ptr<Afb::AfbElement> e = std::make_shared<Afb::AfbElement>();

			QXmlStreamReader reader(f->data());

			if (e->loadFromXml(&reader) == false)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  QObject::tr("Reading contents of the file %1 failed.").arg(fi.fileName()));

				if (reader.errorString().isEmpty() == false)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, "XML error: " + reader.errorString());
				}

				result = false;
				continue;
			}

			afbs.push_back(e);
		}

		afbCollection->setElements(afbs);

		return result;
	}

	bool BuildWorkerThread::modulesConfiguration(BuildWorkerThread* buildWorkerThread, DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage *subsystems, Hardware::OptoModuleStorage *opticModuleStorage, int changesetId, BuildResultWriter* buildWriter)
	{
		if (buildWorkerThread == nullptr ||
			db == nullptr ||
			deviceRoot == nullptr ||
			signalSet == nullptr ||
			subsystems == nullptr ||
			opticModuleStorage == nullptr ||
			buildWriter == nullptr)
		{
			assert(false);
			return false;
		}

		ConfigurationBuilder cfgBuilder = {buildWorkerThread, db, deviceRoot, signalSet, subsystems, opticModuleStorage, m_log,
										   buildWriter->buildInfo().id, changesetId, debug(), projectName(), projectUserName(), buildWriter};

		bool result = cfgBuilder.build();

		return result;

	}

	bool BuildWorkerThread::tuningParameters(DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage *subsystems, Tuning::TuningDataStorage *tuningDataStorage, int changesetId, BuildResultWriter* buildWriter)
	{
		if (db == nullptr ||
			deviceRoot == nullptr ||
			signalSet == nullptr ||
			subsystems == nullptr ||
			tuningDataStorage == nullptr ||
			buildWriter == nullptr)
		{
			assert(false);
			return false;
		}

		TuningBuilder tunBuilder = {db, deviceRoot, signalSet, subsystems, tuningDataStorage, m_log,
									buildWriter->buildInfo().id, changesetId, debug(), projectName(), projectUserName(), buildWriter};

		bool result = tunBuilder.build();

		return result;

	}

	bool BuildWorkerThread::parseApplicationLogic(DbController* db,
												  AppLogicData* appLogicData,
												  Afb::AfbElementCollection* afbCollection,
												  Hardware::EquipmentSet* equipment,
												  SignalSet* signalSet,
												  int changesetId)
	{
		if (db == nullptr ||
			appLogicData == nullptr ||
			afbCollection == nullptr ||
			equipment == nullptr ||
			signalSet == nullptr)
		{
			assert(db);
			assert(appLogicData);
			assert(afbCollection);
			assert(equipment);
			assert(signalSet);
			return false;
		}

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, tr("Application Logic parsing..."));

		Parser alPareser = {db, m_log, appLogicData, afbCollection, equipment, signalSet, changesetId, debug()};

		bool result = alPareser.build();

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
													Afb::AfbElementCollection* afbCollection,
													AppLogicData* appLogicData,
													Tuning::TuningDataStorage* tuningDataStorage,
													BuildResultWriter* buildResultWriter)
	{
		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, tr("Application Logic compilation"));

		ApplicationLogicCompiler appLogicCompiler(subsystems, equipmentSet, optoModuleStorage, connections, signalSet, afbCollection, appLogicData, tuningDataStorage, buildResultWriter, m_log);

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
															BuildResultWriter* buildResultWriter)
	{
		bool result = true;

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("OMS and Tuning software configuration generation...")))
		LOG_EMPTY_LINE(m_log);

		result &= SoftwareCfgGenerator::generalSoftwareCfgGeneration(db, signalSet, equipment, buildResultWriter);

		equipmentWalker(equipment->root(),
			[this, &db, &subsystems, &signalSet, &buildResultWriter, &equipment, &tuningDataStorage, &result](Hardware::DeviceObject* currentDevice)
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
					softwareCfgGenerator = new TuningServiceCfgGenerator(db, subsystems, software, signalSet, equipment, tuningDataStorage, buildResultWriter);
					break;

				case E::SoftwareType::TuningClient:
					softwareCfgGenerator = new TuningClientCfgGenerator(db, subsystems, software, signalSet, equipment, buildResultWriter);
					break;

				case E::SoftwareType::ConfigurationService:
					break;

				case E::SoftwareType::ArchiveService:
					softwareCfgGenerator = new ArchivingServiceCfgGenerator(db, software, signalSet, equipment, buildResultWriter);
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

	bool BuildWorkerThread::jsIsInterruptRequested()
	{
		return QThread::currentThread()->isInterruptionRequested();
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
