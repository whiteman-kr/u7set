#include "EquipmentView.h"

#include <HardwareLib/DeviceSystem.h>
#include <HardwareLib/DeviceRack.h>
#include <HardwareLib/DeviceChassis.h>
#include <HardwareLib/DeviceModule.h>
#include <HardwareLib/DeviceController.h>
#include <HardwareLib/DiagSignal.h>
#include <HardwareLib/DeviceAppSignal.h>
#include <HardwareLib/Workstation.h>
#include <HardwareLib/Software.h>
#include <HardwareLib/PropertyNames.h>
#include <HardwareLib/HardwareLibrary.h>

#include <DbLib/DbControllerTools.h>

#include "../../Builder/SubsystemStorage.h"
#include "../DialogConnections.h"
#include "../Forms/CompareDialog.h"
#include "../Forms/DialogUpdateFromPreset.h"
#include "../Forms/FileHistoryDialog.h"
#include "../GlobalMessanger.h"
#include "../SchemaEditor/CreateSignalDialog.h"
#include "../SignalsTabPage.h"

#include "DialogChoosePreset.h"
#include "EquipmentModel.h"


//
//
// EquipmentView
//
//
const char* EquipmentView::mimeType = "application/x-deviceobjecs";
const char* EquipmentView::mimeTypeShortDescription = "application/x-deviceobjecs-sd";

EquipmentView::EquipmentView(DbController* dbcontroller) :
	m_dbController(dbcontroller)
{
	Q_ASSERT(m_dbController);

	setSortingEnabled(true);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setUniformRowHeights(true);
	setIndentation(10);

	sortByColumn(EquipmentModel::Columns::ObjectPlaceColumn, Qt::SortOrder::AscendingOrder);

	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &EquipmentView::projectOpened, Qt::QueuedConnection);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &EquipmentView::projectClosed, Qt::QueuedConnection);

	return;
}

EquipmentView::~EquipmentView()
{
}

bool EquipmentView::isPresetMode() const
{
	return equipmentModel()->isPresetMode();
}

bool EquipmentView::isConfigurationMode() const
{
	return equipmentModel()->isConfigurationMode();
}

DbController* EquipmentView::db()
{
	return m_dbController;
}

const DbController* EquipmentView::db() const
{
	return m_dbController;
}

void EquipmentView::saveSession() const
{
	// Save all opened schamas
	//
	if (db()->isProjectOpened() == false)
	{
		Q_ASSERT(db()->isProjectOpened());
		return;
	}

	// Save new session data, data saved to settings, for each project separately
	//
	QString equipmentId;
	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.empty() == false)
	{
		auto firstDevice = equipmentModel()->deviceObject(selected.first());
		Q_ASSERT(firstDevice);

		equipmentId = firstDevice->equipmentId();
	}

	// --
	//
	QSettings settings;
	QString keyDir = QString("Session/%1-%2/EquipmentEditor/")
					 .arg(db()->currentProject().projectName())
					 .arg(db()->currentUser().username());

	if (equipmentId.isEmpty() == false)
	{
		settings.setValue(keyDir + "Selected", equipmentId);
	}

	return;
}

void EquipmentView::restoreSession()
{
	m_requireRestoreSession = false;

	// Restore session
	//
	if (db()->isProjectOpened() == false)
	{
		Q_ASSERT(db()->isProjectOpened());
		return;
	}

	QSettings settings;
	QString keyDir = QString("Session/%1-%2/EquipmentEditor/")
					 .arg(db()->currentProject().projectName())
					 .arg(db()->currentUser().username());

	QString equipmentId = settings.value(keyDir + "Selected").toString();

	if (equipmentId.isEmpty() == false)
	{
		findObject(equipmentId);
	}

	return;
}


void EquipmentView::projectOpened()
{
	equipmentModel()->projectOpenedAction();

	setExpanded(QModelIndex{}, true);

	m_requireRestoreSession = true;

	return;
}

void EquipmentView::projectClosed()
{
	equipmentModel()->projectClosedAction();

	m_requireRestoreSession = false;

	return;
}

void EquipmentView::addSystem()
{
	QModelIndex parentModelIndex;		// current is root

	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.size() > 1)
	{
		// Don't know after which item insrt new object
		//
		return;
	}

	if (selected.empty() == false)
	{
		parentModelIndex = selected[0];
	}

	// Add new system to the root
	//
	std::shared_ptr<Hardware::DeviceObject> system = std::make_shared<Hardware::DeviceSystem>(isPresetMode());

	system->setEquipmentIdTemplate("SYSTEMID");
	system->setCaption(tr("System"));
	system->setPlace(0);

	addDeviceObject(system, parentModelIndex, true, true);

	emit updateState();
	return;
}

void EquipmentView::addRack()
{
	QModelIndex parentModelIndex;		// current is root

	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.size() > 1)
	{
		// Don't know after which item insrt new object
		//
		return;
	}

	if (selected.empty() == false)
	{
		parentModelIndex = selected[0];
	}

	// --
	//
	std::shared_ptr<Hardware::DeviceObject> rack = std::make_shared<Hardware::DeviceRack>(isPresetMode());

	rack->setEquipmentIdTemplate("$(PARENT)_RACKID");
	rack->setCaption(tr("Rack"));
	rack->setPlace(0);

	addDeviceObject(rack, parentModelIndex, true, true);

	emit updateState();
	return;
}

void EquipmentView::addChassis()
{
	QModelIndex parentModelIndex;		// current is root

	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.size() > 1)
	{
		// Don't know after which item insrt new object
		//
		return;
	}

	if (selected.empty() == false)
	{
		parentModelIndex = selected[0];
	}

	// --
	//
	std::shared_ptr<Hardware::DeviceObject> chassis = std::make_shared<Hardware::DeviceChassis>(isPresetMode());

	chassis->setEquipmentIdTemplate("$(PARENT)_CH$(PLACE)");
	chassis->setCaption(tr("Chassis"));
	chassis->setPlace(-1);

	addDeviceObject(chassis, parentModelIndex, true, true);

	emit updateState();
	return;
}

void EquipmentView::addModule()
{
	QModelIndex parentModelIndex;		// current is root

	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.size() > 1)
	{
		// Don't know after which item insrt new object
		//
		return;
	}

	if (selected.empty() == false)
	{
		parentModelIndex = selected[0];
	}

	// --
	//
	std::shared_ptr<Hardware::DeviceObject> module = std::make_shared<Hardware::DeviceModule>(isPresetMode());

	module->setEquipmentIdTemplate("$(PARENT)_MD$(PLACE)");
	module->setCaption(tr("Module"));
	module->setPlace(-1);

	addDeviceObject(module, parentModelIndex, true, true);

	emit updateState();
	return;
}

void EquipmentView::addController()
{
	QModelIndex parentModelIndex;		// current is root

	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.size() > 1)
	{
		// Don't know after which item insrt new object
		//
		return;
	}

	if (selected.empty() == false)
	{
		parentModelIndex = selected[0];
	}

	// --
	//
	std::shared_ptr<Hardware::DeviceObject> controller = std::make_shared<Hardware::DeviceController>(isPresetMode());

	controller->setEquipmentIdTemplate("$(PARENT)_CTRLXX");
	controller->setCaption(tr("Controller"));

	addDeviceObject(controller, parentModelIndex, true, true);

	emit updateState();
	return;
}

void EquipmentView::addAppSignalPort()
{
	QModelIndex parentModelIndex;		// current is root

	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.size() > 1)
	{
		// Don't know after which item insrt new object
		//
		return;
	}

	if (selected.empty() == false)
	{
		parentModelIndex = selected[0];
	}

	// --
	//
	std::shared_ptr<Hardware::DeviceObject> signal = std::make_shared<Hardware::DeviceAppSignal>(isPresetMode());

	signal->setEquipmentIdTemplate("$(PARENT)_SIGNAL");
	signal->setCaption(tr("Signal"));

	addDeviceObject(signal, parentModelIndex, true, true);

	emit updateState();
	return;
}

void EquipmentView::addDiagSignal()
{
	return addDiagSignalImpl(false);
}

void EquipmentView::addDiagSignalReflection()
{
	return addDiagSignalImpl(true);
}

void EquipmentView::addDiagSignalImpl(bool isReflection)
{
	QModelIndex parentModelIndex;		// current is root
	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.size() > 1)
	{
		// Don't know after which item insert new object
		//
		return;
	}

	if (selected.empty() == false)
	{
		parentModelIndex = selected[0];
	}

	// --
	//
	auto signal = std::make_shared<Hardware::DiagSignal>(isPresetMode());

	signal->setEquipmentIdTemplate("$(PARENT)_DIAGSIGNAL");
	signal->setCaption(isReflection ? QString{"DiagSignalReflection"} : QString{"DiagSignal"});
	signal->setSignalTypeId(tr("ST_ERR_DISCR"));

	signal->setIsReflection(isReflection);
	signal->setReflectedSignalId(isReflection ? QString{"SET_ID_TO_REFLECTED_SIGNAL"} : QString{""});

	addDeviceObject(signal, parentModelIndex, true, true);

	emit updateState();
	return;
}

void EquipmentView::addWorkstation()
{
	QModelIndex parentModelIndex;		// current is root

	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.size() > 1)
	{
		// Don't know after which item insert new object
		//
		return;
	}

	if (selected.empty() == false)
	{
		parentModelIndex = selected[0];
	}

	// --
	//
	std::shared_ptr<Hardware::DeviceObject> workstation = std::make_shared<Hardware::Workstation>(isPresetMode());

	workstation->setEquipmentIdTemplate("$(PARENT)_WS$(PLACE)");
	workstation->setCaption(tr("Workstation"));
	workstation->setPlace(0);

	addDeviceObject(workstation, parentModelIndex, true, true);

	emit updateState();
	return;
}

void EquipmentView::addSoftware()
{
	QModelIndex parentModelIndex;		// current is root

	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.size() > 1)
	{
		// Don't know after which item insrt new object
		//
		return;
	}

	if (selected.empty() == false)
	{
		parentModelIndex = selected[0];
	}

	// --
	//
	std::shared_ptr<Hardware::DeviceObject> software = std::make_shared<Hardware::Software>(isPresetMode());

	software->setEquipmentIdTemplate("$(PARENT)_SWNAME");
	software->setCaption(tr("Software"));
	software->setPlace(0);

	addDeviceObject(software, parentModelIndex, true, true);

	emit updateState();
	return;
}

void EquipmentView::addPreset()
{
	if (isConfigurationMode() == false)
	{
		Q_ASSERT(isConfigurationMode() == true);
		return;
	}

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() == 1)
	{
		QModelIndex singleSelectedIndex = selectedIndexList[0];

		auto selectedObject = equipmentModel()->deviceObject(singleSelectedIndex);
		if (selectedObject == nullptr)
		{
			Q_ASSERT(selectedObject != nullptr);
			return;
		}

		DialogChoosePreset d(this, db(), selectedObject->deviceType(), false);
		if (d.exec() == QDialog::Accepted && d.selectedPreset != nullptr)
		{
			const DbFileInfo* fio = d.selectedPreset->data();
			Q_ASSERT(fio);

			addPresetToConfiguration(*fio, true);
			emit updateState();
		}
	}

	return;
}

void EquipmentView::replaceObject()
{
	if (isConfigurationMode() == false)
	{
		Q_ASSERT(isConfigurationMode() == true);
		return;
	}

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() != 1)
	{
		Q_ASSERT(selectedIndexList.size() == 1);
		return;
	}

	QModelIndex singleSelectedIndex = selectedIndexList[0];

	auto selectedObject = equipmentModel()->deviceObject(singleSelectedIndex);
	if (selectedObject == nullptr)
	{
		Q_ASSERT(selectedObject != nullptr);
		return;
	}

	if (selectedObject->presetRoot() == false)
	{
		Q_ASSERT(selectedObject->presetRoot());
	}

	DialogChoosePreset d(this, db(), selectedObject->deviceType(), true);

	if (d.exec() == QDialog::Accepted &&
		d.selectedPreset != nullptr)
	{
		std::shared_ptr<Hardware::DeviceObject> selectedPreset = d.selectedPreset;

//		if (selectedObject->presetName() == selectedPreset->presetName())
//		{
//			QMessageBox::critical(this, qAppName(), tr("Cannot replace object to itself."));
//			return;
//		}

		// selectedObject can be not fully loaded, as not all branches could be open in the UI.
		// Load selectedObject from the equipment
		//
		std::shared_ptr<Hardware::DeviceObject> selectedObjectFullTree;

		const DbFileInfo* fio = selectedObject->data();
		Q_ASSERT(fio);

		bool ok = db()->getDeviceTreeLatestVersion(*fio, &selectedObjectFullTree, this);
		if (ok == false)
		{
			return;
		}

		Q_ASSERT(selectedObjectFullTree);

		// Delete selected device
		//
		deleteSelectedDevices();

		// Object will not be added to DB in the next line, it must be changed before adding
		//
		const DbFileInfo* presetFio = selectedPreset->data();
		Q_ASSERT(presetFio);

		std::shared_ptr<Hardware::DeviceObject> device = addPresetToConfiguration(*presetFio, false);

		if (device == nullptr || device->presetRoot() == false)
		{
			Q_ASSERT(device);
			Q_ASSERT(device->presetRoot());
			return;
		}

		// Update proprties form deleted device, recursive lambda
		//
		std::function<void(Hardware::DeviceObject*, Hardware::DeviceObject*)> updatePropertuFunc =
			[&updatePropertuFunc](Hardware::DeviceObject* dst, const Hardware::DeviceObject* src)
			{
				if (dst->deviceType() != src->deviceType() ||
					dst->equipmentIdTemplate() != src->equipmentIdTemplate())
				{
					return;
				}

				std::vector<std::shared_ptr<Property>> dstProps = dst->properties();

				for (std::shared_ptr<Property> dstProp : dstProps)
				{
					if (dstProp->caption() == Hardware::PropertyNames::presetName ||	// special case: we really don't want to update this prop, as it will brake an object
						dstProp->updateFromPreset() == true ||
						dstProp->readOnly() == true ||
						dstProp->visible() == false)
					{
						continue;
					}

					QVariant srcValue = src->propertyValue(dstProp->caption());

					if (srcValue.isValid() == false ||
						srcValue.typeId() != dstProp->value().typeId())
					{
						continue;
					}

					dstProp->setValue(srcValue);
				}

				for (int childIndex = 0; childIndex < dst->childrenCount(); childIndex++)
				{
					auto dstChild = dst->child(childIndex);
					Q_ASSERT(dstChild);

					QString dstChildTemplateId = dstChild->equipmentIdTemplate();

					// Find the pair for this child
					//
					for (int srcChildIndex = 0; srcChildIndex < src->childrenCount(); srcChildIndex++)
					{
						auto srcChild = src->child(srcChildIndex);
						Q_ASSERT(srcChild);

						if (srcChild->equipmentIdTemplate() == dstChildTemplateId)
						{
							updatePropertuFunc(dstChild.get(), srcChild.get());
							break;
						}
					}
				}

				return;
			};

		updatePropertuFunc(device.get(), selectedObjectFullTree.get());	// Recursive func

		// Add device
		//
		addDeviceObject(device, selectionModel()->selectedRows().at(0), true, true);
		emit updateState();
	}

	return;
}


void EquipmentView::addPresetRack()
{
	if (isPresetMode() == true)
	{
		QModelIndex parentModelIndex;		// current is root

		// --
		//
		std::shared_ptr<Hardware::DeviceObject> rack = std::make_shared<Hardware::DeviceRack>(true);

		rack->setEquipmentIdTemplate("$(PARENT)_RACKID");
		rack->setCaption(tr("Rack"));
		rack->setPlace(0);

		rack->setPresetRoot(true);
		rack->setPresetName("PRESET_NAME");

		addDeviceObject(rack, parentModelIndex, true, true);
	}
	else
	{
		choosePreset(Hardware::DeviceType::Rack);
	}

	emit updateState();
	return;
}

void EquipmentView::addPresetChassis()
{
	if (isPresetMode() == true)
	{
		QModelIndex parentModelIndex;		// current is root

		// --
		//
		std::shared_ptr<Hardware::DeviceObject> chassis = std::make_shared<Hardware::DeviceChassis>(true);

		chassis->setEquipmentIdTemplate("$(PARENT)_CHASSISID");
		chassis->setCaption(tr("Chassis"));
		chassis->setPlace(-1);

		chassis->setPresetRoot(true);
		chassis->setPresetName("PRESET_NAME");

		addDeviceObject(chassis, parentModelIndex, true, true);
	}
	else
	{
		choosePreset(Hardware::DeviceType::Chassis);
	}

	emit updateState();
}

void EquipmentView::addPresetModule()
{
	if (isPresetMode() == true)
	{
		QModelIndex parentModelIndex;		// current is root

		// --
		//
		std::shared_ptr<Hardware::DeviceObject> module = std::make_shared<Hardware::DeviceModule>(true);

		module->setEquipmentIdTemplate("$(PARENT)_MD00");
		module->setCaption(tr("Module"));
		module->setPlace(-1);

		module->setPresetRoot(true);
		module->setPresetName("PRESET_NAME");

		addDeviceObject(module, parentModelIndex, true, true);
	}
	else
	{
		choosePreset(Hardware::DeviceType::Module);
	}

	emit updateState();
}

void EquipmentView::addPresetController()
{
	if (isPresetMode() == true)
	{
		QModelIndex parentModelIndex;		// current is root

		// --
		//
		std::shared_ptr<Hardware::DeviceObject> controller = std::make_shared<Hardware::DeviceController>(true);

		controller->setEquipmentIdTemplate("$(PARENT)_CRRLXXX");
		controller->setCaption(tr("Controller"));

		controller->setPresetRoot(true);
		controller->setPresetName("PRESET_NAME");

		addDeviceObject(controller, parentModelIndex, true, true);
	}
	else
	{
		choosePreset(Hardware::DeviceType::Controller);
	}

	emit updateState();
}

void EquipmentView::addPresetWorkstation()
{
	if (isPresetMode() == true)
	{
		QModelIndex parentModelIndex;		// current is root

		// --
		//
		std::shared_ptr<Hardware::DeviceObject> workstation = std::make_shared<Hardware::Workstation>(true);

		workstation->setEquipmentIdTemplate("$(PARENT)_WS00");
		workstation->setCaption(tr("Workstation"));
		workstation->setPlace(0);

		workstation->setPresetRoot(true);
		workstation->setPresetName("PRESET_NAME");

		addDeviceObject(workstation, parentModelIndex, true, true);
	}
	else
	{
		choosePreset(Hardware::DeviceType::Workstation);
	}

	emit updateState();
}

void EquipmentView::addPresetSoftware()
{
	if (isPresetMode() == true)
	{
		QModelIndex parentModelIndex;		// current is root

		// --
		//
		std::shared_ptr<Hardware::DeviceObject> software = std::make_shared<Hardware::Software>(true);

		software->setEquipmentIdTemplate("$(PARENT)_SWNAME");
		software->setCaption(tr("Software"));
		software->setPlace(0);

		software->setPresetRoot(true);
		software->setPresetName("PRESET_NAME");

		addDeviceObject(software, parentModelIndex, true, true);
	}
	else
	{
		choosePreset(Hardware::DeviceType::Software);
	}

	emit updateState();
}


void EquipmentView::choosePreset(Hardware::DeviceType type)
{
	if (isConfigurationMode() == false)
	{
		Q_ASSERT(isConfigurationMode() == true);
		return;
	}

	// Get file list
	//
	std::vector<DbFileInfo> fileList;

	bool ok = db()->getFileList(
				&fileList,
				db()->systemFileId(DbDir::HardwarePresetsDir),
				Hardware::DeviceObject::fileExtension(type),
				true,
				this);

	if (ok == false || fileList.empty() == true)
	{
		return;
	}

	// Read files from DB
	//
	std::vector<std::shared_ptr<DbFile>> files;
	files.reserve(fileList.size());

	ok = db()->getLatestVersion(fileList, &files, this);

	if (ok == false || files.empty() == true)
	{
		return;
	}

	// Read DeviceObjects from raw data
	//
	std::vector<std::shared_ptr<Hardware::DeviceObject>> presets;
	presets.reserve(files.size());

	for (std::shared_ptr<DbFile>& f : files)
	{
		auto object = DbControllerTools::deviceObjectFromDbFile(*f);
		Q_ASSERT(object != nullptr);

		presets.push_back(object);
	}

	// Choose preset
	//
	QMenu* menu=new QMenu(this);

	for (std::shared_ptr<Hardware::DeviceObject>& p : presets)
	{
		QAction* a = new QAction(p->caption(), this);

		connect(a, &QAction::triggered,
			[this, p]()
			{
				const DbFileInfo* fio = p->data();
				Q_ASSERT(fio);

				addPresetToConfiguration(*fio, true);
			});

		menu->addAction(a);
	}

	menu->exec(this->cursor().pos());

	return;
}

std::shared_ptr<Hardware::DeviceObject> EquipmentView::addPresetToConfiguration(const DbFileInfo& fileInfo, bool addToEquipment)
{
	Q_ASSERT(fileInfo.fileId() != -1);
	Q_ASSERT(fileInfo.parentId() != -1);
	Q_ASSERT(fileInfo.parentId() == db()->systemFileId(DbDir::HardwarePresetsDir));

	// Read all preset tree and add it to the hardware configuration
	//
	std::shared_ptr<Hardware::DeviceObject> device;

	bool ok = db()->getDeviceTreeLatestVersion(fileInfo, &device, this);
	if (ok == false)
	{
		return std::shared_ptr<Hardware::DeviceObject>();
	}

	const DbFileInfo* deviceFileInfo = device->data();
	Q_ASSERT(deviceFileInfo);

	if (deviceFileInfo == nullptr ||
		deviceFileInfo->fileId() != fileInfo.fileId() ||
		device->presetRoot() == false)
	{
		Q_ASSERT(deviceFileInfo);
		Q_ASSERT(deviceFileInfo->fileId() == fileInfo.fileId());
		Q_ASSERT(device->presetRoot() == true);
		return std::shared_ptr<Hardware::DeviceObject>();
	}

	// If this is LM modlue, then set SusbSysID to the default value
	//
	if (device->isModule() == true)
	{
		auto module = device->toModule();

		if (module != nullptr && module->isFSCConfigurationModule() == true)
		{
			// Get susbsystems
			//
			Builder::SubsystemStorage subsystems;
			QString errorCode;

			if (subsystems.load(db(), errorCode) == true && subsystems.count() > 0)
			{
				std::shared_ptr<Hardware::Subsystem> subsystem = subsystems.get(0);

				if (module->propertyExists("SubsystemID") == false)
				{
					Q_ASSERT(false);
				}
				else
				{
					module->setPropertyValue("SubsystemID", QVariant::fromValue(subsystem->subsystemId()));
				}
			}
		}
	}

	// Reset fileInfo in all objects
	//

	// Add new device
	//
	QModelIndex parentModelIndex;		// current is root
	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.size() > 1)
	{
		// Don't know after which item insrt new object
		//
		return std::shared_ptr<Hardware::DeviceObject>();
	}

	if (selected.empty() == false)
	{
		parentModelIndex = selected[0];
	}

	// --
	//
	if (addToEquipment == true)
	{
		addDeviceObject(device, parentModelIndex, true, true);
	}
	else
	{
		// Do nothing - probably object needs to be changed before adding
		//
	}

	return device;
}

QModelIndex EquipmentView::addDeviceObject(std::shared_ptr<Hardware::DeviceObject> object,
										   QModelIndex parentModelIndex,
										   bool clearPrevSelection,
										   bool newUuids)
{
	if (object == nullptr)
	{
		Q_ASSERT(object != nullptr);
		return QModelIndex();
	}

	if (isPresetMode() == true && object->isPreset() == false)
	{
		Q_ASSERT(false);
		return QModelIndex();
	}

	// Set new id, recusively to all children
	//
	bool presetMode = isPresetMode();

	std::function<void(Hardware::DeviceObject*)> setUuid = [&setUuid, presetMode](Hardware::DeviceObject* object)
		{
			Q_ASSERT(object);

			object->setUuid(QUuid::createUuid());

			if (presetMode == true)
			{
				object->setPresetObjectUuid(object->uuid());
			}

			for (int i = 0; i < object->childrenCount(); i++)
			{
				setUuid(object->child(i).get());
			}
		};

	if (newUuids == true)
	{
		setUuid(object.get());
	}

	// Set Parent
	//
	std::shared_ptr<Hardware::DeviceObject> parentObject = nullptr;
	//QModelIndex parentIndex;	// Currently it is root;

	if (isPresetMode() == true &&
		object->isPreset() == true &&
		object->presetRoot() == true)
	{
		parentObject = equipmentModel()->deviceObject(parentModelIndex);
	}
	else
	{
//		QModelIndexList selected = selectionModel()->selectedRows();

//		if (selected.size() > 1)
//		{
//			// Don't know after which item insrt new object
//			//
//			return;
//		}

//		if (selected.empty() == false)
//		{
//			parentIndex = selected[0];
//		}

		// --
		//
		parentObject = equipmentModel()->deviceObject(parentModelIndex);
		Q_ASSERT(parentObject);

		if (object->deviceType() == Hardware::DeviceType::Software &&
			(parentObject->deviceType() != Hardware::DeviceType::Workstation && parentObject->deviceType() != Hardware::DeviceType::Software))
		{
			Q_ASSERT(false);
			return QModelIndex();
		}

		if (parentObject->deviceType() == object->deviceType())
		{
			// add the same item to the end of the the parent
			//
			parentModelIndex = parentModelIndex.parent();
			parentObject = equipmentModel()->deviceObject(parentModelIndex);

			Q_ASSERT(parentObject->deviceType() < object->deviceType());
		}

		if (parentObject->deviceType() > object->deviceType() ||
			(object->deviceType() == Hardware::DeviceType::Workstation && parentObject->deviceType() > Hardware::DeviceType::Chassis))
		{
			Q_ASSERT(parentObject->deviceType() <= object->deviceType());
			return QModelIndex();
		}
	}

	// Debugging .... parentObject->setChildRestriction("function(device) { return device.Place >=0 && device.Place < 16; }");
	//
	if (parentObject == nullptr)
	{
		Q_ASSERT(parentObject != nullptr);
		return {};
	}

	//  Set presetName, parent object should contain it
	//
	if (isPresetMode() == true &&
		object->isPreset() == true &&
		object->presetRoot() == false &&
		parentObject != nullptr)
	{
		object->setPresetName(parentObject->presetName());
	}

	// Add device to DB
	//
	const DbFileInfo* parentObjectFileInfo = parentObject->data();
	Q_ASSERT(parentObjectFileInfo);

	bool result = db()->addDeviceObject(object.get(), parentObjectFileInfo->fileId(), this);

	if (result == false)
	{
		return QModelIndex();
	}

	// Add new device to the model and select it
	//
	object->deleteAllChildren();	// if there are any children they will be added to the model by reading project database

	equipmentModel()->insertDeviceObject(object, parentModelIndex);

	QModelIndex objectModelIndex = equipmentModel()->index(parentObject->childIndex(object), parentModelIndex);

	if (clearPrevSelection == true)
	{
		selectionModel()->clearSelection();
	}

	selectionModel()->select(objectModelIndex, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);

	if (clearPrevSelection == true)
	{
		setCurrentIndex(objectModelIndex);
	}

	return objectModelIndex;
}

void EquipmentView::createInOutsToSignals()
{
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.isEmpty() == true)
	{
		Q_ASSERT(selectedIndexList.isEmpty() == false);	// how did we get here?
		return;
	}

	// There are two possible options:
	//	- Add signal for ONE selected module
	//	- Add selected AppSignals
	//
	{
		auto device = equipmentModel()->deviceObject(selectedIndexList.front());
		if (device == nullptr)
		{
			Q_ASSERT(device);
			return;
		}

		// Add signal for ONE selected module
		//
		if (selectedIndexList.size() == 1 && device->isModule() == true)
		{
			std::shared_ptr<Hardware::DeviceModule> module = device->toModule();
			Q_ASSERT(module);

			addInOutsToSignals(module);
			return;
		}
	}

	// Add selected AppSignals
	//

	// Check all selected are AppSignals
	//
	std::vector<std::shared_ptr<Hardware::DeviceAppSignal>> hardwareAppSignals;
	hardwareAppSignals.reserve(selectedIndexList.size());

	for (auto si : selectedIndexList)
	{
		auto deviceObject = equipmentModel()->deviceObject(si);
		if (deviceObject == nullptr)
		{
			Q_ASSERT(deviceObject);
			return;
		}

		if (deviceObject->isAppSignal() == false)
		{
			Q_ASSERT(deviceObject->isAppSignal());
			return;
		}

		hardwareAppSignals.push_back(deviceObject->toAppSignal());
	}

	addInOutsToSignals(hardwareAppSignals);

	return;
}

void EquipmentView::addInOutsToSignals(std::shared_ptr<Hardware::DeviceModule> module)
{
	if (module == nullptr)
	{
		Q_ASSERT(module);
		return;
	}

	const DbFileInfo* moduleFileInfo = module->data();
	if (moduleFileInfo == nullptr)
	{
		Q_ASSERT(moduleFileInfo);
		return;
	}

	// Check if the Place property is correct for the object and all it's parents
	//
	std::shared_ptr<Hardware::DeviceObject> checkPlaceObject = module;

	while (checkPlaceObject != nullptr)
	{
		if (checkPlaceObject->isRoot() == false && checkPlaceObject->place() < 0)
		{
			QMessageBox::critical(this,
								  QApplication::applicationName(),
								  tr("Object's %1 property Place is %2, set the correct value (>=0).")
								  .arg(checkPlaceObject->equipmentId())
								  .arg(checkPlaceObject->place())
								  );
			return;
		}

		checkPlaceObject = checkPlaceObject->parent();
	}

	// Get module from the DB as here it can be not fully loaded
	//
	std::shared_ptr<Hardware::DeviceObject> dbModule;
	bool ok = db()->getDeviceTreeLatestVersion(*moduleFileInfo, &dbModule, this);

	if (ok == false)
	{
		return;
	}

	// Get all hardware inputs outputs from the module
	//
	std::vector<const Hardware::DeviceAppSignal*> inOuts;

	std::function<void(Hardware::DeviceObject*)> getInOuts =
		[&inOuts, &getInOuts](Hardware::DeviceObject* device)
		{
			if (device->deviceType() == Hardware::DeviceType::AppSignal)
			{
				Hardware::DeviceAppSignal* signal = dynamic_cast<Hardware::DeviceAppSignal*>(device);
				Q_ASSERT(signal);

				if (signal->function() == E::SignalFunction::Input ||
					signal->function() == E::SignalFunction::Output ||
					signal->function() == E::SignalFunction::Validity)
				{
					inOuts.push_back(signal);
				}

				return;
			}

			for (int i = 0; i < device->childrenCount(); i++)
			{
				getInOuts(device->child(i).get());
			}
		};

	getInOuts(dbModule.get());

	if (inOuts.empty() == true)
	{
		return;
	}

	// Expand StrID for signals,
	// track parents from the module, and children from the dbModule
	//
	std::list<std::shared_ptr<Hardware::DeviceObject>> equipmentDevices;

	std::shared_ptr<Hardware::DeviceObject> equipmentDevice = module;
	while (equipmentDevice != nullptr)
	{
		if (equipmentDevice != module)
		{
			QByteArray bytes;
			equipmentDevice->saveToByteArray(&bytes);	// save and restore to keep equpment version after expanding strid

			std::shared_ptr<Hardware::DeviceObject> newObject = Hardware::DeviceObject::Create(bytes);

			equipmentDevices.push_front(newObject);
		}

		equipmentDevice = equipmentDevice->parent();
	}

	if (equipmentDevices.empty() != true)
	{
		auto it = equipmentDevices.begin();

		do
		{
			std::shared_ptr<Hardware::DeviceObject> parent = *it;

			++ it;

			if (it != equipmentDevices.end())
			{
				parent->addChild(*it);
			}
			else
			{
				parent->addChild(dbModule);
			}
		}
		while(it != equipmentDevices.end());

		equipmentDevices.front()->expandEquipmentId();
	}

	dbModule->expandEquipmentId();	// StrIds in getInOuts will be updated also

	// Add signals to the project DB
	//
	std::sort(std::begin(inOuts), std::end(inOuts),
		[](const Hardware::DeviceObject* a, const Hardware::DeviceObject* b)
		{
			return a->equipmentIdTemplate() < b->equipmentIdTemplate();
		});

	std::vector<AppSignal> addedSignals;

	bool result = AppSignalSetProvider::getInstance()->autoAddSignals(inOuts, &addedSignals, this);

	qDebug() << "Signals added:" << addedSignals.size();

	if (result == true)
	{
		QString messageText;

		if (addedSignals.empty() == true)
		{
			messageText = tr("Application Logic signals for inputs/outputs were not added. Apparently they were added earlier.");
		}

		if (addedSignals.size() == 1)
		{
			messageText = tr("1 Application Logic signal for inputs/outputs was added.");
		}

		if (addedSignals.size() > 1)
		{
			messageText = tr("%1 Application Logic signals for inputs/outputs were added.").arg(addedSignals.size());
		}

		QMessageBox::information(this, qAppName(), messageText);

		// Show application signals for current module
		//
		if (addedSignals.empty() == false)
		{
			showAppSignals(true, false);
		}
	}

	return;
}

void EquipmentView::addInOutsToSignals(std::vector<std::shared_ptr<Hardware::DeviceAppSignal>> hardwareAppSignals)
{
	Q_ASSERT(hardwareAppSignals.empty() == false);

	// Check if the Place property is correct for signals and all their parents
	//
	for (auto has : hardwareAppSignals)
	{
		std::shared_ptr<Hardware::DeviceObject> checkPlaceObject = has;

		while (checkPlaceObject != nullptr)
		{
			if (checkPlaceObject->isRoot() == false && checkPlaceObject->place() < 0)
			{
				QMessageBox::critical(this,
									  QApplication::applicationName(),
									  tr("Object's %1 property Place is %2, set the correct value (>=0).")
									  .arg(checkPlaceObject->equipmentId())
									  .arg(checkPlaceObject->place())
									  );
				return;
			}

			checkPlaceObject = checkPlaceObject->parent();
		}
	}

	// Get all hardware inputs outputs
	//
	std::vector<const Hardware::DeviceAppSignal*> inOuts;

	for (auto has : hardwareAppSignals)
	{
		if (has->function() == E::SignalFunction::Input ||
			has->function() == E::SignalFunction::Output ||
			has->function() == E::SignalFunction::Validity)
		{
			inOuts.push_back(has.get());
		}
	}

	if (inOuts.empty() == true)
	{
		return;
	}

	// Expand StrID for signals
	//
	std::vector<std::shared_ptr<Hardware::DeviceObject>> equipmentDevices;
	equipmentDevices.reserve(inOuts.size());

	for (const Hardware::DeviceAppSignal* has : inOuts)		// has - hardware app signal
	{
		QByteArray bytes;
		has->saveToByteArray(&bytes);	// save and restore to keep equpment version after expanding strid

		std::shared_ptr<Hardware::DeviceObject> newObject = Hardware::DeviceObject::Create(bytes);
		equipmentDevices.push_back(newObject);

		newObject->setEquipmentIdTemplate(has->equipmentId());	// Setting expanded EquipmentID
	}

	inOuts.clear();
	for (auto devo : equipmentDevices)
	{
		inOuts.push_back(devo->toAppSignal().get());
	}

	// Add signals to the project DB
	//
	std::sort(std::begin(inOuts), std::end(inOuts),
		[](const Hardware::DeviceObject* a, const Hardware::DeviceObject* b)
		{
			return a->equipmentIdTemplate() < b->equipmentIdTemplate();
		});

	std::vector<AppSignal> addedSignals;

	bool result = AppSignalSetProvider::getInstance()->autoAddSignals(inOuts, &addedSignals, this);

	qDebug() << "Signals added:" << addedSignals.size();

	if (result == true)
	{
		QString messageText;

		if (addedSignals.empty() == true)
		{
			messageText = tr("Application Logic signals for inputs/outputs were not added. Apparently they were added earlier.");
		}

		if (addedSignals.size() == 1)
		{
			messageText = tr("1 Application Logic signal for inputs/outputs was added.");
		}

		if (addedSignals.size() > 1)
		{
			messageText = tr("%1 Application Logic signals for inputs/outputs were added.").arg(addedSignals.size());
		}

		QMessageBox::information(this, qAppName(), messageText);

		// Show application signals for current module
		//
		if (addedSignals.empty() == false)
		{
			showAppSignals(true, true);
		}
	}

	return;
}

void EquipmentView::showAppSignals(bool refreshSignalList /*= false*/, bool exactMatch)
{
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.isEmpty() == true)
	{
		Q_ASSERT(false);	// how did we get here?
		return;
	}

	QStringList strIds;

	for (const QModelIndex& mi : selectedIndexList)
	{
		auto device = equipmentModel()->deviceObject(mi);
		Q_ASSERT(device);

		if (device != nullptr)
		{
			if (exactMatch == true)
			{
				strIds.push_back(device->equipmentId());
			}
			else
			{
				strIds.push_back(device->equipmentId() + "*");
			}
		}
	}

	GlobalMessanger::instance().fireShowDeviceApplicationSignals(strIds, refreshSignalList);

	return;
}

void EquipmentView::createInternalAppSignal()
{
	qDebug() << "void EquipmentView::addAppSignal()";

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() != 1)
	{
		Q_ASSERT(false);	// how did we get here?
		return;
	}

	auto device = equipmentModel()->deviceObject(selectedIndexList.front());
	if (device == nullptr)
	{
		Q_ASSERT(device);
		return;
	}

	auto module = device->toModule();
	if (module == nullptr || module->isLogicModule() == false)
	{
		Q_ASSERT(module);
		return;
	}

	QStringList equipmentIdList;
	equipmentIdList << module->equipmentId();

	static CreatingSignalDialogOptions options;

	options.init(module->equipmentId(),
				 module->equipmentId(),
				 equipmentIdList,
				 QStringList());

	CreateSignalDialog::showDialog(db(), &options, this);

	return;
}

void EquipmentView::addLogicSchemaToLm()
{
	qDebug() << __FUNCTION__;

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();
	if (selectedIndexList.empty() == true)
	{
		Q_ASSERT(false);
		return;
	}

	QStringList deviceStrIds;
	QString lmDescriptioFile;
	bool lmDescriptioFileInitialized = false;

	for (const QModelIndex& mi : selectedIndexList)
	{
		auto device = equipmentModel()->deviceObject(mi);
		Q_ASSERT(device);

		deviceStrIds.push_back(device->equipmentId());

		if (device->isModule() == true &&
			device->toModule()->isLogicModule() == true)
		{
			QString thisModuleLmDescriprtionFile = device->propertyValue(Hardware::PropertyNames::lmDescriptionFile).toString();

			if (lmDescriptioFileInitialized == false)
			{
				lmDescriptioFile = thisModuleLmDescriprtionFile;
				lmDescriptioFileInitialized = true;
				continue;
			}

			if (lmDescriptioFile != thisModuleLmDescriprtionFile)
			{
				Q_ASSERT(false);	// How is it possible, it should be fileterd om nenu level
				return;
			}

			continue;
		}
		else
		{
			Q_ASSERT(false);	// How is it possible, it should be fileterd om nenu level
			return;
		}
	}

	GlobalMessanger::instance().fireAddLogicSchema(deviceStrIds, lmDescriptioFile);
	return;
}

void EquipmentView::showLogicSchemaForLm()
{
	qDebug() << __FUNCTION__;

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() != 1)
	{
		Q_ASSERT(false);	// how did we get here?
		return;
	}

	auto device = equipmentModel()->deviceObject(selectedIndexList.front());
	Q_ASSERT(device);

	auto module = device->toModule();
	if (module == nullptr || module->isLogicModule() == false)
	{
		Q_ASSERT(module);
		return;
	}

	GlobalMessanger::instance().fireSearchSchemaForLm(module->equipmentId());

	return;
}

void EquipmentView::createConnection()
{
	qDebug() << __FUNCTION__;

	if (db()->isProjectOpened() == false)
	{
		return;
	}

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() != 1 && selectedIndexList.size() != 2)
	{
		Q_ASSERT(false);	// how did we get here?
		return;
	}

	const auto controller1 = equipmentModel()->deviceObject(selectedIndexList.front())->toController();
	const auto controller2 = equipmentModel()->deviceObject(selectedIndexList.back())->toController();

	if (controller1 == nullptr || controller2 == nullptr)
	{
		Q_ASSERT(controller1);
		Q_ASSERT(controller2);
		return;
	}

	if (theDialogConnections == nullptr)
	{
		theDialogConnections = new DialogConnections(db(), this);
		theDialogConnections->show();
	}
	else
	{
		theDialogConnections->activateWindow();
	}

	QString port1Id = controller1->equipmentId();
	QString port2Id = controller2->equipmentId();

	if (port1Id == port2Id)
	{
		port2Id.clear();	// Single port connection
	}

	bool ok = theDialogConnections->addConnection(port1Id, port2Id);
	if (ok == true)
	{
		theDialogConnections->setFilter(controller1->equipmentId());
	}

	return;
}

void EquipmentView::showObjectConnections()
{
	qDebug() << __FUNCTION__;

	if (db()->isProjectOpened() == false)
	{
		return;
	}

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() == 0)
	{
		Q_ASSERT(false);	// how did we get here?
		return;
	}

	QString filter;
	for (auto mi : selectedIndexList)
	{
		auto device = equipmentModel()->deviceObject(mi);
		if (device == nullptr)
		{
			Q_ASSERT(device);
			return;
		}

		if (filter.isEmpty() == true)
		{
			filter = device->equipmentId();
		}
		else
		{
			filter.append("; ");
			filter.append(device->equipmentId());
		}
	}

	if (theDialogConnections == nullptr)
	{
		theDialogConnections = new DialogConnections(db(), this);
		theDialogConnections->show();
	}
	else
	{
		theDialogConnections->activateWindow();
	}

	theDialogConnections->setFilter(filter);

	return;
}

void EquipmentView::copySelectedDevices()
{
	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.empty())
	{
		Q_ASSERT(false);		// How did we get here, action should be disabled
		return;
	}

	// Check if all selected equipmnet has the same type
	//
	auto firstDevice = equipmentModel()->deviceObject(selected.first());
	Q_ASSERT(firstDevice);

	Hardware::DeviceType type = firstDevice->deviceType();
	bool allObjectsArePresetRoots = true;

	std::vector<const Hardware::DeviceObject*> devices;
	devices.reserve(selected.size());

	for (const QModelIndex& mi : selected)
	{
		const auto device = equipmentModel()->deviceObject(mi);
		Q_ASSERT(device);

		if (type != device->deviceType())
		{
			Q_ASSERT(false);
			return;
		}

		allObjectsArePresetRoots &= device->presetRoot() & device->isPreset();

		devices.push_back(device.get());
	}

	// Read devices from the project database
	//
	std::vector<std::shared_ptr<Hardware::DeviceObject>> latestDevices;
	latestDevices.reserve(devices.size());

	for (const Hardware::DeviceObject* device : devices)
	{
		const DbFileInfo* deviceFileInfo = device->data();
		Q_ASSERT(deviceFileInfo);

		std::shared_ptr<Hardware::DeviceObject> out;

		bool result = db()->getDeviceTreeLatestVersion(*deviceFileInfo, &out, this);

		if (result == false)
		{
			return;
		}

		Q_ASSERT(out);

		latestDevices.push_back(out);
	}

	// Save devices to the clipboard
	//
	::Proto::EnvelopeSet message;

	// Save the short description -- its done for quick understanding what is in the clipboard without reading all data
	//
	::Proto::EnvelopeSetShortDescription descriptionMessage;

	descriptionMessage.set_projectname(db()->currentProject().projectName().toStdString());
	descriptionMessage.set_username(db()->currentUser().username().toStdString());
	descriptionMessage.set_exporttime(QDateTime::currentDateTime().toSecsSinceEpoch());
	descriptionMessage.set_projectdbversion(DbController::databaseVersion());
	descriptionMessage.set_equipmenteditor(isConfigurationMode());
	descriptionMessage.set_preseteditor(isPresetMode());
	descriptionMessage.set_presetroot(allObjectsArePresetRoots);

	for (std::shared_ptr<Hardware::DeviceObject> device : latestDevices)
	{
		auto protoDevice = message.add_items();
		device->SaveObjectTree(protoDevice);

		descriptionMessage.add_classnamehash(protoDevice->classnamehash());
		descriptionMessage.add_devicetype(static_cast<qint32>(device->deviceType()));
	}

	// Save objects (EnvelopeSet) to byte array
	//
	std::string dataString;
	bool ok = message.SerializeToString(&dataString);

	if (ok == false)
	{
		Q_ASSERT(ok);
		return;
	}

	// Save short description (EnvelopeSetShortDescription) to byte array
	//
	std::string descriptionDataString;
	ok = descriptionMessage.SerializeToString(&descriptionDataString);

	if (ok == false)
	{
		Q_ASSERT(ok);
		return;
	}

	// Set data to clipboard
	//
	QByteArray ba(dataString.data(), static_cast<int>(dataString.size()));
	QByteArray descrba(descriptionDataString.data(), static_cast<int>(descriptionDataString.size()));

	if (ba.isEmpty() == false &&
		descrba.isEmpty() == false)
	{
		QClipboard* clipboard = QApplication::clipboard();

		QMimeData* mime = new QMimeData();
		mime->setData(EquipmentView::mimeType, ba);
		mime->setData(EquipmentView::mimeTypeShortDescription, descrba);

		clipboard->clear();
		clipboard->setMimeData(mime);
	}

	return;
}

void EquipmentView::pasteDevices()
{
	bool pasetIsAllowed = canPaste();

	if (pasetIsAllowed == false)
	{
		// How did we get here?
		//
		Q_ASSERT(pasetIsAllowed == true);
		return;
	}

	// Check the clipboard content
	//
	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();

	if (mimeData == nullptr)
	{
		return;
	}

	QStringList hasFormats = mimeData->formats();

	bool hasFullData = false;
	bool hasShortDescription = false;

	for (auto f : hasFormats)
	{
		if (f == EquipmentView::mimeType)
		{
			hasFullData = true;
		}

		if (f == EquipmentView::mimeTypeShortDescription)
		{
			hasShortDescription = true;
		}
	}

	if (hasFullData == false ||
		hasShortDescription == false)
	{
		return;
	}

	// Read short description for the clipboard content
	//
	QByteArray cbData = mimeData->data(EquipmentView::mimeType);

	::Proto::EnvelopeSet messageItems;
	bool ok = messageItems.ParseFromArray(cbData.constData(), static_cast<int>(cbData.size()));

	if (ok == false)
	{
		QMessageBox::critical(this, qApp->applicationName(),  tr("The Clipboard has been corrupted or has incompatible data format."));
		return;
	}

	// Read short description for the clipboard content
	//
	cbData = mimeData->data(EquipmentView::mimeTypeShortDescription);

	::Proto::EnvelopeSetShortDescription messageDescr;
	ok = messageDescr.ParseFromArray(cbData.constData(), static_cast<int>(cbData.size()));

	if (ok == false)
	{
		return;
	}


	// --
	//
	pasteDevices(messageItems, messageDescr, true);

	return;
}

void EquipmentView::pasteDevices(const ::Proto::EnvelopeSet& messageItems,
								 const ::Proto::EnvelopeSetShortDescription& messageDescr,
								 bool newUuids)		// Generate UUIDs is false for importing presets. Preset has PresetObjectUuid which is used for matching preset and its' equipment
{
	QModelIndex parentModelIndex;		// current is root
	QModelIndexList selected = selectionModel()->selectedRows();

	if (isPresetMode() == true && messageDescr.presetroot() == true)
	{
		// insert new presets into the root
		//
	}
	else
	{
		if (selected.size() > 1)
		{
			// Don't know after which item insrt new object
			//
			return;
		}

		if (selected.empty() == false)
		{
			parentModelIndex = selected[0];
		}
	}

	// --
	//

	// Create objects and add children
	//

	// function for setting new uuids
	//
	std::function<void(Hardware::DeviceObject*)> setUuid = [&setUuid](Hardware::DeviceObject* object)
		{
			Q_ASSERT(object);
			object->setUuid(QUuid::createUuid());

			for (int i = 0; i < object->childrenCount(); i++)
			{
				setUuid(object->child(i).get());
			}
		};

	selectionModel()->clearSelection();
	QModelIndex lastModelIndex;

	for (int i = 0; i < messageItems.items_size(); i++)
	{
		std::shared_ptr<Hardware::DeviceObject> object(Hardware::DeviceObject::Create(messageItems.items(i)));

		if (object == nullptr)
		{
			QMessageBox::critical(this, qApp->applicationName(),  tr("The Clipboard has been corrupted or has incompatible data format. Object data parsing error."));
			break;
		}

		if (newUuids == true)
		{
			setUuid(object.get());		// Set new guids to all objects
		}

		lastModelIndex = addDeviceObject(object, parentModelIndex, false, newUuids);
	}

	if (lastModelIndex.isValid() == true)
	{
		selectionModel()->setCurrentIndex(lastModelIndex, QItemSelectionModel::NoUpdate);
	}

	// --
	//
	emit updateState();

	return;
}

bool EquipmentView::canPaste() const
{
	// Check the clipboard content
	//
	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();

	if (mimeData == nullptr)
	{
		return false;
	}

	QStringList hasFormats = mimeData->formats();

	bool hasFullData = false;
	bool hasShortDescription = false;

	for (auto f : hasFormats)
	{
		if (f == EquipmentView::mimeType)
		{
			hasFullData = true;
		}

		if (f == EquipmentView::mimeTypeShortDescription)
		{
			hasShortDescription = true;
		}
	}

	if (hasFullData == false ||
		hasShortDescription == false)
	{
		return false;
	}

	// Read short description for the clipboard content
	//
	QByteArray cbData = mimeData->data(EquipmentView::mimeTypeShortDescription);

	::Proto::EnvelopeSetShortDescription message;
	bool ok = message.ParseFromArray(cbData.constData(), static_cast<int>(cbData.size()));

	if (ok == false)
	{
		return false;
	}

	return canPaste(message);
}

bool EquipmentView::canPaste(const ::Proto::EnvelopeSetShortDescription& message) const
{
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	// Check if the copy was done from current mode, so copy possible only from editor to edir or preset editor to preset editor
	//
	if (isPresetMode() == true && message.preseteditor() == false)
	{
		return false;
	}

	// Allow insert new whole preset(s) in preset editor
	//
	if (isPresetMode() == true && message.presetroot() == true)
	{
		return true;
	}

	if (isConfigurationMode() == true && message.equipmenteditor() == false)
	{
		return false;
	}

	// Is it possible to insert it into current selection
	//
	if (selectedIndexList.size() != 1)
	{
		return false;
	}

	// --
	//
	if (message.classnamehash().size() == 0 || message.devicetype().size() == 0)
	{
		Q_ASSERT(message.classnamehash().size() > 0);
		Q_ASSERT(message.devicetype().size() > 0);
		return false;
	}

	Q_ASSERT(message.classnamehash().size() == message.devicetype().size());

	std::shared_ptr<Hardware::DeviceObject> deviceObject;

	for (int i = 0; i < message.classnamehash_size(); i++)
	{
		quint32 classNameHash = message.classnamehash(i);

		bool canCreateInstance = Hardware::canCreateDevice(classNameHash);
		if (canCreateInstance == false)
		{
			Q_ASSERT(canCreateInstance);
			return false;
		}
	}

	// Check if chese devices can be added
	//
	auto selectedDevice = equipmentModel()->deviceObject(selectedIndexList.at(0));
	Q_ASSERT(selectedDevice);

	if (selectedDevice == nullptr)
	{
		Q_ASSERT(selectedDevice);
		return false;
	}

	for (int i = 0; i < message.devicetype_size(); i++)
	{
		Hardware::DeviceType type = static_cast<Hardware::DeviceType>(message.devicetype(i));

		if (selectedDevice->canAddChild(type) == false)
		{
			return false;
		}
	}

	return true;
}


void EquipmentView::findObject()
{
	static QString findText;

	QStringList completerStringList = QSettings{}.value("EquipmentTabPage/SearchCompleter").toStringList();

	QString text;
	do
	{
		QInputDialog d(this);
		d.setWindowFlags((d.windowFlags() &
						~Qt::WindowMinimizeButtonHint &
						~Qt::WindowMaximizeButtonHint &
						~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint);
		d.setWindowTitle(tr("Find"));
		d.setLabelText(tr("EquipmentID (e.g. USB_RC105_CH02_MD00):"));
		d.setInputMode(QInputDialog::TextInput);
		d.setTextValue(findText);

		QCompleter* completer = new QCompleter(completerStringList, this);
		completer->setCaseSensitivity(Qt::CaseInsensitive);

		QLineEdit* lineEdit = d.findChild<QLineEdit*>();
		if (lineEdit == nullptr)
		{
			Q_ASSERT(lineEdit);
		}
		else
		{
			lineEdit->setCompleter(completer);
			connect(lineEdit, &QLineEdit::textEdited, this, [=](){completer->complete();});
			connect(completer, static_cast<void(QCompleter::*)(const QString&)>(&QCompleter::highlighted), lineEdit, &QLineEdit::setText);
		}

		int width = QSettings().value("EquipmentTabPage/findDialogWidth", 300).toInt();
		int height = QSettings().value("EquipmentTabPage/findDialogHeight", 200).toInt();
		d.resize(width, height);

		if (auto mbresult = d.exec();
			mbresult != QDialog::Accepted)
		{
			return;
		}

		QSettings().setValue("EquipmentTabPage/findDialogWidth", d.width());
		QSettings().setValue("EquipmentTabPage/findDialogHeight", d.height());

		text = d.textValue();

	} while(text.isEmpty() == true);

	if (completerStringList.contains(text, Qt::CaseInsensitive) == false)
	{
		completerStringList.push_back(text);
		while (completerStringList.size() > 50)
		{
			completerStringList.pop_front();
		}
		QSettings{}.setValue("EquipmentTabPage/SearchCompleter", completerStringList);
	}

	// --
	//
	bool found = findObject(text);
	if (found == false)
	{
		QMessageBox::information(this, qAppName(), tr("Object %1 not found.").arg(findText));
	}

	return;
}

bool EquipmentView::findObject(QString equipmentId)
{
	// If the tab page was not shown yet, this update helps to initiate loading of objects, then search will work.
	//
	equipmentModel()->updateFirstLevelObjects();

	QStringList equipmentIdFragments = equipmentId.split('_', Qt::SkipEmptyParts);
	if (equipmentIdFragments.isEmpty() == true)
	{
		return false;
	}

	QModelIndex findIndex = equipmentModel()->findObject(equipmentModel()->index(0, 0, QModelIndex()), 1/*level*/, equipmentIdFragments);

	if (findIndex.isValid() == true)
	{
		selectionModel()->clearSelection();
		selectionModel()->setCurrentIndex(findIndex, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
	}

	return findIndex.isValid();
}

std::shared_ptr<Hardware::DeviceObject> EquipmentView::deviceObject(QString equipmentId) const
{
	std::shared_ptr<Hardware::DeviceObject> result;

	// If the tab page was not shown yet, this update helps to initiate loading of objectes, then search will work.
	//
	equipmentModel()->updateFirstLevelObjects();

	QStringList equipmentIdFragments = equipmentId.split('_', Qt::SkipEmptyParts);
	if (equipmentIdFragments.isEmpty() == true)
	{
		return result;
	}

	QModelIndex findIndex = equipmentModel()->findObject(equipmentModel()->index(0, 0, QModelIndex()), 1/*level*/, equipmentIdFragments);

	if (findIndex.isValid() == true)
	{
		result = equipmentModel()->deviceObject(findIndex);
	}

	return result;
}

std::vector<std::shared_ptr<Hardware::DeviceObject>> EquipmentView::deviceObjects(QString equipmentId) const
{
	std::vector<std::shared_ptr<Hardware::DeviceObject>> result;

	// If the tab page was not shown yet, this update helps to initiate loading of objectes, then search will work.
	//
	equipmentModel()->updateFirstLevelObjects();

	QStringList equipmentIdFragments = equipmentId.split('_', Qt::SkipEmptyParts);
	if (equipmentIdFragments.isEmpty() == true)
	{
		return result;
	}

	QModelIndexList findIndexes = equipmentModel()->findObjects(equipmentModel()->index(0, 0, QModelIndex()), 1/*level*/, equipmentIdFragments);
	result.reserve(findIndexes.size());

	for (auto index : findIndexes)
	{
		result.push_back(equipmentModel()->deviceObject(index));
	}

	return result;
}

void EquipmentView::deleteSelectedDevices()
{
	QModelIndexList selected = selectionModel()->selectedRows();
	if (selected.empty())
	{
		return;
	}

	// disable sending undoChangesDeviceObject::selectionChanged, as it can be called for many objects
	//
	const QSignalBlocker blocker(selectionModel());
	Q_UNUSED(blocker);

	// perform delete
	//
	equipmentModel()->deleteDeviceObject(selected);

	// blocker will enable undoChangesDeviceObject::selectionChanged
	//
	emit updateState();

	return;
}

void EquipmentView::checkInSelectedDevices()
{
	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.empty())
	{
		return;
	}

	// disable sending undoChangesDeviceObject::selectionChanged, as it can be called for many objects
	//
	const QSignalBlocker blocker(selectionModel());
	Q_UNUSED(blocker);

	equipmentModel()->checkInDeviceObject(selected);

	// blocker will enable undoChangesDeviceObject::selectionChanged
	//

	emit updateState();
	return;
}

void EquipmentView::checkOutSelectedDevices()
{
	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.empty())
	{
		return;
	}

	equipmentModel()->checkOutDeviceObject(selected);

	emit updateState();
	return;
}

void EquipmentView::undoChangesSelectedDevices()
{
	QModelIndexList selected = selectionModel()->selectedRows();
	if (selected.empty())
	{
		return;
	}

	// disable sending undoChangesDeviceObject::selectionChanged, as it can be called for many objects
	//
	[[maybe_unused]] const QSignalBlocker blocker{selectionModel()};

	// Perform undo
	//
	equipmentModel()->undoChangesDeviceObject(selected);

	// blocker will enable undoChangesDeviceObject::selectionChanged
	//

	emit updateState();
	return;
}

void EquipmentView::undoChangesRecursively()
{
	QModelIndexList selected = selectionModel()->selectedRows();
	if (selected.size() != 1)
	{
		return;
	}

	// disable sending undoChangesDeviceObject::selectionChanged, as it can be called for many objects
	//
	[[maybe_unused]] const QSignalBlocker blocker{selectionModel()};

	// Perform undo
	//
	equipmentModel()->undoChangesRecursively(selected.front());

	// blocker will enable undoChangesDeviceObject::selectionChanged
	//

	emit updateState();
	return;
}

void EquipmentView::showHistory()
{
	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.size() != 1)
	{
		return;
	}

	// --
	//
	auto device = equipmentModel()->deviceObject(selected.front());
	if (device == nullptr)
	{
		Q_ASSERT(device);
		return;
	}

	const DbFileInfo* deviceFileInfo = device->data();
	Q_ASSERT(deviceFileInfo);

	// Get file history
	//
	std::vector<DbChangeset> fileHistory;

	bool ok = db()->getFileHistoryRecursive(*deviceFileInfo, &fileHistory, this);
	if (ok == false)
	{
		return;
	}

	// Show history dialog
	//
	FileHistoryDialog::showHistory(db(), device->equipmentId(), fileHistory, this);

	return;
}

void EquipmentView::compare()
{
	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.size() != 1)
	{
		return;
	}

	// --
	//
	auto device = equipmentModel()->deviceObject(selected.front());
	if (device == nullptr)
	{
		Q_ASSERT(device);
		return;
	}

	const DbFileInfo* deviceFileInfo = device->data();
	Q_ASSERT(deviceFileInfo);

	// --
	//
	CompareDialog::showCompare(db(), DbChangesetObject(*deviceFileInfo), -1, this);

	return;
}


void EquipmentView::refreshSelectedDevices()
{
	QModelIndexList selected = selectionModel()->selectedRows();
	equipmentModel()->refreshDeviceObject(selected);

	emit updateState();
	return;
}

void EquipmentView::updateSelectedDevices()
{
	QModelIndexList selected = selectionModel()->selectedRows();

	equipmentModel()->updateDeviceObject(selected);

	emit updateState();
	return;
}

void EquipmentView::updateFromPreset()
{
	if (isConfigurationMode() == false)
	{
		return;
	}

	// Check if some files are checked out
	//
	DbFileInfo hcFileInfo = db()->systemFileInfo(db()->systemFileId(DbDir::HardwareConfigurationDir));
	Q_ASSERT(hcFileInfo.isNull() == false);

//	std::vector<DbFileInfo> checkedOutHcFiles;

//	bool ok = db()->getCheckedOutFiles(hcFileInfo, &checkedOutHcFiles, this);
//	if (ok == false)
//	{
//		return;
//	}

//	qDebug() << "===== Checked out files =====";
//	for (const DbFileInfo& fi : checkedOutHcFiles)
//	{
//		fi.trace();
//		qDebug() << "-----------------------------";
//	}

	// Get all presets
	//
	DbFileInfo hpFileInfo = db()->systemFileInfo(DbDir::HardwarePresetsDir); 	//	hp -- stands for Hardware Presets
	Q_ASSERT(hpFileInfo.isNull() == false);

	std::shared_ptr<Hardware::DeviceObject> presetRoot;

	bool ok = db()->getDeviceTreeLatestVersion(hpFileInfo, &presetRoot, this);
	if (ok == false)
	{
		return;
	}

	Q_ASSERT(presetRoot);

	// Get All preset Roots
	//
	std::map<QString, std::shared_ptr<Hardware::DeviceObject>> presets;
	QStringList presetsToUpdate;
	bool updateAppSignals = true;

	for (int i = 0; i < presetRoot->childrenCount(); i++)
	{
		std::shared_ptr<Hardware::DeviceObject> preset = presetRoot->child(i);

		if (preset.get() == nullptr || preset->presetRoot() == false)
		{
			Q_ASSERT(preset);
			Q_ASSERT(preset->presetRoot() == true);
			continue;
		}

		if (presets.count(preset->presetName()) > 0)
		{
			QMessageBox::critical(this,
								  QApplication::applicationName(),
								  tr("There are preset with the same name %1. Preset names must be unique. Update from preset is not possible.")
										.arg(preset->presetName()));
			return;
		}

		presetsToUpdate.push_back(preset->presetName());
		presets[preset->presetName()] = preset;
	}

	presetRoot.reset();

	// Show confirmation dialog.
	//
	DialogUpdateFromPreset dialog(true/*theSettings.isExpertMode() -- ALWAYS ALLOW*/, presetsToUpdate, this);

	if (int result = dialog.exec();
		result != QDialog::Accepted)
	{
		return;
	}

	QStringList forceUpdateProperties;

	// ALWAYS ALLOW
	//if (theSettings.isExpertMode() == true)
	{
		// Get properties which must be updated even if they not meant to update
		//
		forceUpdateProperties = dialog.forceUpdateProperties();
		presetsToUpdate = dialog.selectedPresets();
		updateAppSignals = dialog.updateAppSignlas();
	}

	// Get all equipment from the database
	//
	std::shared_ptr<Hardware::DeviceObject> root;
	ok = db()->getDeviceTreeLatestVersion(hcFileInfo, &root, this);

	if (ok == false)
	{
		return;
	}

	Q_ASSERT(root);

	// Check out ont preset selected by user (presetsToUpdate).
	//
	std::vector<DbFileInfo> presetFiles;									// Files to check out
	presetFiles.reserve(65536 * 2);

	std::vector<std::shared_ptr<Hardware::DeviceObject>> presetRoots;		// Preset root objects to start update from preset operation
	presetRoots.reserve(8192);

	//

	std::function<void(std::shared_ptr<Hardware::DeviceObject>)> getPresetFiles =
		[&presetRoots, &getPresetFiles, &presetFiles, &presetsToUpdate](std::shared_ptr<Hardware::DeviceObject> object)
		{
			Q_ASSERT(object);

			if (object->isPreset() == true && 
				presetsToUpdate.contains(object->presetName()) == true)
			{
				const DbFileInfo* objectFileInfo = object->data();
				Q_ASSERT(objectFileInfo);

				presetFiles.push_back(*objectFileInfo);
			}

			if (object->isPreset() == true &&
				object->presetRoot() == true && 
				presetsToUpdate.contains(object->presetName()) == true)
			{
				presetRoots.push_back(object);
			}

			for (int i = 0; i < object->childrenCount(); i++)
			{
				getPresetFiles(object->child(i));
			}
		};

	qDebug() << "getPresetFiles(root);";
	getPresetFiles(root);

	// Result of getting preset files
	//
	qDebug() << "presetFiles<DbFileInfo>.size() " << presetFiles.size();
	qDebug() << "presetRoots<std::shared_ptr<Hardware::DeviceObject>>.size() " << presetRoots.size();

	// Check out all preset files
	//
	ok = db()->checkOut(presetFiles, this);

	if (ok == false)
	{
		// Cannot check out one or more files, update from preset is impossible
		//
		QMessageBox::critical(this,
							  QApplication::applicationName(),
							  tr("Cannot check out one or more files, update from preset is not possible."));
		return;
	}

	// All files were checked out by the current user, update preset can be performed now
	//

	// Update all preset objects
	//
	std::vector<std::shared_ptr<Hardware::DeviceObject>> updateDeviceList;
	std::vector<Hardware::DeviceObject*> deleteDeviceList;
	std::vector<std::pair<int, int>> addDeviceList;		// first: parent fileId, second: preset file id

	std::vector<const Hardware::DeviceAppSignal*> deviceSignalsToUpdateAppSignals;	// This array will be passed to application signals to
																					// to updater them

	updateDeviceList.reserve(65536 * 2);
	deleteDeviceList.reserve(65536);
	addDeviceList.reserve(65536);
	deviceSignalsToUpdateAppSignals.reserve(65536 * 2);

	for (std::shared_ptr<Hardware::DeviceObject> device : presetRoots)
	{
		if (device->presetRoot() == false)
		{
			// presetFiles contains all files from preset, update from preset is started from presetRoot objects
			//
			continue;
		}

		QString presetName = device->presetName();

		auto foundPreset = presets.find(presetName);

		if (foundPreset == presets.end())
		{
			// preset is not found
			//
			int mbResult = QMessageBox::critical(this,
												 QApplication::applicationName(),
												 tr("Preset %1 is not found.").arg(presetName),
												 QMessageBox::Ignore | QMessageBox::Cancel,
												 QMessageBox::Cancel);

			if (mbResult == QMessageBox::Ignore)
			{
				continue;
			}

			if (mbResult == QMessageBox::Cancel)
			{
				return;
			}

			Q_ASSERT(false);
			return;
		}

		// --
		//
		std::shared_ptr<Hardware::DeviceObject> preset = foundPreset->second;

		Q_ASSERT(preset->presetRoot() == true);
		Q_ASSERT(preset->presetName() == presetName);

		ok = updateDeviceFromPreset(device,
									preset,
									forceUpdateProperties,
									presetsToUpdate,
									&updateDeviceList,
									&deleteDeviceList,
									&addDeviceList,
									&deviceSignalsToUpdateAppSignals);
	}

	// save all updated data to DB
	//
	std::vector<std::shared_ptr<DbFile>> updatedFiles;
	updatedFiles.reserve(updateDeviceList.size());

	for (auto& o : updateDeviceList)
	{
		Hardware::DeviceObject* device = dynamic_cast<Hardware::DeviceObject*>(o.get());

		if (device == nullptr)
		{
			Q_ASSERT(device != nullptr);
			continue;
		}

		QByteArray data;
		ok = device->saveToByteArray(&data);

		if (ok == false)
		{
			Q_ASSERT(false);
			continue;
		}

		const DbFileInfo* deviceFileInfo = device->data();
		Q_ASSERT(deviceFileInfo);

		std::shared_ptr<DbFile> file = std::make_shared<DbFile>(*deviceFileInfo);
		file->setData(data);
		file->setDetails(device->details());

		updatedFiles.push_back(file);
	}

	db()->setWorkcopy(updatedFiles, this);

	// Delete files from DB
	//
	if (deleteDeviceList.empty() == false)
	{
		db()->deleteDeviceObjects(deleteDeviceList, this);
	}

	// Add files to DB, addPresetList contains std::pair<int, int>, first: parent file id, second preset file id
	//
	for (std::pair<int, int> ad : addDeviceList)
	{
		int parentFileId = ad.first;
		int presetFileId = ad.second;

		if (parentFileId == -1 || presetFileId == -1)
		{
			Q_ASSERT(parentFileId != -1);
			Q_ASSERT(presetFileId != -1);
			continue;
		}

		// Read preset tree from DB
		//
		std::shared_ptr<Hardware::DeviceObject> device;
		DbFileInfo presetFileInfo;
		presetFileInfo.setFileId(presetFileId);

		ok = db()->getDeviceTreeLatestVersion(presetFileInfo, &device, this);
		if (ok == false)
		{
			return;
		}

		if (device == nullptr)
		{
			Q_ASSERT(device);
			return;
		}

		const DbFileInfo* deviceFileInfo = device->data();
		Q_ASSERT(deviceFileInfo);

		if (device == nullptr ||
			device->isPreset() == false ||
			deviceFileInfo == nullptr ||
			deviceFileInfo->fileId() != presetFileInfo.fileId())	// can be not presetRoot
		{
			Q_ASSERT(device);
			Q_ASSERT(device->isPreset() == true);
			Q_ASSERT(deviceFileInfo);
			Q_ASSERT(deviceFileInfo->fileId() == presetFileInfo.fileId());
			return;
		}

		// Set new id, recursively to all children
		//
		std::function<void(Hardware::DeviceObject*)> setUuid = [&setUuid](Hardware::DeviceObject* object)
			{
				Q_ASSERT(object);

				object->setUuid(QUuid::createUuid());

				for (int i = 0; i < object->childrenCount(); i++)
				{
					setUuid(object->child(i).get());
				}
			};

		setUuid(device.get());

		// Add device to DB
		//
		bool result = db()->addDeviceObject(device.get(), parentFileId, this);
		if (result == false)
		{
			continue;
		}
	}

	// Update ApplicationSignals
	//
	if (updateAppSignals == true)
	{
		SignalsTabPage::updateSignalsSpecProps(deviceSignalsToUpdateAppSignals, forceUpdateProperties);
	}

	// Reset model
	//
	equipmentModel()->reset();
	equipmentModel()->updateUserList();

	// Done
	//
	QMessageBox::information(this, QApplication::applicationName(), tr("Update from preset done."));

	return;
}

bool EquipmentView::updateDeviceFromPreset(std::shared_ptr<Hardware::DeviceObject> device,		// Device to update from preset
										   std::shared_ptr<Hardware::DeviceObject> preset,		// Preset to update device
										   const QStringList& forceUpdateProperties,			// Update theses props even if they not meant to updayt
										   const QStringList& presetsToUpdate,					// Update only these presets
										   std::vector<std::shared_ptr<Hardware::DeviceObject>>* updateDeviceList,
										   std::vector<Hardware::DeviceObject*>* deleteDeviceList,	// Devices to delete after update
										   std::vector<std::pair<int, int>>* addDeviceList,			// Devices to add after update
										   std::vector<const Hardware::DeviceAppSignal*>* deviceSignalsToUpdateAppSignals)	// DeviceSignal list to updateA ppSignals
{
	if (updateDeviceList == nullptr ||
		deleteDeviceList == nullptr ||
		addDeviceList == nullptr ||
		deviceSignalsToUpdateAppSignals == nullptr)
	{
		Q_ASSERT(updateDeviceList);
		Q_ASSERT(deleteDeviceList);
		Q_ASSERT(addDeviceList);
		Q_ASSERT(deviceSignalsToUpdateAppSignals);
		return false;
	}

	if (device == nullptr)
	{
		Q_ASSERT(device);
		return false;
	}

	if (device->isPreset() == true &&
		(preset == nullptr ||
		device->presetName() != preset->presetName() ||
		device->presetRoot() != preset->presetRoot()))
	{
		Q_ASSERT(preset);
		Q_ASSERT(device->presetName() == preset->presetName());
		Q_ASSERT(device->presetRoot() == preset->presetRoot());
		return false;
	}

	// Update from DbVersion 380 to 381.
	//
	// Problem: Preset of Monitor (v1 to v2) has compatibility breaking changes, here we try to mitigate it.
	//			Before db project version 381 Monitor (preset version 1) had 2 properties AppDataServiceID1 and AppDataServiceID2
	//		    then this property was removed starting from the preset version 2 and changed to a single property AppDataServiceIDs
	//			which is combination of properties AppDataServiceID1 and AppDataServiceID2, but separated with semicolon (;)
	// Solution: If preset version is 1, then find properties AppDataServiceID1 and AppDataServiceID2, then after update if
	//			 property AppDataServiceIDs is present then set old values to it.
	//
	QString monitorMitigateCompatibilityAppDataService;

	if (device->presetRoot() &&
		device->presetName() == QStringLiteral("MONITOR") &&
		device->presetVersion() == 1)
	{
		auto ads1 = device->propertyByCaption(QStringLiteral("AppDataServiceID1"));
		auto ads2 = device->propertyByCaption(QStringLiteral("AppDataServiceID2"));

		QStringList adsList;

		if (ads1 != nullptr)
		{
			adsList << ads1->value().toString();
		}

		if (ads2 != nullptr)
		{
			adsList << ads2->value().toString();
		}

		monitorMitigateCompatibilityAppDataService = adsList.join(QChar(';'));
	}

	// End of Monitor fix
	//

//	qDebug();
//	qDebug() << "EquipmentView::updateDeviceFromPreset"
//			 << ", device: " << device->equipmentIdTemplate()
//			 << "(" << device->equipmentId() << ")"
//			 << ", " << device->caption()
//			 << ", place: " << device->place();

	updateDeviceList->push_back(device);

	// If it is preset, update object add/delete children
	//
	if (device->isPreset() == true)
	{
		if (presetsToUpdate.contains(device->presetName()) == true)
		{
			if (device->isAppSignal() == true)
			{
				// Collect all update DeviceSignals, so AppSignals can be update from them
				//
				deviceSignalsToUpdateAppSignals->push_back(device->toAppSignal().get());
			}

			// Update device object properties
			//
			std::vector<std::shared_ptr<Property>> deviceProperties = device->properties();
			std::vector<std::shared_ptr<Property>> presetProperties = preset->properties();

			for (auto dit = deviceProperties.begin(); dit != deviceProperties.end(); /*iterator incremented in the loop body*/)
			{
				Property* deviceProperty = dit->get();

				// Is property protected for update from preset?
				//
				if (device->presetProtectedProperties().contains(deviceProperty->caption(), Qt::CaseInsensitive) == true)
				{
					++dit;
					continue;
				}

				// --
				//
				auto pit = std::find_if(presetProperties.begin(), presetProperties.end(),
										[deviceProperty](const std::shared_ptr<Property>& preset)
										{
											return preset->caption() == deviceProperty->caption();
										});

				if (pit == presetProperties.end())
				{
					// Preset property is not found, delete this property
					//
					dit = deviceProperties.erase(dit);
					continue;
				}
				else
				{
					std::shared_ptr<Property> presetProperty = *pit;

					// Check if the property was not marked for update from preset
					// Update only limits, description, etc, not value!
					//
					if (deviceProperty->updateFromPreset() == false &&
						forceUpdateProperties.contains(deviceProperty->caption(), Qt::CaseInsensitive) == false)
					{
						deviceProperty->updateFromPreset(presetProperty.get(), false);

						++dit;
						continue;
					}

					// Update property
					//
					if (deviceProperty->isTheSameType(presetProperty.get()) == true)
					{
						deviceProperty->updateFromPreset(presetProperty.get(), true);
					}
					else
					{
						// The type is different, PropertyValue<int> <-> PropertyValue<QString>
						// Obviously thi2s is static properties
						//
						Q_ASSERT(false);
					}

					++dit;
					continue;
				}
			}

			// Check if there are any new properties in preset, the add them to device
			//
			for (auto pit = presetProperties.begin(); pit != presetProperties.end();)
			{
				Property* presetProperty = pit->get();

				auto dit = std::find_if(deviceProperties.begin(), deviceProperties.end(),
										[presetProperty](const std::shared_ptr<Property>& device)
					{
						return device->caption() == presetProperty->caption();
					});

				if (dit == deviceProperties.end())
				{
					// Preset property is not found in device, this is new property, add it
					//
					std::shared_ptr<Property> newDeviceProperty;

					if (dynamic_cast<PropertyValueNoGetterSetter*>(presetProperty) != nullptr)
					{
						newDeviceProperty = std::make_shared<PropertyValueNoGetterSetter>();
					}

					if (auto x = dynamic_cast<PropertyValue<std::vector<std::pair<QString, int>>>*>(presetProperty);
						x != nullptr)
					{
						auto enumList = x->enumValues();
						std::vector<std::pair<QString, int>> enumValues;
						enumValues.reserve(enumList.size());

						for (auto[k, s] : enumList)
						{
							enumValues.emplace_back(s, k);
						}

						newDeviceProperty = std::make_shared<PropertyValue<std::vector<std::pair<QString, int>>>>(enumValues);
					}

					Q_ASSERT(newDeviceProperty != nullptr);

					if (newDeviceProperty != nullptr)
					{
						newDeviceProperty->updateFromPreset(presetProperty, true);
						deviceProperties.push_back(newDeviceProperty);
					}

					continue;
				}

				++pit;
			}

			device->removeAllProperties();
			device->addProperties(deviceProperties);
		}

		// Update existing children, delete children
		//
		for (int i = 0; i < device->childrenCount(); i++)
		{
			std::shared_ptr<Hardware::DeviceObject> deviceChild = device->child(i);
			std::shared_ptr<Hardware::DeviceObject> presetChild = preset->child(deviceChild->presetObjectUuid());

			// WARNING, in the next condition we cannot check for presetsToUpdate, as in this case we cannot go deeper as presetChild is nullptr and
			// we will have assert in the beginning of updateDeviceFromPreset(). So deleted child will be removed from instance even though the preset
			// should not be updated.
			//
			if (deviceChild->isPreset() == true &&
				deviceChild->presetName() == device->presetName() &&
				presetChild == nullptr)
			{
				// Child was deleted from preset, add all deviceChild to delete list
				//
				std::function<void(Hardware::DeviceObject*, std::vector<Hardware::DeviceObject*>*)> deleteDevices =
						[&deleteDevices](Hardware::DeviceObject* device,std::vector<Hardware::DeviceObject*>* deleteDeviceList)
						{
							deleteDeviceList->push_back(device);

							for (int i = 0; i < device->childrenCount(); i++)
							{
								deleteDevices(device->child(i).get(), deleteDeviceList);
							}
						};

				deleteDevices(deviceChild.get(), deleteDeviceList);
				continue;
			}

			// update child
			//
			if (deviceChild->isPreset() &&
				deviceChild->presetName() == device->presetName())
			{
				updateDeviceFromPreset(deviceChild,
									   presetChild,
									   forceUpdateProperties,
									   presetsToUpdate,
									   updateDeviceList,
									   deleteDeviceList,
									   addDeviceList,
									   deviceSignalsToUpdateAppSignals);
			}
		}

		// Add children
		//
		for (int i = 0; i < preset->childrenCount(); i++)
		{
			std::shared_ptr<Hardware::DeviceObject> presetChild = preset->child(i);
			std::shared_ptr<Hardware::DeviceObject> deviceChild = device->childByPresetUuid(presetChild->presetObjectUuid());

			if (deviceChild == nullptr)
			{
				const DbFileInfo* deviceFileInfo = device->data();
				Q_ASSERT(deviceFileInfo);

				const DbFileInfo* presetChildFileInfo = presetChild->data();
				Q_ASSERT(presetChildFileInfo);

				// Child is not added yet, add it
				//
				addDeviceList->push_back(std::make_pair(deviceFileInfo->fileId(), presetChildFileInfo->fileId()));
			}
		}
	}

	// Update all non preset children
	//
	for (int i = 0; i < device->childrenCount(); i++)
	{
		std::shared_ptr<Hardware::DeviceObject> deviceChild = device->child(i);

		if (deviceChild->isPreset() == false)
		{
			updateDeviceFromPreset(deviceChild,
								   {},			// not initialized, as deviceChild is not preset
								   forceUpdateProperties,
								   presetsToUpdate,
								   updateDeviceList,
								   deleteDeviceList,
								   addDeviceList,
								   deviceSignalsToUpdateAppSignals);
		}
	}

	// Problem: Preset of Monitor has compatibility breaking changes, here we try to mitigate it.
	//
	if (monitorMitigateCompatibilityAppDataService.isEmpty() == false)
	{
		auto adsProp = device->propertyByCaption(QStringLiteral("AppDataServiceIDs"));
		if (adsProp != nullptr)
		{
			adsProp->setValue(monitorMitigateCompatibilityAppDataService);
		}
	}

	return true;
}

void EquipmentView::showEvent(QShowEvent* event)
{
	QTreeView::showEvent(event);

	if (m_requireRestoreSession == true)
	{
		restoreSession();
	}

	return;
}

void EquipmentView::focusInEvent(QFocusEvent* /*event*/)
{
	QList<QAction*> acts = actions();

	for (QAction* a : acts)
	{
		Q_ASSERT(a);

		if (a->objectName() == QLatin1String("I_am_a_Copy_Action"))
		{
			a->setShortcut(QKeySequence::Copy);
			continue;
		}

		if (a->objectName() == QLatin1String("I_am_a_Paste_Action"))
		{
			a->setShortcut(QKeySequence::Paste);
			continue;
		}

		if (a->objectName() == QLatin1String("I_am_a_Find_Action"))
		{
			a->setShortcut(QKeySequence::Find);
			continue;
		}
	}

	return;
}

void EquipmentView::focusOutEvent(QFocusEvent* event)
{
	if (event->reason() != Qt::PopupFocusReason)
	{
		QList<QAction*> acts = actions();

		for (QAction* a : acts)
		{
			Q_ASSERT(a);

			if (a->objectName() == QLatin1String("I_am_a_Copy_Action"))
			{
				a->setShortcut(QKeySequence());
				continue;
			}

			if (a->objectName() == QLatin1String("I_am_a_Paste_Action"))
			{
				a->setShortcut(QKeySequence());
				continue;
			}

			if (a->objectName() == QLatin1String("I_am_a_Find_Action"))
			{
				a->setShortcut(QKeySequence());
				continue;
			}
		}
	}

	return;
}

EquipmentModel* EquipmentView::equipmentModel()
{
	EquipmentModel* result = dynamic_cast<EquipmentModel*>(model());
	Q_ASSERT(result);
	return result;
}

EquipmentModel* EquipmentView::equipmentModel() const
{
	EquipmentModel* result = dynamic_cast<EquipmentModel*>(model());
	Q_ASSERT(result);
	return result;
}
