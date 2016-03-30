#include "Builder.h"
#include "Parser.h"
#include "TuningBuilder.h"
#include "ConfigurationBuilder.h"
#include "Subsystem.h"
#include "GlobalMessanger.h"

#include "../../include/DbController.h"
#include "../../include/DeviceObject.h"

#include "../../VFrame30/LogicSchema.h"
#include "../../VFrame30/SchemaItemLink.h"
#include "../../VFrame30/HorzVertLinks.h"

#include "../Builder/ApplicationLogicCompiler.h"
#include "../Builder/SoftwareCfgGenerator.h"
#include <QBuffer>
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
				QThread::currentThread()->requestInterruption();
				break;
			}

			if (release() == true && isAnyCheckedOut == true)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  tr("There are some checked out objects. Please check in all objects before building release version."));
				QThread::currentThread()->requestInterruption();
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
				//LOG_ERROR(m_log, Builder::IssueType::NotDefined, tr("Error"));
				QThread::currentThread()->requestInterruption();
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

			//
			// Check same Uuids and same StrIds
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Checking for same Uuids and StrIds"));

			ok = checkSameUuidAndStrId(deviceRoot.get());

			if (ok == false)
			{
				break;
			}

			LOG_SUCCESS(m_log, tr("Ok"));

			//
			// SignalSet
			//

			//auto aaa = equipmentSet.deviceObject(QString("SYSTEMID1_RACKID2_SIGNAL1"));
			//auto aaa1 = equipmentSet.deviceObjectSharedPointer("SYSTEMID1_RACKID2_SIGNAL1");

			SignalSet signalSet;

			if (loadSignals(&db, &signalSet) == false)
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
				//LOG_ERROR(m_log, Builder::IssueType::NotDefined, tr("Error"));
				QThread::currentThread()->requestInterruption();
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

            Hardware::OptoModuleStorage opticModuleStorage(&equipmentSet, m_log);

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

			//
			// Parse application logic
			//
			AppLogicData appLogicData;

			parseApplicationLogic(&db, &appLogicData, &afbCollection, &equipmentSet, lastChangesetId);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Compile application logic
			//
            compileApplicationLogic(&subsystems, &equipmentSet, &opticModuleStorage, &connections, &signalSet, &afbCollection, &appLogicData, &buildWriter);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Generate SCADA software configurations
			//
			generateSoftwareConfiguration(&db, &equipmentSet, &signalSet, &buildWriter);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}


            //
            // Compile Module configuration
            //
            LOG_EMPTY_LINE(m_log);
            LOG_MESSAGE(m_log, tr("Module configurations compilation"));

            ok = modulesConfiguration(&db, dynamic_cast<Hardware::DeviceRoot*>(deviceRoot.get()), &signalSet, &subsystems, &opticModuleStorage, lastChangesetId, &buildWriter);

            if (QThread::currentThread()->isInterruptionRequested() == true)
            {
                break;
            }

			if (ok == false)
			{
//				LOG_ERROR(m_log, Builder::IssueType::NotDefined, tr("Error"));
				QThread::currentThread()->requestInterruption();
				break;
			}


            //
            // Tuning parameters
            //
            LOG_EMPTY_LINE(m_log);
            LOG_MESSAGE(m_log, tr("Tuning parameters compilation"));

            ok = tuningParameters(&db, dynamic_cast<Hardware::DeviceRoot*>(deviceRoot.get()), &signalSet, &subsystems, &opticModuleStorage, lastChangesetId, &buildWriter);

            if (QThread::currentThread()->isInterruptionRequested() == true)
            {
                break;
            }

			if (ok == false)
			{
//                LOG_ERROR(m_log, Builder::IssueType::NotDefined, tr("Error"));
				QThread::currentThread()->requestInterruption();
				break;
			}

            LOG_SUCCESS(m_log, tr("Ok"));
        }
		while (false);

		buildWriter.finish();

		db.closeProject(nullptr);

		// Set Shceme Items Issues to GlobalMessanger
		//
		std::map<QUuid, OutputMessageLevel> schemaItemsIssues;
		m_log->swapItemsIssues(&schemaItemsIssues);

		GlobalMessanger::instance()->swapSchemaIssues(schemaItemsIssues);

		// We've done, exit
		//
		emit resultReady(QString("Cool, we've done!"));

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

		if (parent->deviceType() == Hardware::DeviceType::System)
		{
			LOG_MESSAGE(m_log, tr("Getting system %1...").arg(parent->caption()));
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

			if (file == nullptr || ok == false)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Cannot get %1 instance.").arg(fi.fileName()));
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

	bool BuildWorkerThread::expandDeviceStrId(Hardware::DeviceObject* device)
	{
		if (device == nullptr)
		{
			assert(device != nullptr);
			return false;
		}

		device->expandStrId();

		return true;
	}

	bool BuildWorkerThread::checkSameUuidAndStrId(Hardware::DeviceObject* root)
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

		bool ok = checkSameUuidAndStrIdWorker(root, uuidMap, strIdMap);

		return ok;
	}

	bool BuildWorkerThread::checkSameUuidAndStrIdWorker(Hardware::DeviceObject* device,
									 std::map<QUuid, Hardware::DeviceObject*>& uuidMap,
									 std::map<QString, Hardware::DeviceObject*>& strIdMap)
	{
		if (device == nullptr)
		{
			assert(device);
			return false;
		}

		auto foundSameUuid = uuidMap.find(device->uuid());
		auto foundSameStrId = strIdMap.find(device->strId());

		bool ok = true;

		if (foundSameUuid != uuidMap.end())
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  tr("There are DeviceObjects with the same Uuid %1, StrID1: %2, StrID2: %3")
						.arg(device->uuid().toString())
						.arg(device->strId())
						.arg(foundSameUuid->second->strId()));

			ok = false;
		}
		else
		{
			uuidMap[device->uuid()] = device;
		}

		if (foundSameStrId != strIdMap.end())
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
					  tr("There are DeviceObjects with the same StrID %1, Parent1: %2, Parent2: %3")
						  .arg(device->strId())
						  .arg(device->parent()->strId())
						  .arg(foundSameStrId->second->parent()->strId()));

			ok = false;
		}
		else
		{
			strIdMap[device->strId()] = device;
		}

		int childCount = device->childrenCount();

		for (int i = 0; i < childCount; i++)
		{
			ok &= checkSameUuidAndStrIdWorker(device->child(i), uuidMap, strIdMap);
		}

		return ok;
	}


	bool BuildWorkerThread::loadSignals(DbController* db, SignalSet* signalSet)
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

		if (result)
		{
			result = result && db->getUnits(Signal::m_unitList.get(), nullptr);
		}

		if (result == false)
		{
			//LOG_ERROR(m_log, tr("Error"));
			return false;
		}

		LOG_SUCCESS(m_log, tr("Ok"));

		return true;
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

    bool BuildWorkerThread::modulesConfiguration(DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage *subsystems, Hardware::OptoModuleStorage *opticModuleStorage, int changesetId, BuildResultWriter* buildWriter)
	{
		if (db == nullptr ||
			deviceRoot == nullptr ||
			signalSet == nullptr ||
			subsystems == nullptr ||
            opticModuleStorage == nullptr ||
			buildWriter == nullptr)
		{
			assert(false);
			return false;
		}

        ConfigurationBuilder cfgBuilder = {db, deviceRoot, signalSet, subsystems, opticModuleStorage, m_log, changesetId, debug(), projectName(), projectUserName(), buildWriter};

		bool result = cfgBuilder.build();

		return result;

	}

    bool BuildWorkerThread::tuningParameters(DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage *subsystems, Hardware::OptoModuleStorage *opticModuleStorage, int changesetId, BuildResultWriter* buildWriter)
    {
        if (db == nullptr ||
            deviceRoot == nullptr ||
            signalSet == nullptr ||
            subsystems == nullptr ||
            opticModuleStorage == nullptr ||
            buildWriter == nullptr)
        {
            assert(false);
            return false;
        }

        TuningBuilder tunBuilder = {db, deviceRoot, signalSet, subsystems, opticModuleStorage, m_log, changesetId, debug(), projectName(), projectUserName(), buildWriter};

        bool result = tunBuilder.build();

        return result;

    }

    bool BuildWorkerThread::parseApplicationLogic(DbController* db,
												  AppLogicData* appLogicData,
												  Afb::AfbElementCollection* afbCollection,
												  Hardware::EquipmentSet* equipment,
												  int changesetId)
	{
		if (db == nullptr ||
			appLogicData == nullptr ||
			afbCollection == nullptr)
		{
			assert(db);
			assert(appLogicData);
			assert(afbCollection);
			return false;
		}

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, tr("Application Logic parsing"));

		Parser alPareser = {db, m_log, appLogicData, afbCollection, equipment, changesetId, debug()};

		bool result = alPareser.build();

		if (result == false)
		{
			//LOG_ERROR(m_log, tr("Error"));	// Error must be logged and described where it was found
			QThread::currentThread()->requestInterruption();
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
                                                    BuildResultWriter* buildResultWriter)
	{
		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, tr("Application Logic compilation"));

        ApplicationLogicCompiler appLogicCompiler(subsystems, equipmentSet, optoModuleStorage, connections, signalSet, afbCollection, appLogicData, buildResultWriter, m_log);

		bool result = appLogicCompiler.run();

		LOG_EMPTY_LINE(m_log);

		if (result == false)
		{
			LOG_MESSAGE(m_log, tr("Application Logic compilation was finished with errors"));
			QThread::currentThread()->requestInterruption();
		}
		else
		{
			LOG_SUCCESS(m_log, tr("Application Logic compilation was succesfully finished"));
		}

		return result;
	}


	bool BuildWorkerThread::generateSoftwareConfiguration(DbController *db, Hardware::EquipmentSet* equipment, SignalSet* signalSet, BuildResultWriter* buildResultWriter)
	{
		bool result = true;

		LOG_EMPTY_LINE(m_log);

		LOG_MESSAGE(m_log, QString(tr("SCADA sofware configuration generation...")))

		equipmentWalker(equipment->root(),
			[&db, &signalSet, &buildResultWriter, equipment, &result](Hardware::DeviceObject* currentDevice)
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

				SoftwareCfgGenerator softwareCfgGenerator(db, software, signalSet, equipment, buildResultWriter);

				result &= softwareCfgGenerator.run();
			}
		);

		buildResultWriter->writeConfigurationXmlFiles();

		LOG_EMPTY_LINE(m_log);

		if (result == false)
		{
			LOG_MESSAGE(m_log, tr("Sofware configuration generation was finished with errors"));
			QThread::currentThread()->requestInterruption();
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
