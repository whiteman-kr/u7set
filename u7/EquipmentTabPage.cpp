#include "Stable.h"
#include "EquipmentTabPage.h"
#include "../include/DbController.h"
#include "Settings.h"
#include "CheckInDialog.h"
#include "Subsystem.h"

#include <QtTreePropertyBrowser>
#include <QtGroupPropertyManager>
#include <QtStringPropertyManager>
#include <QtEnumPropertyManager>
#include <QtIntPropertyManager>
#include <QtDoublePropertyManager>
#include <QtBoolPropertyManager>
#include <QtSpinBoxFactory>

//
//
// EquipmentModel
//
//

EquipmentModel::EquipmentModel(DbController* dbcontroller, QWidget* parentWidget, QObject* parent) :
	QAbstractItemModel(parent),
	m_dbController(dbcontroller),
	m_parentWidget(parentWidget),
	m_configuration(std::make_shared<Hardware::DeviceRoot>()),
	m_preset(std::make_shared<Hardware::DeviceRoot>())
{
	m_root = m_configuration;	// Edit configuration default mode

	assert(dbcontroller);

	connect(dbcontroller, &DbController::projectOpened, this, &EquipmentModel::projectOpened);
	connect(dbcontroller, &DbController::projectClosed, this, &EquipmentModel::projectClosed);
}

EquipmentModel::~EquipmentModel()
{
}

QModelIndex EquipmentModel::index(int row, const QModelIndex& parentIndex) const
{
	return index(row, 0, parentIndex);
}

QModelIndex EquipmentModel::index(int row, int column, const QModelIndex& parentIndex) const
{
	if (hasIndex(row, column, parentIndex) == false)
	{
		return QModelIndex();
	}

	// Is it request for the root's items?
	//
	if (parentIndex.isValid() == false)
	{
		return createIndex(row, column, const_cast<Hardware::DeviceObject*>(m_root->child(row)));
	}

	Hardware::DeviceObject* parent = static_cast<Hardware::DeviceObject*>(parentIndex.internalPointer());

	if (parent == nullptr)
	{
		assert(parent);
		return QModelIndex();
	}

	QModelIndex resultIndex = createIndex(row, column, parent->child(row));
	return resultIndex;
}

QModelIndex EquipmentModel::parent(const QModelIndex& childIndex) const
{
	if (childIndex.isValid() == false)
	{
		return QModelIndex();
	}

	Hardware::DeviceObject* child = static_cast<Hardware::DeviceObject*>(childIndex.internalPointer());
	if (child == nullptr)
	{
		assert(child != nullptr);
		return QModelIndex();
	}

	if (child->parent() == nullptr || child->parent() == m_root.get())
	{
		return QModelIndex();
	}

	// Determine the position of the parent in the parent's parent
	//
	if (child->parent()->parent() == nullptr)
	{
		int row = m_root->childIndex(child);
		if (row == -1)
		{
			assert(row != -1);
			return QModelIndex();
		}

		return createIndex(row, 0, child->parent());
	}
	else
	{
		int row = child->parent()->parent()->childIndex(child->parent());
		if (row == -1)
		{
			assert(row != -1);
			return QModelIndex();
		}

		return createIndex(row, 0, child->parent());
	}
}

int EquipmentModel::rowCount(const QModelIndex& parentIndex) const
{
	const Hardware::DeviceObject* parent = deviceObject(parentIndex);

	if (parent == nullptr)
	{
		assert(false);
		return 0;
	}

	return parent->childrenCount();
}

int EquipmentModel::columnCount(const QModelIndex& parentIndex) const
{
	Q_UNUSED(parentIndex);
	return ColumnCount;		// Always the same
}

QVariant EquipmentModel::data(const QModelIndex& index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	Hardware::DeviceObject* device = static_cast<Hardware::DeviceObject*>(index.internalPointer());
	assert(device != nullptr);

	switch (role)
	{
	case Qt::DisplayRole:
		{
			QVariant v;

			switch (index.column())
			{
			case ObjectNameColumn:
				v.setValue<QString>(device->caption());
				break;

			case ObjectStrIdColumn:
				v.setValue<QString>(device->strId());
				break;

			case ObjectPlaceColumn:
				if (device->place() >= 0)
				{
					v.setValue<int>(device->place());
				}
				else
				{
					v.setValue<QString>("");
				}
				break;

			case ObjectStateColumn:
				{
					if (device->fileInfo().state() == VcsState::CheckedOut)
					{
						QString state = device->fileInfo().action().text();
						v.setValue<QString>(state);
					}
				}
				break;

			case ObjectUserColumn:
				if (device->fileInfo().state() == VcsState::CheckedOut)
				{
					v.setValue<qint32>(device->fileInfo().userId());
				}
				break;

			default:
				assert(false);
			}

			return v;
		}
		break;

	case Qt::TextAlignmentRole:
		{
			return Qt::AlignLeft + Qt::AlignVCenter;
		}
		break;

	case Qt::BackgroundRole:
		{
			if (device->fileInfo().state() == VcsState::CheckedOut)
			{
				QBrush b(QColor(0xFF, 0xFF, 0xFF));

				switch (static_cast<VcsItemAction::VcsItemActionType>(device->fileInfo().action().toInt()))
				{
				case VcsItemAction::Added:
					b.setColor(QColor(0xF9, 0xFF, 0xF9));
					break;
				case VcsItemAction::Modified:
					b.setColor(QColor(0xF4, 0xFA, 0xFF));
					break;
				case VcsItemAction::Deleted:
					b.setColor(QColor(0xFF, 0xF4, 0xF4));
					break;
				}

				return b;
			}
		}
		break;
	}

	return QVariant();
}

QVariant EquipmentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal) {
			switch (section)
			{
			case ObjectNameColumn:
				return QObject::tr("Object");

			case ObjectStrIdColumn:
				return QObject::tr("StrId");

				case ObjectPlaceColumn:
					return QObject::tr("Place");

			case ObjectStateColumn:
				return QObject::tr("State");

			case ObjectUserColumn:
				return QObject::tr("User");

			default:
				assert(false);
			}
		}
	}

	return QVariant();
}

bool EquipmentModel::hasChildren(const QModelIndex& parentIndex) const
{
	if (dbController()->isProjectOpened() == false)
	{
		return false;
	}

	const Hardware::DeviceObject* object = deviceObject(parentIndex);

	if (object->childrenCount() > 0)
	{
		return true;	// seems that we already got file list for this object
	}

	if (object->deviceType() == Hardware::DeviceType::Signal)
	{
		return false;	// DeviceType::DiagSignal cannot have children
	}

	bool hasChildren = false;
	DbFileInfo fi = object->fileInfo();

	bool result = dbController()->fileHasChildren(&hasChildren, fi, m_parentWidget);
	if (result == false)
	{
		return false;
	}

	return hasChildren;
}

bool EquipmentModel::canFetchMore(const QModelIndex& parent) const
{
	if (dbController()->isProjectOpened() == false)
	{
		return false;
	}

	const Hardware::DeviceObject* object = deviceObject(parent);

	if (object->childrenCount() > 0)
	{
		return false;	// seems that we already got file list for this object
	}

	if (object->deviceType() == Hardware::DeviceType::Signal)
	{
		return false;	// DeviceType::DiagSignal cannot have children
	}

	bool hasChildren = false;
	DbFileInfo fi = object->fileInfo();

	bool result = dbController()->fileHasChildren(&hasChildren, fi, m_parentWidget);

	if (result == false)
	{
		return false;
	}

	return hasChildren;
}

void EquipmentModel::fetchMore(const QModelIndex& parentIndex)
{
	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	Hardware::DeviceObject* parentObject = deviceObject(const_cast<QModelIndex&>(parentIndex));

	std::vector<DbFileInfo> files;

	bool ok = dbController()->getFileList(&files, parentObject->fileInfo().fileId(), m_parentWidget);
	if (ok == false)
		return;

	beginInsertRows(parentIndex, 0, static_cast<int>(files.size()) - 1);

	parentObject->deleteAllChildren();

	for (auto& fi : files)
	{
		std::shared_ptr<DbFile> file;

		dbController()->getLatestVersion(fi, &file, m_parentWidget);
		if (file == nullptr)
		{
			continue;
		}

		Hardware::DeviceObject* object = Hardware::DeviceObject::Create(file->data());
		assert(object);

		if (object == nullptr)
		{
			continue;
		}

		object->setFileInfo(fi);

		std::shared_ptr<Hardware::DeviceObject> sp(object);
		parentObject->addChild(sp);
	}

	sortDeviceObject(parentObject, m_sortColumn, m_sortOrder);

	endInsertRows();

	return;
}

void EquipmentModel::sortDeviceObject(Hardware::DeviceObject* object, int column, Qt::SortOrder order)
{
	if (object == nullptr)
	{
		assert(object);
		return;
	}

	switch (column)
	{
	case ObjectNameColumn:
		object->sortByCaption(order);
		break;
	case ObjectStrIdColumn:
		object->sortByStrId(order);
		break;
	case ObjectPlaceColumn:
		object->sortByPlace(order);
		break;
	case ObjectStateColumn:
		object->sortByState(order);
		break;
	case ObjectUserColumn:
		object->sortByUser(order);
		break;
	default:
		assert(false);
	}

	int childCont = object->childrenCount();

	for (int i = 0; i < childCont; i++)
	{
		Hardware::DeviceObject* child = object->child(i);

		if (child->deviceType() != Hardware::DeviceType::Signal)
		{
			sortDeviceObject(child, column, order);
		}
	}

	return;
}

void EquipmentModel::sort(int column, Qt::SortOrder order/* = Qt::AscendingOrder*/)
{
	qDebug() << "Sort column: " << column << ", order: " << static_cast<int>(order);

	m_sortColumn = column;
	m_sortOrder = order;

	emit layoutAboutToBeChanged();
	QModelIndexList pers = persistentIndexList();

	// Sort
	//
	sortDeviceObject(m_configuration.get(), column, order);
	sortDeviceObject(m_preset.get(), column, order);

	// Move pers indexes
	//
	for (QModelIndex& oldIndex : pers)
	{
		Hardware::DeviceObject* device = deviceObject(oldIndex);
		assert(device);

		Hardware::DeviceObject* parentDevice = device->parent();
		assert(parentDevice);

		QModelIndex newIndex = index(parentDevice->childIndex(device), oldIndex.column(), oldIndex.parent());

		if (oldIndex != newIndex)
		{
			changePersistentIndex(oldIndex, newIndex);
		}
	}

	emit layoutChanged();

	return;
}

bool EquipmentModel::insertDeviceObject(std::shared_ptr<Hardware::DeviceObject> object, QModelIndex parentIndex)
{
	// TODO: This function should take into consideration sort property!!!
	//

	Hardware::DeviceObject* parent = deviceObject(parentIndex);

	beginInsertRows(parentIndex, parent->childrenCount(), parent->childrenCount());
	parent->addChild(object);
	endInsertRows();

	return true;
}

void EquipmentModel::deleteDeviceObject(QModelIndexList& rowList)
{
	std::vector<Hardware::DeviceObject*> devices;

	for (QModelIndex& index : rowList)
	{
		Hardware::DeviceObject* d = deviceObject(index);
		assert(d);

		devices.push_back(d);
	}

	bool result = dbController()->deleteDeviceObjects(devices, m_parentWidget);
	if (result == false)
	{
		return;
	}

	// Update model
	//
	for (QModelIndex& index : rowList)
	{
		Hardware::DeviceObject* d = deviceObject(index);
		assert(d);

		if (d->fileInfo().deleted() == true)
		{
			QModelIndex pi = index.parent();
			Hardware::DeviceObject* po = deviceObject(pi);
			assert(po);

			int childIndex = po->childIndex(d);
			assert(childIndex != -1);

			beginRemoveRows(pi, childIndex, childIndex);
			po->deleteChild(d);
			endRemoveRows();
		}
		else
		{
			emit dataChanged(index, index);
		}
	}

	return;
}

void EquipmentModel::checkInDeviceObject(QModelIndexList& rowList)
{
	std::vector<DbFileInfo> files;
	QModelIndexList checkedOutList;
	DbUser currentUser = dbController()->currentUser();

	for (QModelIndex& index : rowList)
	{
		Hardware::DeviceObject* d = deviceObject(index);
		assert(d);

		if (d->fileInfo().state() == VcsState::CheckedOut &&
			(d->fileInfo().userId() == currentUser.userId() || currentUser.isAdminstrator() == true))
		{
			files.push_back(d->fileInfo());
			checkedOutList.push_back(index);
		}
	}

	CheckInDialog::checkIn(files, dbController(), m_parentWidget);

	// Update FileInfo in devices and Update model
	//
	for (QModelIndex& index : rowList)
	{
		Hardware::DeviceObject* d = deviceObject(index);
		assert(d);

		for (const auto& fi : files)
		{
			if (fi.fileId() == d->fileInfo().fileId())
			{
				d->setFileInfo(fi);

				if (d->fileInfo().deleted() == true ||
					(d->fileInfo().action() == VcsItemAction::Deleted && d->fileInfo().state() == VcsState::CheckedIn))
				{
					QModelIndex pi = index.parent();
					Hardware::DeviceObject* po = deviceObject(pi);
					assert(po);

					int childIndex = po->childIndex(d);
					assert(childIndex != -1);

					beginRemoveRows(pi, childIndex, childIndex);
					po->deleteChild(d);
					endRemoveRows();
				}
				else
				{
					emit dataChanged(index, index);
				}

				break;
			}
		}
	}

	return;
}

void EquipmentModel::checkOutDeviceObject(QModelIndexList& rowList)
{
	std::vector<DbFileInfo> files;
	QModelIndexList checkedInList;
	DbUser currentUser = dbController()->currentUser();

	for (QModelIndex& index : rowList)
	{
		Hardware::DeviceObject* d = deviceObject(index);
		assert(d);

		if (d->fileInfo().state() == VcsState::CheckedIn)
		{
			files.push_back(d->fileInfo());
			checkedInList.push_back(index);
		}
	}

	dbController()->checkOut(files, m_parentWidget);

	// Update FileInfo in devices and Update model
	//
	for (QModelIndex& index : rowList)
	{
		Hardware::DeviceObject* d = deviceObject(index);
		assert(d);

		for (const auto& fi : files)
		{
			if (fi.fileId() == d->fileInfo().fileId())
			{
				d->setFileInfo(fi);						// Update file info record in the DeviceOubject

				if (d->fileInfo().state() == VcsState::CheckedOut)
				{
					emit dataChanged(index, index);		// Notify view about data update
				}

				break;
			}
		}
	}

	return;
}

void EquipmentModel::undoChangesDeviceObject(QModelIndexList& rowList)
{
	std::vector<DbFileInfo> files;
	QModelIndexList checkedOutList;
	DbUser currentUser = dbController()->currentUser();

	for (QModelIndex& index : rowList)
	{
		Hardware::DeviceObject* d = deviceObject(index);
		assert(d);

		if (d->fileInfo().state() == VcsState::CheckedOut &&
			(d->fileInfo().userId() == currentUser.userId() || currentUser.isAdminstrator() == true))
		{
			files.push_back(d->fileInfo());
			checkedOutList.push_back(index);
		}
	}

	auto mb = QMessageBox::question(
		m_parentWidget,
		tr("Undo Changes"),
		tr("Do you want to undo pending changes? All selected objects' changes will be lost!"));

	if (mb == QMessageBox::No)
	{
		return;
	}

	dbController()->undoChanges(files, m_parentWidget);

	// Update FileInfo in devices and Update model
	//
	for (QModelIndex& index : rowList)
	{
		Hardware::DeviceObject* d = deviceObject(index);
		assert(d);

		bool updated = false;
		for (DbFileInfo& fi : files)
		{
			if (fi.fileId() == d->fileInfo().fileId())
			{
				d->setFileInfo(fi);

				if (fi.deleted() == true)
				{
					QModelIndex pi = index.parent();
					Hardware::DeviceObject* po = deviceObject(pi);
					assert(po);

					int childIndex = po->childIndex(d);
					assert(childIndex != -1);

					beginRemoveRows(pi, childIndex, childIndex);
					po->deleteChild(d);
					endRemoveRows();
				}
				else
				{
					emit dataChanged(index, index);
				}

				updated = true;
				break;
			}
		}
		assert(updated == true);
	}

	return;
}

void EquipmentModel::refreshDeviceObject(QModelIndexList& rowList)
{
	if (rowList.isEmpty() == true)
	{
		// Refresh all model
		//
		beginResetModel();
		m_root->deleteAllChildren();
		endResetModel();
		return;
	}

	// Refresh selected indexes
	//
	for (QModelIndex& index : rowList)
	{
		Hardware::DeviceObject* d = deviceObject(index);
		assert(d);

		if (d->childrenCount() > 0)
		{
			beginRemoveRows(index, 0, d->childrenCount() - 1);
			d->deleteAllChildren();
			endRemoveRows();

			emit dataChanged(index, index);
		}
	}

	return;
}

void EquipmentModel::updateDeviceObject(QModelIndexList& rowList)
{
	if (rowList.isEmpty() == true)
	{
		return;
	}

	// Update data
	//
	for (QModelIndex& i : rowList)
	{
		QModelIndex bottomRight = index(i.row(), columnCount() - 1, i.parent());

		emit dataChanged(i, bottomRight);
	}

	// Sort items
	//
	emit layoutAboutToBeChanged();

	QModelIndexList pers = persistentIndexList();

	// --
	//
	std::set<Hardware::DeviceObject*> updatedParents;

	for (QModelIndex& i : rowList)
	{
		QModelIndex parentIndex = i.parent();

		Hardware::DeviceObject* parentDevice = deviceObject(parentIndex);
		assert(parentDevice);

		if (updatedParents.count(parentDevice) == 0)
		{
			updatedParents.insert(parentDevice);
			sortDeviceObject(parentDevice, m_sortColumn, m_sortOrder);
		}
	}

	// --
	//

	for (QModelIndex& oldIndex : pers)
	{
		Hardware::DeviceObject* device = deviceObject(oldIndex);
		assert(device);

		Hardware::DeviceObject* parentDevice = device->parent();
		assert(parentDevice);

		QModelIndex newIndex = index(parentDevice->childIndex(device), oldIndex.column(), oldIndex.parent());

		if (oldIndex != newIndex)
		{
			changePersistentIndex(oldIndex, newIndex);
		}
	}

	emit layoutChanged();

	return;
}

Hardware::DeviceObject* EquipmentModel::deviceObject(QModelIndex& index)
{
	Hardware::DeviceObject* object = nullptr;

	if (index.isValid() == false)
	{
		object = m_root.get();
	}
	else
	{
		object = static_cast<Hardware::DeviceObject*>(index.internalPointer());
	}

	assert(object != nullptr);
	return object;
}

const Hardware::DeviceObject* EquipmentModel::deviceObject(const QModelIndex& index) const
{
	const Hardware::DeviceObject* object = nullptr;

	if (index.isValid() == false)
	{
		object = m_root.get();
	}
	else
	{
		object = static_cast<const Hardware::DeviceObject*>(index.internalPointer());
	}

	assert(object != nullptr);
	return object;
}

std::shared_ptr<Hardware::DeviceObject> EquipmentModel::deviceObjectSharedPtr(QModelIndex& index)
{
	Hardware::DeviceObject* rawPtr = nullptr;

	if (index.isValid() == false)
	{
		rawPtr = m_root.get();
	}
	else
	{
		rawPtr = static_cast<Hardware::DeviceObject*>(index.internalPointer());
	}

	if (rawPtr == nullptr)
	{
		assert(rawPtr);
		return std::shared_ptr<Hardware::DeviceObject>();
	}

	if (rawPtr->parent() == nullptr)
	{
		assert(rawPtr->parent() != nullptr);
		return std::shared_ptr<Hardware::DeviceObject>();
	}

	int childIndex = rawPtr->parent()->childIndex(rawPtr);
	if (childIndex == -1)
	{
		assert(childIndex != -1);
		return std::shared_ptr<Hardware::DeviceObject>();
	}

	std::shared_ptr<Hardware::DeviceObject> object = rawPtr->parent()->childSharedPtr(childIndex);

	assert(object != nullptr);
	return object;
}

void EquipmentModel::projectOpened()
{
	beginResetModel();

	m_configuration->fileInfo().setFileId(dbController()->hcFileId());
	m_preset->fileInfo().setFileId(dbController()->hpFileId());

	endResetModel();

	return;
}

void EquipmentModel::projectClosed()
{
	// Release all children
	//
	beginResetModel();

	m_configuration->deleteAllChildren();
	m_preset->deleteAllChildren();

	endResetModel();

	return;
}

void EquipmentModel::switchMode()
{
	beginResetModel();

	if (isPresetMode() == true)
	{
		m_root = m_configuration;
	}
	else
	{
		m_root = m_preset;
	}

	endResetModel();
}

DbController* EquipmentModel::dbController()
{
	return m_dbController;
}

DbController* EquipmentModel::dbController() const
{
	return m_dbController;
}

bool EquipmentModel::isPresetMode() const
{
	return m_root == m_preset;
}

bool EquipmentModel::isConfigurationMode() const
{
	return m_root == m_configuration;
}

//
//
// EquipmentView
//
//
EquipmentView::EquipmentView(DbController* dbcontroller) :
	m_dbController(dbcontroller)
{
	assert(m_dbController);

	setSortingEnabled(true);

	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);

	setUniformRowHeights(true);
	setIndentation(10);

	sortByColumn(EquipmentModel::Columns::ObjectPlaceColumn, Qt::SortOrder::AscendingOrder);

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

void EquipmentView::addSystem()
{
	// Add new system to the root
	//
	std::shared_ptr<Hardware::DeviceObject> system = std::make_shared<Hardware::DeviceSystem>(isPresetMode());

	system->setStrId("SYSTEMID");
	system->setCaption(tr("System"));

	addDeviceObject(system);
	return;
}

void EquipmentView::addRack()
{
	std::shared_ptr<Hardware::DeviceObject> rack = std::make_shared<Hardware::DeviceRack>(isPresetMode());

	rack->setStrId("$(PARENT)_RACKID");
	rack->setCaption(tr("Rack"));

	addDeviceObject(rack);
	return;
}

void EquipmentView::addChassis()
{
	std::shared_ptr<Hardware::DeviceObject> chassis = std::make_shared<Hardware::DeviceChassis>(isPresetMode());

    chassis->setStrId("$(PARENT)_CHASSISID");
    chassis->setCaption(tr("Chassis"));

    addDeviceObject(chassis);
	return;
}

void EquipmentView::addModule()
{
	std::shared_ptr<Hardware::DeviceObject> module = std::make_shared<Hardware::DeviceModule>(isPresetMode());

	module->setStrId("$(PARENT)_MD$(PLACE)");
	module->setCaption(tr("Module"));

	addDeviceObject(module);
	return;
}

void EquipmentView::addController()
{
	std::shared_ptr<Hardware::DeviceObject> controller = std::make_shared<Hardware::DeviceController>(isPresetMode());

	controller->setStrId("$(PARENT)_CTRLXXX");
	controller->setCaption(tr("Controller"));

	addDeviceObject(controller);
	return;
}

void EquipmentView::addSignal()
{
	std::shared_ptr<Hardware::DeviceObject> signal = std::make_shared<Hardware::DeviceSignal>(isPresetMode());

	signal->setStrId("$(PARENT)_SIGNAL");
	signal->setCaption(tr("Signal"));

	addDeviceObject(signal);
	return;
}

void EquipmentView::addWorkstation()
{
	std::shared_ptr<Hardware::DeviceObject> workstation = std::make_shared<Hardware::Workstation>(isPresetMode());

	workstation->setStrId("$(PARENT)_WS$(PLACE)");
	workstation->setCaption(tr("Workstation"));

	addDeviceObject(workstation);
	return;
}

void EquipmentView::addSoftware()
{
	std::shared_ptr<Hardware::DeviceObject> software = std::make_shared<Hardware::Software>(isPresetMode());

	software->setStrId("$(PARENT)_SWNAME");
	software->setCaption(tr("Software"));

	addDeviceObject(software);
	return;
}


void EquipmentView::addPresetRack()
{
	if (isPresetMode() == true)
	{
		std::shared_ptr<Hardware::DeviceObject> rack = std::make_shared<Hardware::DeviceRack>(true);

		rack->setStrId("$(PARENT)_RACKID");
		rack->setCaption(tr("Rack"));

		rack->setPresetRoot(true);
		rack->setPresetName("PRESET_NAME");

		addDeviceObject(rack);
	}
	else
	{
		choosePreset(Hardware::DeviceType::Rack);
	}
	return;
}

void EquipmentView::addPresetChassis()
{
	if (isPresetMode() == true)
	{
        std::shared_ptr<Hardware::DeviceObject> chassis = std::make_shared<Hardware::DeviceChassis>(true);

        chassis->setStrId("$(PARENT)_CHASSISID");
        chassis->setCaption(tr("Chassis"));

        chassis->setPresetRoot(true);
        chassis->setPresetName("PRESET_NAME");

        addDeviceObject(chassis);
	}
	else
	{
		choosePreset(Hardware::DeviceType::Chassis);
	}
}

void EquipmentView::addPresetModule()
{
	if (isPresetMode() == true)
	{
		std::shared_ptr<Hardware::DeviceObject> module = std::make_shared<Hardware::DeviceModule>(true);

		module->setStrId("$(PARENT)_MD00");
		module->setCaption(tr("Module"));

		module->setPresetRoot(true);
		module->setPresetName("PRESET_NAME");

		addDeviceObject(module);
	}
	else
	{
		choosePreset(Hardware::DeviceType::Module);
	}
}

void EquipmentView::addPresetController()
{
	if (isPresetMode() == true)
	{
		std::shared_ptr<Hardware::DeviceObject> controller = std::make_shared<Hardware::DeviceController>(true);

		controller->setStrId("$(PARENT)_CRRLXXX");
		controller->setCaption(tr("Controller"));

		controller->setPresetRoot(true);
		controller->setPresetName("PRESET_NAME");

		addDeviceObject(controller);
	}
	else
	{
		choosePreset(Hardware::DeviceType::Controller);
	}
}

void EquipmentView::addPresetWorkstation()
{
	if (isPresetMode() == true)
	{
		std::shared_ptr<Hardware::DeviceObject> workstation = std::make_shared<Hardware::Workstation>(true);

		workstation->setStrId("$(PARENT)_WS00");
		workstation->setCaption(tr("Workstation"));

		workstation->setPresetRoot(true);
		workstation->setPresetName("PRESET_NAME");

		addDeviceObject(workstation);
	}
	else
	{
		choosePreset(Hardware::DeviceType::Workstation);
	}
}

void EquipmentView::addPresetSoftware()
{
	if (isPresetMode() == true)
	{
		std::shared_ptr<Hardware::DeviceObject> software = std::make_shared<Hardware::Software>(true);

		software->setStrId("$(PARENT)_SWNAME");
		software->setCaption(tr("Software"));

		software->setPresetRoot(true);
		software->setPresetName("PRESET_NAME");

		addDeviceObject(software);
	}
	else
	{
		choosePreset(Hardware::DeviceType::Software);
	}
}


void EquipmentView::choosePreset(Hardware::DeviceType type)
{
	if (isConfigurationMode() == false)
	{
		assert(isConfigurationMode() == true);
		return;
	}

	// Get file list
	//
	std::vector<DbFileInfo> fileList;

	bool ok = db()->getFileList(
				&fileList,
				db()->hpFileId(),
				Hardware::DeviceObject::fileExtension(type),
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
		std::shared_ptr<Hardware::DeviceObject> object(Hardware::DeviceObject::fromDbFile(*f));
		assert(object != nullptr);

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
				addPresetToConfiguration(p->fileInfo());
			});

		menu->addAction(a);
	}

	menu->exec(this->cursor().pos());

	return;
}

void EquipmentView::addPresetToConfiguration(const DbFileInfo& fileInfo)
{
	assert(fileInfo.fileId() != -1);
	assert(fileInfo.parentId() != -1);
	assert(fileInfo.parentId() == db()->hpFileId());

	// Read all preset tree and add it to the hardware configuration
	//
	std::shared_ptr<Hardware::DeviceObject> device;

	bool ok = db()->getDeviceTreeLatestVersion(fileInfo, &device, this);
	if (ok == false)
	{
		return;
	}

	if (device->fileInfo().fileId() != fileInfo.fileId() ||
		device->presetRoot() == false)
	{
		assert(device->fileInfo().fileId() == fileInfo.fileId());
		assert(device->presetRoot() == true);
		return;
	}

	// If this is LM modlue, then set SusbSysID to the default value
	//
	if (device->isModule() == true)
	{
		Hardware::DeviceModule* module = device->toModule();

		if (module != nullptr && module->moduleFamily() == Hardware::DeviceModule::LM)
		{
			// Get susbsystems
			//
			Hardware::SubsystemStorage subsystems;
			QString errorCode;

			if (subsystems.load(db(), errorCode) == true && subsystems.count() > 0)
			{
				std::shared_ptr<Hardware::Subsystem> subsystem = subsystems.get(0);

				module->setSubSysID(subsystem->strId());
			}
		}
	}

	// Reset fileInfo in all objects
	//

	// Add new device
	//

	addDeviceObject(device);
	return;
}

void EquipmentView::addDeviceObject(std::shared_ptr<Hardware::DeviceObject> object)
{
	if (object == nullptr)
	{
		assert(object != nullptr);
		return;
	}

	if (isPresetMode() == true && object->preset() == false)
	{
		assert(false);
		return;
	}

	Hardware::DeviceObject* parentObject = nullptr;
	QModelIndex parentIndex;	// Currently it is root;

	if (isPresetMode() == true &&
		object->preset() == true &&
		object->presetRoot() == true)
	{
		parentObject = equipmentModel()->deviceObject(parentIndex);
	}
	else
	{
		QModelIndexList selected = selectionModel()->selectedRows();

		if (selected.size() > 1)
		{
			// Don't know after which item insrt new object
			//
			return;
		}

		if (selected.empty() == false)
		{
			parentIndex = selected[0];
		}

		// --
		//
		parentObject = equipmentModel()->deviceObject(parentIndex);
		assert(parentObject);

		if (object->deviceType() == Hardware::DeviceType::Software &&
			(parentObject->deviceType() != Hardware::DeviceType::Workstation && parentObject->deviceType() != Hardware::DeviceType::Software))
		{
			assert(false);
			return;
		}

		if (parentObject->deviceType() == object->deviceType())
		{
			// add the same item to the end of the the parent
			//
			parentIndex = parentIndex.parent();
			parentObject = equipmentModel()->deviceObject(parentIndex);

			assert(parentObject->deviceType() < object->deviceType());
		}

		if (parentObject->deviceType() > object->deviceType() ||
			(object->deviceType() == Hardware::DeviceType::Workstation && parentObject->deviceType() > Hardware::DeviceType::Chassis))
		{
			assert(parentObject->deviceType() <= object->deviceType());
			return;
		}
	}

	// Debugging .... parentObject->setChildRestriction("function(device) { return device.Place >=0 && device.Place < 16; }");

	QString errorMessage;
	bool allowed = parentObject->checkChild(object.get(), &errorMessage);

	if (allowed == false)
	{
		QMessageBox::critical(this, tr("Error"), tr("It is not allowed to add child, error message: %1").arg(errorMessage));
		return;
	}

	// Add device to DB
	//
	assert(parentObject != nullptr);

	bool result = db()->addDeviceObject(object.get(), parentObject->fileInfo().fileId(), this);

	if (result == false)
	{
		return;
	}

	// Add new device to the model and select it
	//
	equipmentModel()->insertDeviceObject(object, parentIndex);

	QModelIndex objectModelIndex = equipmentModel()->index(parentObject->childIndex(object.get()), parentIndex);

	selectionModel()->clearSelection();
	selectionModel()->select(objectModelIndex, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
	setCurrentIndex(objectModelIndex);

	return;
}

void EquipmentView::addInOutsToSignals()
{
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() != 1)
	{
		assert(false);	// how did we get here?
		return;
	}

	Hardware::DeviceObject* device = equipmentModel()->deviceObject(selectedIndexList.front());
	assert(device);

	Hardware::DeviceModule* module = dynamic_cast<Hardware::DeviceModule*>(device);

	if (module == nullptr || module->isIOModule() == false)
	{
		assert(module);
		return;
	}

	// Get module from the DB as here it can be not fully loaded
	//
	std::shared_ptr<Hardware::DeviceObject> dbModule;
	bool ok = db()->getDeviceTreeLatestVersion(module->fileInfo(), &dbModule, this);

	if (ok == false)
	{
		return;
	}

	// Get all hardware inputs outputs from the module
	//
	std::vector<Hardware::DeviceSignal*> inOuts;

	std::function<void(Hardware::DeviceObject*)> getInOuts =
		[&inOuts, &getInOuts](Hardware::DeviceObject* device)
		{
			if (device->deviceType() == Hardware::DeviceType::Signal)
			{
				Hardware::DeviceSignal* signal = dynamic_cast<Hardware::DeviceSignal*>(device);
				assert(signal);

				if (signal->function() == Hardware::DeviceSignal::SignalFunction::Input ||
					signal->function() == Hardware::DeviceSignal::SignalFunction::Output ||
					signal->function() == Hardware::DeviceSignal::SignalFunction::Validity)
				{
					inOuts.push_back(signal);
				}

				return;
			}

			for (int i = 0; i < device->childrenCount(); i++)
			{
				getInOuts(device->child(i));
			}
		};

	getInOuts(dbModule.get());

	if (inOuts.empty() == true)
	{
		return;
	}

	// Expand StrID for signals,
	// track aprents from the module, and children from the dbModuke
	//
	std::list<std::shared_ptr<Hardware::DeviceObject>> equipmentDevices;

	Hardware::DeviceObject* equipmentDevice = module;
	while (equipmentDevice != nullptr)
	{
		if (equipmentDevice != module)
		{
			QByteArray bytes;

			equipmentDevice->Save(bytes);	// save and restore to keep equpment version after expanding strid

			Hardware::DeviceObject* newObject = Hardware::DeviceObject::Create(bytes);
			std::shared_ptr<Hardware::DeviceObject> newObjectSp(newObject);

			equipmentDevices.push_front(newObjectSp);
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

		equipmentDevices.front()->expandStrId();
	}

	dbModule->expandStrId();	// StrIds in getInOuts will be updated also

	// Add signals to the project DB
	//

	std::sort(std::begin(inOuts), std::end(inOuts),
		[](Hardware::DeviceObject* a, Hardware::DeviceObject* b)
		{
			return a->strId() < b->strId();
		});

	db()->autoAddSignals(&inOuts, this);

	return;
}


void EquipmentView::deleteSelectedDevices()
{
	QModelIndexList selected = selectionModel()->selectedRows();
	if (selected.empty())
	{
		return;
	}

	// --
	//
	equipmentModel()->deleteDeviceObject(selected);

	return;
}

void EquipmentView::checkInSelectedDevices()
{
	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.empty())
	{
		return;
	}

	equipmentModel()->checkInDeviceObject(selected);
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
	return;
}

void EquipmentView::undoChangesSelectedDevices()
{
	QModelIndexList selected = selectionModel()->selectedRows();

	if (selected.empty())
	{
		return;
	}

	equipmentModel()->undoChangesDeviceObject(selected);
	return;
}

void EquipmentView::refreshSelectedDevices()
{
	QModelIndexList selected = selectionModel()->selectedRows();
	equipmentModel()->refreshDeviceObject(selected);
	return;
}

void EquipmentView::updateSelectedDevices()
{
	QModelIndexList selected = selectionModel()->selectedRows();

	equipmentModel()->updateDeviceObject(selected);

	return;
}

EquipmentModel* EquipmentView::equipmentModel()
{
	EquipmentModel* result = dynamic_cast<EquipmentModel*>(model());
	assert(result);
	return result;
}

EquipmentModel* EquipmentView::equipmentModel() const
{
	EquipmentModel* result = dynamic_cast<EquipmentModel*>(model());
	assert(result);
	return result;
}

//
//
// EquipmentTabPage
//
//
EquipmentTabPage::EquipmentTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	assert(dbcontroller != nullptr);

	//
	// Controls
	//


	// Equipment View
	//
	m_equipmentView = new EquipmentView(dbcontroller);
	m_equipmentModel = new EquipmentModel(dbcontroller, this, this);
	m_equipmentView->setModel(m_equipmentModel);

	// Create Actions
	//
	CreateActions();

	// Set context menu to Equipment View
	//
	m_equipmentView->setContextMenuPolicy(Qt::ActionsContextMenu);

	// -----------------
	m_equipmentView->addAction(m_addObjectAction);

		m_addObjectMenu->addAction(m_addSystemAction);
		m_addObjectMenu->addAction(m_addRackAction);
		m_addObjectMenu->addAction(m_addChassisAction);
		m_addObjectMenu->addAction(m_addModuleAction);
		m_addObjectMenu->addAction(m_addControllerAction);
		m_addObjectMenu->addAction(m_addSignalAction);
		m_addObjectMenu->addAction(m_addWorkstationAction);
		m_addObjectMenu->addAction(m_addSoftwareAction);


	m_equipmentView->addAction(m_addPresetAction);

		m_addPresetMenu->addAction(m_addPresetRackAction);
		m_addPresetMenu->addAction(m_addPresetChassisAction);
		m_addPresetMenu->addAction(m_addPresetModuleAction);
		m_addPresetMenu->addAction(m_addPresetControllerAction);
		m_addPresetMenu->addAction(m_addPresetWorkstationAction);
		m_addPresetMenu->addAction(m_addPresetSoftwareAction);

	m_equipmentView->addAction(m_inOutsToSignals);

	// -----------------
	m_equipmentView->addAction(m_SeparatorAction1);
	m_equipmentView->addAction(m_deleteObjectAction);
	// -----------------
	m_equipmentView->addAction(m_SeparatorAction2);
	m_equipmentView->addAction(m_checkOutAction);
	m_equipmentView->addAction(m_checkInAction);
	m_equipmentView->addAction(m_undoChangesAction);
	m_equipmentView->addAction(m_refreshAction);
	// -----------------
	m_equipmentView->addAction(m_SeparatorAction3);
	m_equipmentView->addAction(m_switchMode);
	// -----------------
	//m_equipmentView->addAction(m_SeparatorAction4);
	//m_equipmentView->addAction(m_moduleConfigurationAction);

	// Property View
	//

	// Splitter
	//
	m_splitter = new QSplitter(this);

	m_propertyEditor = new ExtWidgets::PropertyEditor(m_splitter);

	m_splitter->addWidget(m_equipmentView);
	m_splitter->addWidget(m_propertyEditor);


	m_splitter->setStretchFactor(0, 2);
	m_splitter->setStretchFactor(1, 1);

	m_splitter->restoreState(theSettings.m_equipmentTabPageSplitterState);

	//
	// Layouts
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout();

	pMainLayout->addWidget(m_splitter);

	setLayout(pMainLayout);

	// --
	//
	connect(dbController(), &DbController::projectOpened, this, &EquipmentTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &EquipmentTabPage::projectClosed);

	connect(m_equipmentView->selectionModel(), & QItemSelectionModel::selectionChanged, this, &EquipmentTabPage::selectionChanged);
	connect(m_equipmentModel, &EquipmentModel::dataChanged, this, &EquipmentTabPage::modelDataChanged);

	connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &EquipmentTabPage::propertiesChanged);


	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

EquipmentTabPage::~EquipmentTabPage()
{
	theSettings.m_equipmentTabPageSplitterState = m_splitter->saveState();
	theSettings.writeUserScope();
}

void EquipmentTabPage::CreateActions()
{
	//-------------------------------
	m_addObjectMenu = new QMenu(this);

	m_addObjectAction = new QAction(tr("Add Object"), this);
	m_addObjectAction->setEnabled(true);
	m_addObjectAction->setMenu(m_addObjectMenu);

		m_addSystemAction = new QAction(tr("System"), this);
		m_addSystemAction->setStatusTip(tr("Add system to the configuration..."));
		m_addSystemAction->setEnabled(false);
		connect(m_addSystemAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSystem);

		m_addRackAction = new QAction(tr("Rack"), this);
		m_addRackAction->setStatusTip(tr("Add rack to the configuration..."));
		m_addRackAction->setEnabled(false);
		connect(m_addRackAction, &QAction::triggered, m_equipmentView, &EquipmentView::addRack);

		m_addChassisAction = new QAction(tr("Chassis"), this);
		m_addChassisAction->setStatusTip(tr("Add chassis to the configuration..."));
		m_addChassisAction->setEnabled(false);
		connect(m_addChassisAction, &QAction::triggered, m_equipmentView, &EquipmentView::addChassis);

		m_addModuleAction = new QAction(tr("Module"), this);
		m_addModuleAction->setStatusTip(tr("Add module to the configuration..."));
		m_addModuleAction->setEnabled(false);
		connect(m_addModuleAction, &QAction::triggered, m_equipmentView, &EquipmentView::addModule);

		m_addControllerAction = new QAction(tr("Controller"), this);
		m_addControllerAction->setStatusTip(tr("Add controller to the configuration..."));
		m_addControllerAction->setEnabled(false);
		connect(m_addControllerAction, &QAction::triggered, m_equipmentView, &EquipmentView::addController);

		m_addSignalAction = new QAction(tr("Signal"), this);
		m_addSignalAction->setStatusTip(tr("Add signal to the configuration..."));
		m_addSignalAction->setEnabled(false);
		connect(m_addSignalAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSignal);


		m_addWorkstationAction = new QAction(tr("Workstation"), this);
		m_addWorkstationAction->setStatusTip(tr("Add workstation to the configuration..."));
		m_addWorkstationAction->setEnabled(false);
		connect(m_addWorkstationAction, &QAction::triggered, m_equipmentView, &EquipmentView::addWorkstation);

		m_addSoftwareAction = new QAction(tr("Software"), this);
		m_addSoftwareAction->setStatusTip(tr("Add software to the configuration..."));
		m_addSoftwareAction->setEnabled(false);
		connect(m_addSoftwareAction, &QAction::triggered, m_equipmentView, &EquipmentView::addSoftware);


	//----------------------------------
	m_addPresetMenu = new QMenu(this);

	m_addPresetAction = new QAction(tr("Add From Preset"), this);
	m_addPresetAction->setEnabled(true);
	m_addPresetAction->setMenu(m_addPresetMenu);

		m_addPresetRackAction = new QAction(tr("Preset Rack"), this);
		m_addPresetRackAction->setStatusTip(tr("Add rack to the preset..."));
		m_addPresetRackAction->setEnabled(false);
		connect(m_addPresetRackAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPresetRack);

		m_addPresetChassisAction = new QAction(tr("Preset Chassis"), this);
		m_addPresetChassisAction->setStatusTip(tr("Add chassis to the preset..."));
		m_addPresetChassisAction->setEnabled(false);
		connect(m_addPresetChassisAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPresetChassis);

		m_addPresetModuleAction = new QAction(tr("Preset Module"), this);
		m_addPresetModuleAction->setStatusTip(tr("Add module to the preset..."));
		m_addPresetModuleAction->setEnabled(false);
		connect(m_addPresetModuleAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPresetModule);

		m_addPresetControllerAction = new QAction(tr("Preset Controller"), this);
		m_addPresetControllerAction->setStatusTip(tr("Add controller to the preset..."));
		m_addPresetControllerAction->setEnabled(false);
		connect(m_addPresetControllerAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPresetController);

		m_addPresetWorkstationAction = new QAction(tr("Preset Worksation"), this);
		m_addPresetWorkstationAction->setStatusTip(tr("Add workstation to the preset..."));
		m_addPresetWorkstationAction->setEnabled(false);
		connect(m_addPresetWorkstationAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPresetWorkstation);

		m_addPresetSoftwareAction = new QAction(tr("Preset Software"), this);
		m_addPresetSoftwareAction->setStatusTip(tr("Add software to the preset..."));
		m_addPresetSoftwareAction->setEnabled(false);
		connect(m_addPresetSoftwareAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPresetSoftware);

	m_inOutsToSignals = new QAction(tr("Inputs/Outs to signals"), this);
	m_inOutsToSignals->setStatusTip(tr("Add intputs/outputs to signals..."));
	m_inOutsToSignals->setEnabled(false);
	m_inOutsToSignals->setVisible(false);
	connect(m_inOutsToSignals, &QAction::triggered, m_equipmentView, &EquipmentView::addInOutsToSignals);

	//-----------------------------------
	m_SeparatorAction1 = new QAction(this);
	m_SeparatorAction1->setSeparator(true);

	m_deleteObjectAction = new QAction(tr("Delete Device"), this);
	m_deleteObjectAction->setStatusTip(tr("Delete Device from the configuration..."));
	m_deleteObjectAction->setEnabled(false);
	connect(m_deleteObjectAction, &QAction::triggered, m_equipmentView, &EquipmentView::deleteSelectedDevices);

	m_SeparatorAction2 = new QAction(this);
	m_SeparatorAction2->setSeparator(true);

	m_checkOutAction = new QAction(tr("CheckOut"), this);
	m_checkOutAction->setStatusTip(tr("Check out device for edit"));
	m_checkOutAction->setEnabled(false);
	connect(m_checkOutAction, &QAction::triggered, m_equipmentView, &EquipmentView::checkOutSelectedDevices);

	m_checkInAction = new QAction(tr("CheckIn..."), this);
	m_checkInAction->setStatusTip(tr("Check in changes"));
	m_checkInAction->setEnabled(false);
	connect(m_checkInAction, &QAction::triggered, m_equipmentView, &EquipmentView::checkInSelectedDevices);

	m_undoChangesAction = new QAction(tr("Undo Changes..."), this);
	m_undoChangesAction->setStatusTip(tr("Undo all pending changes for the object"));
	m_undoChangesAction->setEnabled(false);
	connect(m_undoChangesAction, &QAction::triggered, m_equipmentView, &EquipmentView::undoChangesSelectedDevices);

	m_refreshAction = new QAction(tr("Refresh"), this);
	m_refreshAction->setStatusTip(tr("Refresh object list"));
	m_refreshAction->setEnabled(false);
	connect(m_refreshAction, &QAction::triggered, m_equipmentView, &EquipmentView::refreshSelectedDevices);

	//-----------------------------------
	m_SeparatorAction3 = new QAction(this);
	m_SeparatorAction3->setSeparator(true);

	m_switchMode = new QAction(tr("Switch to Preset"), this);
	m_switchMode->setStatusTip(tr("Switch to preset/configuration mode"));
	m_switchMode->setEnabled(true);
	connect(m_switchMode, &QAction::triggered, m_equipmentModel, &EquipmentModel::switchMode);
	connect(m_switchMode, &QAction::triggered, this, &EquipmentTabPage::modeSwitched);

	//-----------------------------------
	m_SeparatorAction4 = new QAction(this);
	m_SeparatorAction4->setSeparator(true);

//	m_moduleConfigurationAction = new QAction(tr("Modules Configuration..."), this);
//	m_moduleConfigurationAction->setStatusTip(tr("Edit module configuration"));
//	m_moduleConfigurationAction->setEnabled(false);
//	connect(m_moduleConfigurationAction, &QAction::triggered, this, &EquipmentTabPage::moduleConfiguration);

	return;
}

bool EquipmentTabPage::isPresetMode() const
{
	return m_equipmentModel->isPresetMode();
}

bool EquipmentTabPage::isConfigurationMode() const
{
	return m_equipmentModel->isConfigurationMode();
}

void EquipmentTabPage::closeEvent(QCloseEvent* e)
{
	e->accept();
}

void EquipmentTabPage::projectOpened()
{
	this->setEnabled(true);
	selectionChanged(QItemSelection(), QItemSelection());
	return;
}

void EquipmentTabPage::projectClosed()
{
	this->setEnabled(false);
	m_propertyEditor->clear();
	return;
}

void EquipmentTabPage::selectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
	setActionState();
	setProperties();
	return;
}

void EquipmentTabPage::modelDataChanged(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/, const QVector<int>& /*roles = QVector<int>()*/)
{
	return setActionState();
}

void EquipmentTabPage::setActionState()
{
	assert(m_addSystemAction);
	assert(m_addSystemAction);
	assert(m_addChassisAction);
	assert(m_addModuleAction);
	assert(m_addControllerAction);
	assert(m_addSignalAction);
	assert(m_addWorkstationAction);
	assert(m_addSoftwareAction);
	assert(m_deleteObjectAction);
	assert(m_checkOutAction);
	assert(m_checkInAction);
	assert(m_undoChangesAction);
	assert(m_refreshAction);
	assert(m_addPresetRackAction);
	assert(m_addPresetChassisAction);
	assert(m_addPresetModuleAction);
	assert(m_addPresetControllerAction);
	assert(m_addPresetWorkstationAction);
	assert(m_addPresetSoftwareAction);
	assert(m_inOutsToSignals);

	// Disable all
	//
	m_addSystemAction->setEnabled(false);
	m_addRackAction->setEnabled(false);
	m_addChassisAction->setEnabled(false);
	m_addModuleAction->setEnabled(false);
	m_addControllerAction->setEnabled(false);
	m_addSignalAction->setEnabled(false);

	m_addWorkstationAction->setEnabled(false);
	m_addSoftwareAction->setEnabled(false);

	m_deleteObjectAction->setEnabled(false);
	m_checkOutAction->setEnabled(false);
	m_checkInAction->setEnabled(false);
	m_undoChangesAction->setEnabled(false);
	m_refreshAction->setEnabled(false);

	m_addPresetRackAction->setEnabled(false);
	m_addPresetChassisAction->setEnabled(false);
	m_addPresetModuleAction->setEnabled(false);
	m_addPresetControllerAction->setEnabled(false);
	m_addPresetWorkstationAction->setEnabled(false);
	m_addPresetSoftwareAction->setEnabled(false);

	m_inOutsToSignals->setEnabled(false);
	m_inOutsToSignals->setVisible(false);


	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	QModelIndexList selectedIndexList = m_equipmentView->selectionModel()->selectedRows();

	// Refresh
	//
	m_refreshAction->setEnabled(true);

	// Add inputs/outputs to signals
	//
	if (selectedIndexList.size() == 1)
	{
		const Hardware::DeviceObject* device = m_equipmentModel->deviceObject(selectedIndexList.front());
		assert(device);

		const Hardware::DeviceModule* module = dynamic_cast<const Hardware::DeviceModule*>(device);

		if (module != nullptr && module->isIOModule() == true)
		{
			m_inOutsToSignals->setEnabled(true);
			m_inOutsToSignals->setVisible(true);
		}
	}

	// Delete Items action
	//
	m_deleteObjectAction->setEnabled(false);
	for (const QModelIndex& mi : selectedIndexList)
	{
		const Hardware::DeviceObject* device = m_equipmentModel->deviceObject(mi);
		assert(device);

		if (device->fileInfo().state() == VcsState::CheckedIn /*&&
			device->fileInfo().action() != VcsItemAction::Deleted*/)
		{
			m_deleteObjectAction->setEnabled(true);
			break;
		}

		if (device->fileInfo().state() == VcsState::CheckedOut &&
			(device->fileInfo().userId() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator())
			&& device->fileInfo().action() != VcsItemAction::Deleted)
		{
			m_deleteObjectAction->setEnabled(true);
			break;
		}
	}

	// CheckIn, CheckOut
	//
	bool canAnyBeCheckedIn = false;
	bool canAnyBeCheckedOut = false;

	for (const QModelIndex& mi : selectedIndexList)
	{
		const Hardware::DeviceObject* device = m_equipmentModel->deviceObject(mi);
		assert(device);

		if (device->fileInfo().state() == VcsState::CheckedOut &&
			(device->fileInfo().userId() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator()))
		{
			canAnyBeCheckedIn = true;
		}

		if (device->fileInfo().state() == VcsState::CheckedIn)
		{
			canAnyBeCheckedOut = true;
		}

		// Don't need to go further
		//
		if (canAnyBeCheckedIn == true &&
			canAnyBeCheckedIn == true )
		{
			break;
		}
	}

	m_checkInAction->setEnabled(canAnyBeCheckedIn);
	m_checkOutAction->setEnabled(canAnyBeCheckedOut);
	m_undoChangesAction->setEnabled(canAnyBeCheckedIn);

	// Enbale possible creation items;
	//
	if (selectedIndexList.size() > 1)
	{
		// Don't know after which item possible to insert new Device
		//
	}
	else
	{
		if (selectedIndexList.empty() == true)
		{
			m_addSystemAction->setEnabled(true);
		}
		else
		{
			QModelIndex singleSelectedIndex = selectedIndexList[0];

			Hardware::DeviceObject* selectedObject = m_equipmentModel->deviceObject(singleSelectedIndex);
			assert(selectedObject != nullptr);

			if (isPresetMode() == true && selectedObject->preset() == false)
			{
				assert(false);
				return;
			}

			switch (selectedObject->deviceType())
			{
			case Hardware::DeviceType::System:
				m_addSystemAction->setEnabled(true);
				m_addRackAction->setEnabled(true);
				m_addChassisAction->setEnabled(true);
				m_addModuleAction->setEnabled(true);
				m_addControllerAction->setEnabled(true);
				m_addSignalAction->setEnabled(true);
				m_addWorkstationAction->setEnabled(true);

				m_addPresetRackAction->setEnabled(true);
				m_addPresetChassisAction->setEnabled(true);
				m_addPresetModuleAction->setEnabled(true);
				m_addPresetControllerAction->setEnabled(true);
				m_addPresetWorkstationAction->setEnabled(true);
				break;

			case Hardware::DeviceType::Rack:
				if (isConfigurationMode() == true)
				{
					m_addRackAction->setEnabled(true);
					m_addChassisAction->setEnabled(true);
					m_addModuleAction->setEnabled(true);
					m_addControllerAction->setEnabled(true);
					m_addSignalAction->setEnabled(true);
					m_addWorkstationAction->setEnabled(true);
				}
				else
				{
					m_addRackAction->setEnabled(selectedObject->presetRoot() == false);
					m_addChassisAction->setEnabled(true);
					m_addModuleAction->setEnabled(true);
					m_addControllerAction->setEnabled(true);
					m_addSignalAction->setEnabled(true);
					m_addWorkstationAction->setEnabled(true);
				}

				m_addPresetRackAction->setEnabled(true);
				m_addPresetChassisAction->setEnabled(true);
				m_addPresetModuleAction->setEnabled(true);
				m_addPresetControllerAction->setEnabled(true);
				m_addPresetWorkstationAction->setEnabled(true);
				break;

			case Hardware::DeviceType::Chassis:
				if (isConfigurationMode() == true)
				{
					m_addChassisAction->setEnabled(true);
					m_addModuleAction->setEnabled(true);
					m_addControllerAction->setEnabled(true);
					m_addSignalAction->setEnabled(true);
					m_addWorkstationAction->setEnabled(true);
				}
				else
				{
					m_addChassisAction->setEnabled(selectedObject->presetRoot() == false);
					m_addModuleAction->setEnabled(true);
					m_addControllerAction->setEnabled(true);
					m_addSignalAction->setEnabled(true);
					m_addWorkstationAction->setEnabled(true);
				}

				m_addPresetChassisAction->setEnabled(true);
				m_addPresetModuleAction->setEnabled(true);
				m_addPresetControllerAction->setEnabled(true);
				m_addPresetWorkstationAction->setEnabled(true);
				break;

			case Hardware::DeviceType::Module:
				if (isConfigurationMode() == true)
				{
					m_addModuleAction->setEnabled(true);
				}
				else
				{
					m_addModuleAction->setEnabled(selectedObject->presetRoot() == false);
				}

				m_addControllerAction->setEnabled(true);
				m_addSignalAction->setEnabled(true);

				m_addPresetModuleAction->setEnabled(true);
				m_addPresetControllerAction->setEnabled(true);
				break;

			case Hardware::DeviceType::Controller:
				if (isConfigurationMode() == true)
				{
					m_addControllerAction->setEnabled(true);
				}
				else
				{
					m_addControllerAction->setEnabled(selectedObject->presetRoot() == false);
				}

				m_addSignalAction->setEnabled(true);

				m_addPresetControllerAction->setEnabled(true);
				break;

			case Hardware::DeviceType::Workstation:
				if (isConfigurationMode() == true)
				{
					m_addWorkstationAction->setEnabled(true);
					m_addSoftwareAction->setEnabled(true);
					m_addSignalAction->setEnabled(true);
				}
				else
				{
					m_addWorkstationAction->setEnabled(selectedObject->presetRoot() == false);
					m_addSoftwareAction->setEnabled(true);
					m_addSignalAction->setEnabled(true);
				}

				m_addPresetWorkstationAction->setEnabled(true);
				m_addPresetSoftwareAction->setEnabled(true);
				break;

			case Hardware::DeviceType::Software:
				if (isConfigurationMode() == true)
				{
					m_addSoftwareAction->setEnabled(true);
				}
				else
				{
					m_addSoftwareAction->setEnabled(selectedObject->presetRoot() == false);
				}

				m_addSignalAction->setEnabled(true);

				m_addPresetSoftwareAction->setEnabled(true);
				break;

			case Hardware::DeviceType::Signal:
				if (isConfigurationMode() == true)
				{
					m_addSignalAction->setEnabled(true);
				}
				else
				{
					m_addSignalAction->setEnabled(selectedObject->presetRoot() == false);
				}

				break;

			default:
				assert(false);
			}
		}
	}

	// Enable elements in preset mode
	//
	if (isPresetMode() == true)
	{
		m_addSystemAction->setEnabled(false);

		m_addPresetRackAction->setEnabled(true);
		m_addPresetChassisAction->setEnabled(true);
		m_addPresetModuleAction->setEnabled(true);
		m_addPresetControllerAction->setEnabled(true);
		m_addPresetWorkstationAction->setEnabled(true);
		m_addPresetSoftwareAction->setEnabled(true);
	}

	return;
}

void EquipmentTabPage::modeSwitched()
{
	if (m_equipmentModel->isPresetMode() == true)
	{
		m_switchMode->setText(tr("Switch to Configuration"));

		m_addPresetAction->setText(tr("Add New Preset"));
	}
	else
	{
		m_switchMode->setText(tr("Switch to Preset"));

		m_addPresetAction->setText(tr("Add From Preset"));
	}

	setActionState();

	return;
}

//void EquipmentTabPage::moduleConfiguration()
//{
	// Show modules configurations dialog
	//


//	//assert(m_propertyEditor != nullptr);

//	// Get objects from the selected rows
//	//
//	QModelIndexList selectedIndexList = m_equipmentView->selectionModel()->selectedRows();

//	if (selectedIndexList.size() != 1)
//	{
//		//m_propertyEditor->clear();
//		return;
//	}

//	std::shared_ptr<QObject> module;

//	std::shared_ptr<Hardware::DeviceObject> device = m_equipmentModel->deviceObjectSharedPtr(selectedIndexList[0]);

//	if (device.get() == nullptr || dynamic_cast<Hardware::DeviceModule*>(device.get()) == nullptr)
//	{
//		assert(device);
//		assert(dynamic_cast<Hardware::DeviceModule*>(device.get()) != nullptr);
//		return;
//	}



//	// Set objects to the PropertyEditor
//	//
//	//m_propertyEditor->setObjects(devices);

//	return;
//}

void EquipmentTabPage::setProperties()
{
	assert(m_propertyEditor != nullptr);

	// Get objects from the selected rows
	//
	QModelIndexList selectedIndexList = m_equipmentView->selectionModel()->selectedRows();

	if (selectedIndexList.empty() == true)
	{
		m_propertyEditor->clear();
		return;
	}

	QList<std::shared_ptr<QObject>> checkedInList;
	QList<std::shared_ptr<QObject>> checkedOutList;

	for (QModelIndex& mi : selectedIndexList)
	{
		std::shared_ptr<Hardware::DeviceObject> device = m_equipmentModel->deviceObjectSharedPtr(mi);
		assert(device);

		if (device->fileInfo().state() == VcsState::CheckedOut)
		{
			checkedOutList << device;
		}
		else
		{
			checkedInList << device;
		}
	}

	// Set objects to the PropertyEditor
	//
	if (checkedOutList.isEmpty() == false)
	{
		m_propertyEditor->setEnabled(true);
		m_propertyEditor->setObjects(checkedOutList);
	}
	else
	{
		m_propertyEditor->setEnabled(false);
		m_propertyEditor->setObjects(checkedInList);
	}

	return;
}

void EquipmentTabPage::propertiesChanged(QList<std::shared_ptr<QObject>> objects)
{
	std::vector<std::shared_ptr<DbFile>> files;

	for (auto& o : objects)
	{
		Hardware::DeviceObject* device = dynamic_cast<Hardware::DeviceObject*>(o.get());

		if (device == nullptr)
		{
			assert(device != nullptr);
			return;
		}

		QByteArray data;
		bool ok = device->Save(data);

		if (ok == false)
		{
			assert(false);
			return;
		}

		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		*file = device->fileInfo();
		file->swapData(data);

		files.push_back(file);
	}

	qDebug() << "Update Properties in the Database";

	bool ok = dbController()->setWorkcopy(files, this);

	if (ok == true)
	{
		// Refresh selected items
		//
		m_equipmentView->updateSelectedDevices();
	}

	return;
}
