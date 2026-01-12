#include "TaskGetEquipment.h"

#include <HardwareLib/DeviceModule.h>
#include <HardwareLib/PropertyNames.h>
#include <HardwareLib/ScriptDeviceObject.h>
#include <HardwareLib/ScriptEquipment.h>

#include "Context.h"
#include "version.h"

namespace Builder
{

	TaskGetEquipment::TaskGetEquipment(Builder::Context& context) :
		m_context{context},
		m_log{*context.m_log}
	{
	}

	bool TaskGetEquipment::doIt()
	{
		std::shared_ptr<Hardware::DeviceRoot> deviceRoot = std::make_shared<Hardware::DeviceRoot>();

		int rootFileId = m_context.m_db.systemFileId(DbDir::HardwareConfigurationDir);
		auto fio = std::make_shared<DbFileInfo>(rootFileId);

		deviceRoot->setData(fio);

		if (bool ok = getEquipment(deviceRoot.get()); ok == false)
		{
			return false;
		}

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			return true;
		}

		//
		// Remove excluded devices, DeviceObject::isExcludedBromBuild()
		//
		LOG_MESSAGE_REF(m_log, tr("Removing excluded devices"));

		if (bool ok = removeExcludedDevices(deviceRoot.get()); ok == false)
		{
			return false;
		}
		else
		{
			LOG_MESSAGE_REF(m_log, tr("Ok"));
		}

		//
		// Expand Devices StrId
		//
		LOG_MESSAGE_REF(m_log, tr("Expanding devices StrIds"));

		if (bool ok = expandDeviceStrId(deviceRoot.get()); ok == false)
		{
			return false;
		}

		LOG_MESSAGE_REF(m_log, tr("Ok"));

		m_context.m_equipmentSet = std::make_shared<Hardware::EquipmentSet>(deviceRoot);

		deviceRoot.reset(); // Use equipmentSet.root() instead

		//
		// Checking for identical UUIDs and StrIDs
		//
		LOG_MESSAGE_REF(m_log, tr("Checking for identical UUIDs and StrIDs"));

		if (bool ok = checkUuidAndStrId(m_context.m_equipmentSet->root().get()); ok == false)
		{
			return false;
		}

		//
		// Execute pre-build scripts
		//
		if (bool ok = execPreBuildScript(m_context.m_equipmentSet->root()); ok == false)
		{
			return false;
		}

		//
		// Check child restrictions
		//
		if (bool ok = checkChildRestrictions(m_context.m_equipmentSet->root()); ok == false)
		{
			return false;
		}

		// Creating equipment file for Monitor: Filter out all AppSignal and DiagSignals objects (leave Root, System, Rack, Chassis,
		// Modules, Controllers, Workstation, Software.
		//
		{
			LOG_MESSAGE_REF(m_log, tr("Creating Monitor equipment file"));

			Proto::Envelope message;
			auto predicate = [](const Hardware::DeviceObject& device) -> bool
			{
				if (device.isAppSignal() == true || device.isDiagSignal() == true)
				{
					return false;
				}

				return true;
			};

			// Save data to proto-structure.
			//
			bool saveOk = m_context.m_equipmentSet->root()->SaveObjectTreeIf(&message, predicate);
			if (saveOk == false)
			{
				m_log.errINT1000("Failed to serialize Monitor Equipment data to proto-structure.");
				return false;
			}

			// Save proto-structure to byte array.
			//
			QByteArray data;
			saveOk = Hardware::DeviceObject::saveToByteArray(&data, message, Proto::ProtoCompress::Always);

			if (saveOk == false)
			{
				m_log.errINT1000("Failed to serialize Monitor Equipment data from proto-structure to byte array.");
				return false;
			}

			// Save byte array to a build file.
			//
			auto filePtr =
				m_context.m_buildResultWriter->addFile(Directory::COMMON, File::MONITOR_EQUIPMENT, CfgFileId::MONITOR_EQUIPMENT, "", data);
			if (filePtr == nullptr)
			{
				// addFile has already logged the error.
				//
				return false;
			}
		}

		// Done
		//
		LOG_MESSAGE_REF(m_log, tr("Ok"));

		return true;
	}

	bool TaskGetEquipment::getEquipment(Hardware::DeviceObject* parent)
	{
		assert(m_context.m_db.isProjectOpened() == true);
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

		if (const DbFileInfo* parentFileInfo = parent->data(); parentFileInfo == nullptr)
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
		ok = m_context.m_db.getFileList(&files, parentFileId, true, nullptr);

		if (ok == false)
		{
			LOG_ERROR_OBSOLETE_REF(m_log, Builder::IssueType::NotDefined, tr("Cannot get equipment file list"));
			return false;
		}

		parent->deleteAllChildren();

		for (DbFileInfo& fi : files)
		{
			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			if (fi.action() == E::VcsItemAction::Deleted) // File is deleted
			{
				continue;
			}

			LOG_MESSAGE_REF(m_log, tr("Getting equipment object, file id: %1, details: %2").arg(fi.fileId()).arg(fi.details()));

			std::shared_ptr<Hardware::DeviceObject> device;
			ok = m_context.m_db.getDeviceTreeLatestVersion(fi, &device, nullptr);

			if (ok == false || device.get() == nullptr)
			{
				LOG_ERROR_OBSOLETE_REF(m_log, "", tr("Failed to load equipment, file id: %1").arg(fi.fileId()));
				continue;
			}

			parent->addChild(device);
		}

		LOG_MESSAGE_REF(m_log, tr("Ok"));

		return true;
	}

	bool TaskGetEquipment::removeExcludedDevices(Hardware::DeviceObject* parent)
	{
		if (parent == nullptr)
		{
			assert(parent != nullptr);
			return false;
		}

		// Remove excluded devices
		//
		std::list<std::shared_ptr<Hardware::DeviceObject>> toDelete;
		for (auto child : parent->children())
		{
			if (child->excludeFromBuild() == true)
			{
				toDelete.push_back(child);
			}
		}

		// Sort is for better log readability.
		//
		toDelete.sort(
			[](const auto& lhs, const auto& rhs)
			{
				return lhs->equipmentId() < rhs->equipmentId();
			});

		for (auto child : toDelete)
		{
			m_log.wrnCFG3102(child->equipmentId()); // Device '%1' is excluded from build.
			parent->deleteChild(child);
		}

		// Recursively check children.
		//
		for (auto child : parent->children())
		{
			removeExcludedDevices(child.get());
		}

		return true;
	}

	bool TaskGetEquipment::expandDeviceStrId(Hardware::DeviceObject* device)
	{
		if (device == nullptr)
		{
			assert(device != nullptr);
			return false;
		}

		device->expandEquipmentId();

		return true;
	}

	bool TaskGetEquipment::checkUuidAndStrId(Hardware::DeviceObject* root)
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


	bool TaskGetEquipment::checkUuidAndStrIdWorker(Hardware::DeviceObject* device,
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
			m_log.errEQP6002(device->uuid(), device->equipmentId(), foundSameUuid->second->equipmentId());
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
			m_log.errEQP6001(device->equipmentId(), device->uuid(), foundSameStrId->second->uuid());
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
				m_log.errEQP6009(module->equipmentIdTemplate(), module->uuid());
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
			m_log.errEQP6000(device->equipmentIdTemplate(), device->uuid());
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

	bool TaskGetEquipment::execPreBuildScript(std::shared_ptr<Hardware::DeviceObject> root)
	{
		if (root == nullptr)
		{
			assert(root);
			return false;
		}

		QJSEngine engine;

		// Setting global variables.
		//
		engine.globalObject().setProperty("version",
										  QString("%1.%2.%3").arg(U7SET_MAJOR_VERSION).arg(U7SET_MINOR_VERSION).arg(U7SET_PATCH_VERSION));
		engine.globalObject().setProperty("versionMajor", U7SET_MAJOR_VERSION);
		engine.globalObject().setProperty("versionMinor", U7SET_MINOR_VERSION);
		engine.globalObject().setProperty("versionPatch", U7SET_PATCH_VERSION);

		Hardware::ScriptEquipment equipment{engine, nullptr};
		equipment.setRoot(root);

		QJSValue jsEquipment = engine.newQObject(&equipment);
		QJSEngine::setObjectOwnership(&equipment, QJSEngine::CppOwnership);

		bool result = true;

		auto func = [this, &result, &engine, &jsEquipment](Hardware::DeviceObject* device)
		{
			result &= execPreBuildScriptWorker(device, engine, jsEquipment);
		};

		::Hardware::equipmentWalker(root.get(), func);

		// Call recursive function
		//
		return result;
	}

	bool TaskGetEquipment::execPreBuildScriptWorker(Hardware::DeviceObject* device, QJSEngine& engine, QJSValue& jsEquipment)
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

		QString script = device->preBuildScript().trimmed();

		// Clear script, we don't need it anymore, do not store it in the equipment object
		//
		device->setPreBuildScript({});

		if (script.isEmpty() == true)
		{
			return true;
		}

		QJSValue function = engine.evaluate(script);
		if (function.isError())
		{
			qDebug() << "TaskGetEquipment::execPreBuildScriptWorker, device: " << device->equipmentId();
			qDebug() << "Script evaluate error at line " << function.property("lineNumber").toInt();
			qDebug() << "\tClass: " << metaObject()->className();
			qDebug() << "\tStack: " << function.property("stack").toString();
			qDebug() << "\tMessage: " << function.toString();

			m_log.errEQP6301(device->equipmentId(),
							 Hardware::PropertyNames::preBuildScript,
							 function.property("lineNumber").toInt(),
							 function.toString());

			return false;
		}

		QJSValue jsDevice = engine.newQObject(new ::Hardware::ScriptDeviceObject(device->shared_from_this()));

		QJSValueList arguments;
		arguments << jsDevice;
		arguments << jsEquipment;

		// Watch-dog thread
		//
		int timeoutSeconds = 5;
		std::mutex mutexExec;
		std::condition_variable cvExec;
		bool finished = false;

		// Async function returns true if timeout has occurred
		//
		auto watchDogFunc = [&engine, &mutexExec, &cvExec, &finished, timeoutSeconds]() -> bool
		{
			std::unique_lock lock{mutexExec};
			if (cvExec.wait_for(lock,
								std::chrono::seconds(timeoutSeconds),
								[&finished]
								{
									return finished;
								}) == true)
			{
				// Task has finished without timeout
				//
				return false;
			}

			// Task has not finished within the timeout
			//
			engine.setInterrupted(true);
			return true;
		};

		auto watchDogFuture = std::async(std::launch::async, watchDogFunc);

		QJSValue result = function.call(arguments);

		// Wait for watchdog to finish
		//
		{
			std::lock_guard lock{mutexExec};
			finished = true;
			cvExec.notify_one();             // Notify that watchdog has finished execution
		}

		bool timeout = watchDogFuture.get(); // returns true if timeout has occurred
		if (timeout == true)
		{
			m_log.errEQP6310(device->equipmentId(),
							 Hardware::PropertyNames::preBuildScript,
							 -1,
							 QString{"Timeout %1 seconds."}.arg(timeoutSeconds));
			return false;
		}

		if (result.isError() == true)
		{
			m_log.errEQP6310(device->equipmentId(),
							 Hardware::PropertyNames::preBuildScript,
							 result.property("lineNumber").toInt(),
							 result.toString());
			return false;
		}

		return true;
	}

	bool TaskGetEquipment::checkChildRestrictions(std::shared_ptr<Hardware::DeviceObject> root)
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

	bool TaskGetEquipment::checkChildRestrictionsWorker(std::shared_ptr<Hardware::DeviceObject> device)
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
					m_log.errINT1001(errorMessage);
				}

				m_log.errEQP6008(device->equipmentId(), child->equipmentId(), child->place());
				return false;
			}

			if (checkChildRestrictionsWorker(child) == false)
			{
				return false;
			}
		}

		return true;
	}

} // namespace Builder