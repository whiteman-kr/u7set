#include "Stable.h"
#include "EquipmentTabPage.h"
#include "../lib/DbController.h"
#include "Settings.h"
#include "CheckInDialog.h"
#include "Subsystem.h"
#include "EquipmentVcsDialog.h"
#include "DialogConnections.h"
#include "DialogChoosePreset.h"
#include "GlobalMessanger.h"
#include "SignalsTabPage.h"
#include "./Forms/FileHistoryDialog.h"
#include "./Forms/CompareDialog.h"
#include "./Forms/ComparePropertyObjectDialog.h"
#include "./Forms/DialogUpdateFromPreset.h"
#include "CreateSignalDialog.h"

#include <QPalette>
#include <QDialog>
#include <QtTreePropertyBrowser>
#include <QtGroupPropertyManager>
#include <QtStringPropertyManager>
#include <QtEnumPropertyManager>
#include <QtIntPropertyManager>
#include <QtDoublePropertyManager>
#include <QtBoolPropertyManager>
#include <QtSpinBoxFactory>
#include <QMimeData>

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
	m_configuration->setUuid(QUuid::createUuid());
	m_preset->setUuid(QUuid::createUuid());

	m_root = m_configuration;	// Edit configuration default mode

	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &EquipmentModel::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &EquipmentModel::projectClosed);
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

	const DbFileInfo& devieFileInfo = device->fileInfo();

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

			case ObjectEquipmentIdColumn:
				v.setValue<QString>(device->equipmentIdTemplate());
				break;

			case ObjectPlaceColumn:
				if (device->isRoot() ||
					device->isSystem() ||
					device->isRack() ||
					device->isSoftware())
				{
					v.setValue<QString>("");
				}
				else
				{
					v.setValue<int>(device->place());
				}
				break;

			case ObjectStateColumn:
				if (devieFileInfo.state() == VcsState::CheckedOut)
				{
					QString state = devieFileInfo.action().text();
					v.setValue<QString>(state);
				}
				break;

			case ObjectUserColumn:
				if (devieFileInfo.state() == VcsState::CheckedOut)
				{
					v.setValue<QString>(usernameById(devieFileInfo.userId()));
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
			if (devieFileInfo.state() == VcsState::CheckedOut)
			{
				QBrush b(QColor(0xFF, 0xFF, 0xFF));

				switch (static_cast<VcsItemAction::VcsItemActionType>(devieFileInfo.action().toInt()))
				{
				case VcsItemAction::Added:
					b.setColor(QColor(0xE9, 0xFF, 0xE9));
					break;
				case VcsItemAction::Modified:
					b.setColor(QColor(0xEA, 0xF0, 0xFF));
					break;
				case VcsItemAction::Deleted:
					b.setColor(QColor(0xFF, 0xF0, 0xF0));
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

			case ObjectEquipmentIdColumn:
				return QObject::tr("EquipmentIDTemplate");

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

	//qDebug() << object->caption() << " has children = " << hasChildren;

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

	bool ok = dbController()->getFileList(&files, parentObject->fileInfo().fileId(), true, m_parentWidget);
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

		std::shared_ptr<Hardware::DeviceObject> object = Hardware::DeviceObject::Create(file->data());

		if (object == nullptr)
		{
			assert(object);
			continue;
		}

		object->setFileInfo(fi);

		parentObject->addChild(object);
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
	case ObjectEquipmentIdColumn:
		object->sortByEquipmentId(order);
		break;
	case ObjectPlaceColumn:
		object->sortByPlace(order);
		break;
	case ObjectStateColumn:
		object->sortByState(order);
		break;
	case ObjectUserColumn:
		object->sortByUser(order, m_users);
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
	Hardware::DeviceObject* parent = deviceObject(parentIndex);

	// Insert
	//
	beginInsertRows(parentIndex, parent->childrenCount(), parent->childrenCount());
	parent->addChild(object);
	endInsertRows();

	// Sort items
	//
	sort(m_sortColumn, m_sortOrder);

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

	// As some rows can be deleted during update model,
	// rowList must be sorted in FileID descending order,
	// to delete first children and then their parents
	//
	QModelIndexList sortedRowList = rowList;

	qSort(sortedRowList.begin(), sortedRowList.end(),
		[this](QModelIndex& m1, QModelIndex m2)
		{
			Hardware::DeviceObject* d1 = deviceObject(m1);
			Hardware::DeviceObject* d2 = deviceObject(m2);

			return d1->fileInfo().fileId() >= d2->fileInfo().fileId();
		});

	// Update model
	//
	for (QModelIndex& index : sortedRowList)
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
			QModelIndex bottomRightIndex = this->index(index.row(), ColumnCount, index.parent());
			emit dataChanged(index, bottomRightIndex);
		}
	}

	emit objectVcsStateChanged();

	return;
}


void EquipmentModel::updateRowFuncOnCheckIn(QModelIndex modelIndex, const std::map<int, DbFileInfo>& updateFiles, std::set<void*>& updatedModelIndexes)
{
//static QString nested;
//	nested += "+---";

	if (updatedModelIndexes.find(modelIndex.internalPointer()) != updatedModelIndexes.end())
	{
//		qDebug() << "updateRowFuncOnCheckIn" << nested << " already updated";
		return;
	}
	else
	{
		updatedModelIndexes.insert(modelIndex.internalPointer());
	}

	// --
	//
	Hardware::DeviceObject* device = this->deviceObject(modelIndex);
	assert(device);
	assert(device->fileInfo().fileId() != -1);

	// Update children first, as items can be deleted
	//
	for (int childRow = device->childrenCount() - 1; childRow >= 0; childRow--)
	{
		if (updatedModelIndexes.find(device->child(childRow)) != updatedModelIndexes.end())
		{
			// skip, it was already processed
			//
			continue;
		}

		QModelIndex childIndex = modelIndex.child(childRow, 0);
		if (childIndex.isValid() == false)
		{
			assert(childIndex.isValid() == true);
			break;
		}

		Hardware::DeviceObject* childDevice = this->deviceObject(childIndex);
		Q_UNUSED(childDevice);

		assert(childDevice);
		assert(childDevice == device->child(childRow));

		updateRowFuncOnCheckIn(childIndex, updateFiles, updatedModelIndexes);
	}

	// Update current ModelIndex, Check if thes device was cgecked out
	//
	auto foundFileInfo = updateFiles.find(device->fileInfo().fileId());

	if (foundFileInfo != std::end(updateFiles))
	{
		assert(foundFileInfo->second.fileId() == device->fileInfo().fileId());

		device->setFileInfo(foundFileInfo->second);

		if (device->fileInfo().deleted() == true ||
			(device->fileInfo().action() == VcsItemAction::Deleted && device->fileInfo().state() == VcsState::CheckedIn))
		{
			QModelIndex pi = modelIndex.parent();
			Hardware::DeviceObject* po = this->deviceObject(pi);
			assert(po);

			int childIndex = po->childIndex(device);
			assert(childIndex != -1);

			beginRemoveRows(pi, childIndex, childIndex);
			po->deleteChild(device);
			endRemoveRows();
		}
		else
		{
			QModelIndex bottomRightIndex = this->index(modelIndex.row(), ColumnCount, modelIndex.parent());
			emit dataChanged(modelIndex, bottomRightIndex);
		}
	}

//	nested = nested.left(nested.size() - 4);
}

void EquipmentModel::checkInDeviceObject(QModelIndexList& rowList)
{
	QModelIndexList checkedOutList;
	DbUser currentUser = dbController()->currentUser();

	std::vector<DbFileInfo> files;
	files.reserve(rowList.size());

	for (QModelIndex& index : rowList)
	{
		Hardware::DeviceObject* d = deviceObject(index);
		assert(d);

		files.push_back(d->fileInfo());
	}

	// Get all checked out files for selected parents
	//
	std::vector<DbFileInfo> checkedOutFiles;
	dbController()->getCheckedOutFiles(&files, &checkedOutFiles, m_parentWidget);

	// Check in
	//
	std::vector<DbFileInfo> checkedInFiles;
	CheckInDialog::checkIn(checkedOutFiles, false, &checkedInFiles, dbController(), m_parentWidget);

	// Update model, look for all checkedInFiles and update their FileInfo
	//

	// Copy everything to map for faster access
	//
	std::map<int, DbFileInfo> updateFiles;

	for (const DbFileInfo& f : checkedInFiles)
	{
		updateFiles[f.fileId()] = f;
	}

	// Update FileInfo in devices and Update model
	//
	std::set<void*> updatedModelIndexes;

	for (QModelIndex& index : rowList)
	{
		updateRowFuncOnCheckIn(index, updateFiles, updatedModelIndexes);
	}

	emit objectVcsStateChanged();

	return;
}

void EquipmentModel::checkOutDeviceObject(QModelIndexList& rowList)
{
	std::vector<DbFileInfo> files;
	std::vector<std::shared_ptr<Hardware::DeviceObject>> objects;

	files.reserve(rowList.size());
	objects.reserve(rowList.size());

	QModelIndexList checkedInList;
	//DbUser currentUser = dbController()->currentUser();

	for (QModelIndex& index : rowList)
	{
		Hardware::DeviceObject* d = deviceObject(index);
		assert(d);

		if (d->fileInfo().state() == VcsState::CheckedIn)
		{
			files.push_back(d->fileInfo());
			checkedInList.push_back(index);

			objects.push_back(deviceObjectSharedPtr(index));
		}
	}

	bool ok = dbController()->checkOut(files, m_parentWidget);
	if (ok == false)
	{
		return;
	}

	// Get latest version of the objectcs
	// Update FileInfo in devices and Update model
	//
	std::vector<std::shared_ptr<DbFile>> freshFiles;
	freshFiles.reserve(files.size());

	ok = dbController()->getLatestVersion(files, &freshFiles, m_parentWidget);
	if (ok == false)
	{
		return;
	}

	for (QModelIndex& index : rowList)
	{
		Hardware::DeviceObject* d = deviceObject(index);

		if (d == nullptr)
		{
			assert(d);
			continue;
		}

		// Update object
		//
		auto freshFileIt = std::find_if(freshFiles.begin(), freshFiles.end(),
				[d](const std::shared_ptr<DbFile>& f)
				{
					return d->fileId() == f->fileId();
				});

		if (freshFileIt == freshFiles.end())
		{
			assert(false);
			continue;
		}
		else
		{
			const std::shared_ptr<DbFile>& freshFile = *freshFileIt;
			ok = d->Load(freshFile->data());	// Refresh data in the object
		}

		// Update fileInfo and model
		//
		for (const auto& fi : files)
		{
			if (fi.fileId() == d->fileInfo().fileId())
			{
				d->setFileInfo(fi);						// Update file info record in the DeviceOubject

				if (d->fileInfo().state() == VcsState::CheckedOut)
				{
					QModelIndex bottomRightIndex = this->index(index.row(), ColumnCount, index.parent());
					emit dataChanged(index, bottomRightIndex);		// Notify view about data update
				}

				break;
			}
		}
	}

	emit objectVcsStateChanged();

	return;
}

void EquipmentModel::undoChangesDeviceObject(QModelIndexList& undowRowList)
{
	QModelIndexList rowList = undowRowList;

	// As some rows can be deleted during update model,
	// rowList must be sorted in FileID descending order,
	// to delete first children and then their parents
	//
	qSort(rowList.begin(), rowList.end(),
		[this](const QModelIndex& m1, const QModelIndex& m2)
		{
			const Hardware::DeviceObject* d1 = this->deviceObject(m1);
			const Hardware::DeviceObject* d2 = this->deviceObject(m2);

			return d1->fileInfo().fileId() >= d2->fileInfo().fileId();
		});

	// --
	//
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

	bool ok = dbController()->undoChanges(files, m_parentWidget);
	if (ok == false)
	{
		return;
	}

	// If file was just added it will be removed completely from the DB
	//
	std::vector<DbFileInfo> latestFiles = files;

	latestFiles.erase(std::remove_if(latestFiles.begin(), latestFiles.end(),
					[](const DbFileInfo& fi)
					{
						return fi.deleted();
					}),
				latestFiles.end());

	// Get latest version of the object
	//
	std::vector<std::shared_ptr<DbFile>> latestFilesVersion;

	if (latestFiles.empty() == false)
	{
		ok = dbController()->getLatestVersion(latestFiles, &latestFilesVersion, m_parentWidget);

		if (ok == false)
		{
			// Can't update objects
			//
			return;
		}
	}

	// Update FileInfo in devices and Update model
	//
	for (QModelIndex& index : checkedOutList)
	{
		Hardware::DeviceObject* d = deviceObject(index);
		assert(d);

		// Set latest version to the object
		//
		auto foundFile = std::find_if(latestFilesVersion.begin(), latestFilesVersion.end(),
					[d](const std::shared_ptr<DbFile>& f)
					{
						return f->fileId() == d->fileInfo().fileId();
					});

		if (foundFile != latestFilesVersion.end())
		{
			const std::shared_ptr<DbFile>& f = *foundFile;
			d->Load(f->data());
		}
		else
		{
			// Apparently file was completely deleted from the DB
			//
		}

		// Update fileInfo
		//
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
					QModelIndex bottomRightIndex = this->index(index.row(), ColumnCount, index.parent());
					emit dataChanged(index, bottomRightIndex);
				}

				updated = true;
				break;
			}
		}
		assert(updated == true);
	}

	emit objectVcsStateChanged();

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

QString EquipmentModel::usernameById(int userId) const
{
	auto it = m_users.find(userId);

	if (it == m_users.end())
	{
		return QString("Undefined");
	}
	else
	{
		return it->second;
	}
}

void EquipmentModel::reset()
{
	beginResetModel();

	m_configuration->deleteAllChildren();
	m_preset->deleteAllChildren();
	m_users.clear();

	endResetModel();
}

void EquipmentModel::projectOpened()
{
	if (dbController()->isProjectOpened() == false)
	{
		assert(dbController()->isProjectOpened() == true);
		return;
	}

	beginResetModel();

	m_configuration->fileInfo().setFileId(dbController()->hcFileId());
	m_preset->fileInfo().setFileId(dbController()->hpFileId());

	// Fill user list
	//
	m_users.clear();

	std::vector<DbUser> users;
	bool ok = dbController()->getUserList(&users, nullptr);

	if (ok == true)
	{
		for (const DbUser& u : users)
		{
			m_users[u.userId()] = u.username();
		}
	}

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
const char* EquipmentView::mimeType = "application/x-deviceobjecs";
const char* EquipmentView::mimeTypeShortDescription = "application/x-deviceobjecs-sd";

EquipmentView::EquipmentView(DbController* dbcontroller) :
	m_dbController(dbcontroller)
{
	assert(m_dbController);

	setUniformRowHeights(true);

	setSortingEnabled(true);

	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);

	setUniformRowHeights(true);
	setIndentation(10);

	sortByColumn(EquipmentModel::Columns::ObjectPlaceColumn, Qt::SortOrder::AscendingOrder);


	// RPCT-633, somehow in this widow selectiop background was white, set it to something default
	// And the same situation was seen in QtCreator, I think it's some kind of bug, so, just set come colors
	// for selection
	//
	auto p = qApp->palette("QListView");

	QColor highlight = p.highlight().color();
	QColor highlightText = p.highlightedText().color();

	QString selectionColor = QString("QTreeView::item:selected { background-color: %1; color: %2; }")
							 .arg(highlight.name())
							 .arg(highlightText.name());

	setStyleSheet(selectionColor);

	// end of RPCT-633
	//

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

	addDeviceObject(system, parentModelIndex, true);

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

	addDeviceObject(rack, parentModelIndex, true);

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

	addDeviceObject(chassis, parentModelIndex, true);

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

	addDeviceObject(module, parentModelIndex, true);

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

	addDeviceObject(controller, parentModelIndex, true);

	emit updateState();
	return;
}

void EquipmentView::addSignal()
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
	std::shared_ptr<Hardware::DeviceObject> signal = std::make_shared<Hardware::DeviceSignal>(isPresetMode());

	signal->setEquipmentIdTemplate("$(PARENT)_SIGNAL");
	signal->setCaption(tr("Signal"));

	addDeviceObject(signal, parentModelIndex, true);

	emit updateState();
	return;
}

void EquipmentView::addWorkstation()
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
	std::shared_ptr<Hardware::DeviceObject> workstation = std::make_shared<Hardware::Workstation>(isPresetMode());

	workstation->setEquipmentIdTemplate("$(PARENT)_WS$(PLACE)");
	workstation->setCaption(tr("Workstation"));
	workstation->setPlace(0);

	addDeviceObject(workstation, parentModelIndex, true);

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

	addDeviceObject(software, parentModelIndex, true);

	emit updateState();
	return;
}

void EquipmentView::addPreset()
{
	if (isConfigurationMode() == false)
	{
		assert(isConfigurationMode() == true);
		return;
	}

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() == 1)
	{
		QModelIndex singleSelectedIndex = selectedIndexList[0];

		Hardware::DeviceObject* selectedObject = equipmentModel()->deviceObject(singleSelectedIndex);
		if (selectedObject == nullptr)
		{
			assert(selectedObject != nullptr);
			return;
		}

		DialogChoosePreset d(this, db(), selectedObject->deviceType(), false);
		if (d.exec() == QDialog::Accepted && d.selectedPreset != nullptr)
		{
			addPresetToConfiguration(d.selectedPreset->fileInfo(), true);
			emit updateState();
		}
	}

	return;
}

void EquipmentView::replaceObject()
{
	if (isConfigurationMode() == false)
	{
		assert(isConfigurationMode() == true);
		return;
	}

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() != 1)
	{
		assert(selectedIndexList.size() == 1);
		return;
	}

	QModelIndex singleSelectedIndex = selectedIndexList[0];

	Hardware::DeviceObject* selectedObject = equipmentModel()->deviceObject(singleSelectedIndex);
	if (selectedObject == nullptr)
	{
		assert(selectedObject != nullptr);
		return;
	}

	if (selectedObject->presetRoot() == false)
	{
		assert(selectedObject->presetRoot());
	}

	DialogChoosePreset d(this, db(), selectedObject->deviceType(), true);

	if (d.exec() == QDialog::Accepted &&
		d.selectedPreset != nullptr)
	{
		std::shared_ptr<Hardware::DeviceObject> selectedPreset = d.selectedPreset;

		if (selectedObject->presetName() == selectedPreset->presetName())
		{
			QMessageBox::critical(this, qAppName(), tr("Cannot replace object to itself."));
			return;
		}

		// selectedObject can be not fully loaded, as not all branches could be open in the UI.
		// Load selectedObject from the equipment
		//
		std::shared_ptr<Hardware::DeviceObject> selectedObjectFullTree;

		bool ok = db()->getDeviceTreeLatestVersion(selectedObject->fileInfo(), &selectedObjectFullTree, this);
		if (ok == false)
		{
			return;
		}

		assert(selectedObjectFullTree);

		// Delete selected device
		//
		deleteSelectedDevices();

		// Object will not be added to DB in the next line, it must be changed before adding
		//
		std::shared_ptr<Hardware::DeviceObject> device = addPresetToConfiguration(selectedPreset->fileInfo(), false);

		if (device == nullptr || device->presetRoot() == false)
		{
			assert(device);
			assert(device->presetRoot());
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
						srcValue.type() != dstProp->value().type())
					{
						continue;
					}

					dstProp->setValue(srcValue);
				}

				for (int childIndex = 0; childIndex < dst->childrenCount(); childIndex++)
				{
					Hardware::DeviceObject* dstChild = dst->child(childIndex);
					assert(dstChild);

					QString dstChildTemplateId = dstChild->equipmentIdTemplate();

					// Find the pair for this child
					//
					for (int srcChildIndex = 0; srcChildIndex < src->childrenCount(); srcChildIndex++)
					{
						Hardware::DeviceObject* srcChild = src->child(srcChildIndex);
						assert(srcChild);

						if (srcChild->equipmentIdTemplate() == dstChildTemplateId)
						{
							updatePropertuFunc(dstChild, srcChild);
							break;
						}
					}
				}

				return;
			};

		updatePropertuFunc(device.get(), selectedObjectFullTree.get());	// Recursive func

		// Add device
		//
		addDeviceObject(device, selectionModel()->selectedRows().at(0), true);
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

		addDeviceObject(rack, parentModelIndex, true);
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

		addDeviceObject(chassis, parentModelIndex, true);
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

		addDeviceObject(module, parentModelIndex, true);
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

		addDeviceObject(controller, parentModelIndex, true);
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

		addDeviceObject(workstation, parentModelIndex, true);
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

		addDeviceObject(software, parentModelIndex, true);
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
				addPresetToConfiguration(p->fileInfo(), true);
			});

		menu->addAction(a);
	}

	menu->exec(this->cursor().pos());

	return;
}

std::shared_ptr<Hardware::DeviceObject> EquipmentView::addPresetToConfiguration(const DbFileInfo& fileInfo, bool addToEquipment)
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
		return std::shared_ptr<Hardware::DeviceObject>();
	}

	if (device->fileInfo().fileId() != fileInfo.fileId() ||
		device->presetRoot() == false)
	{
		assert(device->fileInfo().fileId() == fileInfo.fileId());
		assert(device->presetRoot() == true);
		return std::shared_ptr<Hardware::DeviceObject>();
	}

	// If this is LM modlue, then set SusbSysID to the default value
	//
	if (device->isModule() == true)
	{
		Hardware::DeviceModule* module = device->toModule();

		if (module != nullptr && module->isFSCConfigurationModule() == true)
		{
			// Get susbsystems
			//
			Hardware::SubsystemStorage subsystems;
			QString errorCode;

			if (subsystems.load(db(), errorCode) == true && subsystems.count() > 0)
			{
				std::shared_ptr<Hardware::Subsystem> subsystem = subsystems.get(0);

				if (module->propertyExists("SubsystemID") == false)
                {
                    assert(false);
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
		addDeviceObject(device, parentModelIndex, true);
	}
	else
	{
		// Do nothing - probably object needs to be changed before adding
		//
	}

	return device;
}

QModelIndex EquipmentView::addDeviceObject(std::shared_ptr<Hardware::DeviceObject> object, QModelIndex parentModelIndex, bool clearPrevSelection)
{
	if (object == nullptr)
	{
		assert(object != nullptr);
		return QModelIndex();
	}

	if (isPresetMode() == true && object->preset() == false)
	{
		assert(false);
		return QModelIndex();
	}

	// Set new id, recusively to all children
	//
	bool presetMode = isPresetMode();

	std::function<void(Hardware::DeviceObject*)> setUuid = [&setUuid, presetMode](Hardware::DeviceObject* object)
		{
			assert(object);

			object->setUuid(QUuid::createUuid());

			if (presetMode == true)
			{
				object->setPresetObjectUuid(object->uuid());
			}

			for (int i = 0; i < object->childrenCount(); i++)
			{
				setUuid(object->child(i));
			}
		};

	setUuid(object.get());

	// Set Parent
	//
	Hardware::DeviceObject* parentObject = nullptr;
	//QModelIndex parentIndex;	// Currently it is root;

	if (isPresetMode() == true &&
		object->preset() == true &&
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
		assert(parentObject);

		if (object->deviceType() == Hardware::DeviceType::Software &&
			(parentObject->deviceType() != Hardware::DeviceType::Workstation && parentObject->deviceType() != Hardware::DeviceType::Software))
		{
			assert(false);
			return QModelIndex();
		}

		if (parentObject->deviceType() == object->deviceType())
		{
			// add the same item to the end of the the parent
			//
			parentModelIndex = parentModelIndex.parent();
			parentObject = equipmentModel()->deviceObject(parentModelIndex);

			assert(parentObject->deviceType() < object->deviceType());
		}

		if (parentObject->deviceType() > object->deviceType() ||
			(object->deviceType() == Hardware::DeviceType::Workstation && parentObject->deviceType() > Hardware::DeviceType::Chassis))
		{
			assert(parentObject->deviceType() <= object->deviceType());
			return QModelIndex();
		}
	}

	// Debugging .... parentObject->setChildRestriction("function(device) { return device.Place >=0 && device.Place < 16; }");

	assert(parentObject != nullptr);

	//  Set presetName, parent object should contain it
	//
	if (isPresetMode() == true &&
		object->preset() == true &&
		object->presetRoot() == false &&
		parentObject != nullptr)
	{
		object->setPresetName(parentObject->presetName());
	}

	// Add device to DB
	//
	bool result = db()->addDeviceObject(object.get(), parentObject->fileInfo().fileId(), this);

	if (result == false)
	{
		return QModelIndex();
	}

	// Add new device to the model and select it
	//
	object->deleteAllChildren();	// if there are any children they will be added to the model by reading project database

	equipmentModel()->insertDeviceObject(object, parentModelIndex);

	QModelIndex objectModelIndex = equipmentModel()->index(parentObject->childIndex(object.get()), parentModelIndex);

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

	if (module == nullptr || (module->isIOModule() == false && module->isLogicModule() == false && module->isOptoModule() == false))
	{
		assert(module);
		return;
	}

	// Check if the Place property coorect for the object and all it's parents
	//
	Hardware::DeviceObject* checkPlaceObject = module;

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
				getInOuts(device->child(i));
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

	Hardware::DeviceObject* equipmentDevice = module;
	while (equipmentDevice != nullptr)
	{
		if (equipmentDevice != module)
		{
			QByteArray bytes;

			equipmentDevice->Save(bytes);	// save and restore to keep equpment version after expanding strid

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
		[](Hardware::DeviceObject* a, Hardware::DeviceObject* b)
		{
			return a->equipmentIdTemplate() < b->equipmentIdTemplate();
		});

	std::vector<Signal> addedSignals;

	bool result = db()->autoAddSignals(&inOuts, &addedSignals, this);

	qDebug() << "Signals added:" << addedSignals.size();

	if (result == true)
	{
		QString messageText;

		if (addedSignals.empty() == true)
		{
			messageText = tr("Application Logic signlas for inputs/outputs were not added. Apparently they were added earlier.");
		}

		if (addedSignals.size() == 1)
		{
			messageText = tr("1 Appllication Logic signal for inputs/Ooutputs was added.");
		}

		if (addedSignals.size() > 1)
		{
			messageText = tr("%1 Appllication Logic signals for inputs/outputs were added.").arg(addedSignals.size());
		}

		QMessageBox::information(this, qAppName(), messageText);

		// Show application signals for current module
		//
		if (addedSignals.empty() == false)
		{
			showAppSignals(true);
		}
	}

	return;
}

void EquipmentView::showAppSignals(bool refreshSignalList /*= false*/)
{
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.isEmpty() == true)
	{
		assert(false);	// how did we get here?
		return;
	}

	QStringList strIds;

	for (const QModelIndex& mi : selectedIndexList)
	{
		const Hardware::DeviceObject* device = equipmentModel()->deviceObject(mi);
		assert(device);

		if (device != nullptr)
		{
			strIds.push_back(device->equipmentId() + "*");
		}
	}

	GlobalMessanger::instance()->fireShowDeviceApplicationSignals(strIds, refreshSignalList);

	return;
}

void EquipmentView::addAppSignal()
{
	qDebug() << "void EquipmentView::addAppSignal()";

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() != 1)
	{
		assert(false);	// how did we get here?
		return;
	}

	Hardware::DeviceObject* device = equipmentModel()->deviceObject(selectedIndexList.front());
	assert(device);

	Hardware::DeviceModule* module = dynamic_cast<Hardware::DeviceModule*>(device);

	if (module == nullptr || module->isLogicModule() == false)
	{
		assert(module);
		return;
	}

	QStringList equipmentIdList;
	equipmentIdList << module->equipmentId();

	static CreatingSignalDialogOptions options;
	options.init(module->equipmentId(), module->equipmentId(), equipmentIdList, QStringList());

	CreateSignalDialog::showDialog(db(), &options, this);

	return;
}

void EquipmentView::addLogicSchemaToLm()
{
	qDebug() << __FUNCTION__;

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();
	if (selectedIndexList.empty() == true)
	{
		assert(false);
		return;
	}

	QStringList deviceStrIds;
	QString lmDescriptioFile;
	bool lmDescriptioFileInitialized = false;

	for (const QModelIndex& mi : selectedIndexList)
	{
		const Hardware::DeviceObject* device = equipmentModel()->deviceObject(mi);
		assert(device);

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
				assert(false);	// How is it possible, it should be fileterd om nenu level
				return;
			}

			continue;
		}
		else
		{
			assert(false);	// How is it possible, it should be fileterd om nenu level
			return;
		}
	}

	GlobalMessanger::instance()->fireAddLogicSchema(deviceStrIds, lmDescriptioFile);
	return;
}

void EquipmentView::showLogicSchemaForLm()
{
	qDebug() << __FUNCTION__;

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() != 1)
	{
		assert(false);	// how did we get here?
		return;
	}

	Hardware::DeviceObject* device = equipmentModel()->deviceObject(selectedIndexList.front());
	assert(device);

	Hardware::DeviceModule* module = dynamic_cast<Hardware::DeviceModule*>(device);

	if (module == nullptr || module->isLogicModule() == false)
	{
		assert(module);
		return;
	}

	GlobalMessanger::instance()->fireSearchSchemaForLm(module->equipmentId());

	return;
}

void EquipmentView::addOptoConnection()
{
	qDebug() << __FUNCTION__;

	if (db()->isProjectOpened() == false)
	{
		return;
	}

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() != 1 && selectedIndexList.size() != 2)
	{
		assert(false);	// how did we get here?
		return;
	}

	const Hardware::DeviceController* controller1 = dynamic_cast<const Hardware::DeviceController*>(equipmentModel()->deviceObject(selectedIndexList.front()));
	const Hardware::DeviceController* controller2 = dynamic_cast<const Hardware::DeviceController*>(equipmentModel()->deviceObject(selectedIndexList.back()));

	if (controller1 == nullptr || controller2 == nullptr)
	{
		assert(controller1);
		assert(controller2);

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

void EquipmentView::showModuleOptoConnections()
{
	qDebug() << __FUNCTION__;

	if (db()->isProjectOpened() == false)
	{
		return;
	}

	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() == 0)
	{
		assert(false);	// how did we get here?
		return;
	}

	QString filter;
	for (auto mi : selectedIndexList)
	{
		Hardware::DeviceObject* device = equipmentModel()->deviceObject(mi);

		if (device == nullptr)
		{
			assert(device);
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
		assert(false);		// How did we get here, action should be disabled
		return;
	}

	// Check if all selected equipmnet has the same type
	//
	const Hardware::DeviceObject* firstDevice = equipmentModel()->deviceObject(selected.first());
	assert(firstDevice);

	Hardware::DeviceType type = firstDevice->deviceType();

	std::vector<const Hardware::DeviceObject*> devices;
	devices.reserve(selected.size());

	for (const QModelIndex& mi : selected)
	{
		const Hardware::DeviceObject* device = equipmentModel()->deviceObject(mi);
		assert(device);

		if (type != device->deviceType())
		{
			assert(false);
			return;
		}

		devices.push_back(device);
	}

	// Read devices from the project database
	//
	std::vector<std::shared_ptr<Hardware::DeviceObject>> latestDevices;
	latestDevices.reserve(devices.size());

	for (const Hardware::DeviceObject* device : devices)
	{
		std::shared_ptr<Hardware::DeviceObject> out;

		bool result = db()->getDeviceTreeLatestVersion(device->fileInfo(), &out, this);

		if (result == false)
		{
			return;
		}

		assert(out);

		latestDevices.push_back(out);
	}

	// Save devices to the clipboard
	//
	::Proto::EnvelopeSet message;

	// Save the short description -- its done for quick understanding what is in the clipboard without reading all data
	//
	::Proto::EnvelopeSetShortDescription descriptionMessage;

	descriptionMessage.set_projectdbversion(DbController::databaseVersion());
	descriptionMessage.set_equipmenteditor(isConfigurationMode());
	descriptionMessage.set_preseteditor(isPresetMode());

	for (std::shared_ptr<Hardware::DeviceObject> device : latestDevices)
	{
		::Proto::Envelope* protoDevice = message.add_items();
		device->SaveObjectTree(protoDevice);

		descriptionMessage.add_classnamehash(protoDevice->classnamehash());
	}

	// Save objects (EnvelopeSet) to byte array
	//
	std::string dataString;
	bool ok = message.SerializeToString(&dataString);

	if (ok == false)
	{
		assert(ok);
		return;
	}

	// Save short description (EnvelopeSetShortDescription) to byte array
	//
	std::string descriptionDataString;
	ok = descriptionMessage.SerializeToString(&descriptionDataString);

	if (ok == false)
	{
		assert(ok);
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
		assert(pasetIsAllowed == true);
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

	::Proto::EnvelopeSet message;
	bool ok = message.ParseFromArray(cbData.constData(), cbData.size());

	if (ok == false)
	{
		QMessageBox::critical(this, qApp->applicationName(),  tr("The Clipboard has been corrupted or has incompatible data format."));
		return;
	}

	// --
	//
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

	// Create objects andd add children
	//

	// function for setting new uuids
	//
	std::function<void(Hardware::DeviceObject*)> setUuid = [&setUuid](Hardware::DeviceObject* object)
		{
			assert(object);
			object->setUuid(QUuid::createUuid());

			for (int i = 0; i < object->childrenCount(); i++)
			{
				setUuid(object->child(i));
			}
		};

	selectionModel()->clearSelection();
	QModelIndex lastModelIndex;

	for (int i = 0; i < message.items_size(); i++)
	{
		std::shared_ptr<Hardware::DeviceObject> object(Hardware::DeviceObject::Create(message.items(i)));

		if (object == nullptr)
		{
			QMessageBox::critical(this, qApp->applicationName(),  tr("The Clipboard has been corrupted or has incompatible data format. Object data parsing error."));
			break;
		}

		setUuid(object.get());		// Set new guids to all objects

		lastModelIndex = addDeviceObject(object, parentModelIndex, false);
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
	bool ok = message.ParseFromArray(cbData.constData(), cbData.size());

	if (ok == false)
	{
		return false;
	}

	// Check if the copy was done from current mode, so copy possible only from editor to edir or preset editor to preset editor
	//
	if (isPresetMode() == true && message.preseteditor() == false)
	{
		return false;
	}

	if (isConfigurationMode() == true && message.equipmenteditor() == false)
	{
		return false;
	}

	// --
	//
	if (message.classnamehash().size() == 0)
	{
		assert(message.classnamehash().size() > 0);
		return false;
	}

	quint32 classNameHash = message.classnamehash(0);		// get only first, suppose all of the have the same type

	std::shared_ptr<Hardware::DeviceObject> deviceObject = Hardware::DeviceObjectFactory.Create(classNameHash);

	if (deviceObject == nullptr)
	{
		assert(deviceObject);
		return false;
	}

	// Is it possible to insert it into current selection
	//
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

	if (selectedIndexList.size() != 1)
	{
		return false;
	}

	const Hardware::DeviceObject* selectedDevice = equipmentModel()->deviceObject(selectedIndexList.at(0));
	assert(selectedDevice);

	if (selectedDevice == nullptr)
	{
		assert(selectedDevice);
		return false;
	}

	if (selectedDevice->canAddChild(deviceObject.get()) == false)
	{
		return false;
	}

	return true;
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
	const QSignalBlocker blocker(selectionModel());
	Q_UNUSED(blocker);

	// Perform undo
	//
	equipmentModel()->undoChangesDeviceObject(selected);

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
	Hardware::DeviceObject* device = equipmentModel()->deviceObject(selected.front());

	if (device == nullptr)
	{
		assert(device);
		return;
	}

	// Get file history
	//
	std::vector<DbChangeset> fileHistory;

	bool ok = db()->getFileHistoryRecursive(device->fileInfo(), &fileHistory, this);
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
	Hardware::DeviceObject* device = equipmentModel()->deviceObject(selected.front());

	if (device == nullptr)
	{
		assert(device);
		return;
	}

	// --
	//
	CompareDialog::showCompare(db(), DbChangesetObject(device->fileInfo()), -1, this);

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

	// Get all presets
	//
	DbFileInfo hpFileInfo = db()->systemFileInfo(db()->hpFileId());		//	hp -- stands for Hardware Presets
	assert(hpFileInfo.isNull() == false);

	std::shared_ptr<Hardware::DeviceObject> presetRoot;

	bool ok = db()->getDeviceTreeLatestVersion(hpFileInfo, &presetRoot, this);

	if (ok == false)
	{
		return;
	}

	assert(presetRoot);

	// Get All preset Roots
	//
	std::map<QString, std::shared_ptr<Hardware::DeviceObject>> presets;
	QStringList presetsToUpdate;

	for (int i = 0; i < presetRoot->childrenCount(); i++)
	{
		std::shared_ptr<Hardware::DeviceObject> preset = presetRoot->childSharedPtr(i);

		if (preset.get() == nullptr || preset->presetRoot() == false)
		{
			assert(preset);
			assert(preset->presetRoot() == true);
			continue;
		}

		if (presets.count(preset->presetName()) > 0)
		{
			QMessageBox::critical(this,
								  QApplication::applicationName(),
								  tr("There are preset with the same name %1. Preset names must be unique. Update from preset is not posible.")
										.arg(preset->presetName()));
			return;
		}

		presetsToUpdate.push_back(preset->presetName());
		presets[preset->presetName()] = preset;
	}

	presetRoot.reset();

	// Show confirmation dialog
	//
	DialogUpdateFromPreset dialog(theSettings.isExpertMode(), presetsToUpdate, this);

	int result = dialog.exec();

	if (result != QDialog::Accepted)
	{
		return;
	}

	QStringList forceUpdateProperties;

	if (theSettings.isExpertMode() == true)
	{
		// Get properties which must be updated even if they not meant to update
		//
		forceUpdateProperties = dialog.forceUpdateProperties();
		presetsToUpdate = dialog.selectedPresets();
	}

	// Get all equipment from the database
	//
	DbFileInfo hcFileInfo = db()->systemFileInfo(db()->hcFileId());
	assert(hcFileInfo.isNull() == false);

	std::shared_ptr<Hardware::DeviceObject> root;

	ok = db()->getDeviceTreeLatestVersion(hcFileInfo, &root, this);

	if (ok == false)
	{
		return;
	}

	assert(root);

	// Check out all preset files
	//
	std::vector<DbFileInfo> presetFiles;									// Files to check out
	presetFiles.reserve(65536 * 2);

	std::vector<std::shared_ptr<Hardware::DeviceObject>> presetRoots;		// preset root objectes to start update from preset operation
	presetRoots.reserve(8192);

	std::function<void(std::shared_ptr<Hardware::DeviceObject>)> getPresetFiles =
		[&presetRoots, &getPresetFiles, &presetFiles](std::shared_ptr<Hardware::DeviceObject> object)
		{
			assert(object);

			if (object->preset() == true)
			{
				presetFiles.push_back(object->fileInfo());
			}

			if (object->preset() == true &&
				object->presetRoot() == true)
			{
				presetRoots.push_back(object);
			}

			for (int i = 0; i < object->childrenCount(); i++)
			{
				getPresetFiles(object->childSharedPtr(i));
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
		// Cannot check out one or more files, update from preset is imposiible
		//
		QMessageBox::critical(this,
							  QApplication::applicationName(),
							  tr("Cannot check out one or more files, update from preset is not posible."));
		return;
	}

	// All files were checked out by the current user, update preset can be performed now
	//

	// Update all preset objects
	//
	std::vector<std::shared_ptr<Hardware::DeviceObject>> updateDeviceList;
	std::vector<Hardware::DeviceObject*> deleteDeviceList;
	std::vector<std::pair<int, int>> addDeviceList;		// first: parent fileId, second: preset file id

	updateDeviceList.reserve(65536 * 2);
	deleteDeviceList.reserve(65536);
	addDeviceList.reserve(65536);

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

			assert(false);
			return;
		}

		// --
		//
		std::shared_ptr<Hardware::DeviceObject> preset = foundPreset->second;

		assert(preset->presetRoot() == true);
		assert(preset->presetName() == presetName);

		ok = updateDeviceFromPreset(device, preset, forceUpdateProperties, presetsToUpdate, &updateDeviceList, &deleteDeviceList, &addDeviceList);
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
			assert(device != nullptr);
			continue;
		}

		QByteArray data;
		bool ok = device->Save(data);

		if (ok == false)
		{
			assert(false);
			continue;
		}

		std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

		*file = device->fileInfo();
		file->swapData(data);

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
			assert(parentFileId != -1);
			assert(presetFileId != -1);
			continue;
		}

		// Read preset tree from DB
		//
		std::shared_ptr<Hardware::DeviceObject> device;
		DbFileInfo presetFileInfo;
		presetFileInfo.setFileId(presetFileId);

		bool ok = db()->getDeviceTreeLatestVersion(presetFileInfo, &device, this);
		if (ok == false)
		{
			return;
		}

		if (device == nullptr ||
			device->preset() == false ||
			device->fileInfo().fileId() != presetFileInfo.fileId())		// can be not presetRoot
		{
			assert(device);
			assert(device->preset() == true);
			assert(device->fileInfo().fileId() == presetFileInfo.fileId());
			return;
		}

		// Set new id, recusively to all children
		//
		std::function<void(Hardware::DeviceObject*)> setUuid = [&setUuid](Hardware::DeviceObject* object)
			{
				assert(object);

				object->setUuid(QUuid::createUuid());

				for (int i = 0; i < object->childrenCount(); i++)
				{
					setUuid(object->child(i));
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

	// Reset model
	//
	equipmentModel()->reset();

	return;
}

bool EquipmentView::updateDeviceFromPreset(std::shared_ptr<Hardware::DeviceObject> device,		// Device to update from preset
										   std::shared_ptr<Hardware::DeviceObject> preset,		// Preset to update device
										   const QStringList& forceUpdateProperties,			// Update theses props even if they not meant to updayt
										   const QStringList& presetsToUpdate,					// Update only these presets
										   std::vector<std::shared_ptr<Hardware::DeviceObject>>* updateDeviceList,
										   std::vector<Hardware::DeviceObject*>* deleteDeviceList,
										   std::vector<std::pair<int, int>>* addDeviceList)
{
	if (updateDeviceList == nullptr ||
		deleteDeviceList == nullptr ||
		addDeviceList == nullptr)
	{
		assert(updateDeviceList);
		assert(deleteDeviceList);
		assert(addDeviceList);
		return false;
	}

	if (device == nullptr)
	{
		assert(device);
		return false;
	}

	if (device->preset() == true &&
		(preset == nullptr ||
		device->presetName() != preset->presetName() ||
		device->presetRoot() != preset->presetRoot()))
	{
		assert(preset);
		assert(device->presetName() == preset->presetName());
		assert(device->presetRoot() == preset->presetRoot());
		return false;
	}

//	qDebug();
//	qDebug() << "EquipmentView::updateDeviceFromPreset"
//			 << ", device: " << device->equipmentIdTemplate()
//			 << "(" << device->equipmentId() << ")"
//			 << ", " << device->caption()
//			 << ", place: " << device->place();

	updateDeviceList->push_back(device);

	// If it is preset, update object add/delete children
	//
	if (device->preset() == true)
	{
		if (presetsToUpdate.contains(device->presetName()) == true)
		{
			// Update device object properties
			//
			std::vector<std::shared_ptr<Property>> deviceProperties = device->properties();
			std::vector<std::shared_ptr<Property>> presetProperties = preset->properties();

			for (auto dit = deviceProperties.begin(); dit != deviceProperties.end(); /*iterator inceremented in the loop body*/)
			{
				std::shared_ptr<Property> deviceProperty = *dit;

				auto pit = std::find_if(presetProperties.begin(), presetProperties.end(),
										[&deviceProperty](std::shared_ptr<Property> preset)
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
						// The type is different, PropertyValue<int> <-> PropettyValue<QString>
						// Obviosly thi2s is static properties
						//
						assert(false);
					}

					++dit;
					continue;
				}

				assert(false);
			}

			// Check if there are any new proprties in preset, the add them to device
			//
			for (auto pit = presetProperties.begin(); pit != presetProperties.end();)
			{
				std::shared_ptr<Property> presetProperty = *pit;

				auto dit = std::find_if(deviceProperties.begin(), deviceProperties.end(),
										[presetProperty](std::shared_ptr<Property> device)
					{
						return device->caption() == presetProperty->caption();
					});

				if (dit == deviceProperties.end())
				{
					// Preset property is not found in device, this is new property, add it
					//
					assert(dynamic_cast<PropertyValueNoGetterSetter*>(presetProperty.get()) != nullptr);

					std::shared_ptr<PropertyValueNoGetterSetter> newDeviceProperty = std::make_shared<PropertyValueNoGetterSetter>();
					newDeviceProperty->updateFromPreset(presetProperty.get(), true);

					deviceProperties.push_back(newDeviceProperty);
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
			std::shared_ptr<Hardware::DeviceObject> deviceChild = device->childSharedPtr(i);
			std::shared_ptr<Hardware::DeviceObject> presetChild = preset->childSharedPtr(deviceChild->presetObjectUuid());

			// WARNING, in the nex condition we cannot check for presetsToUpdate, as in this case we cannot go deeper as presetChild is nullptr and
			// we will have assert in the begining of updateDeviceFromPreset(). So deleted child will be removed from instance even though the prteset
			// should not be updated
			//
			if (deviceChild->preset() == true &&
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
								deleteDevices(device->child(i), deleteDeviceList);
							}
						};

				deleteDevices(deviceChild.get(), deleteDeviceList);
				continue;
			}

			// update child
			//
			if (deviceChild->preset() &&
				deviceChild->presetName() == device->presetName())
			{
				updateDeviceFromPreset(deviceChild, presetChild, forceUpdateProperties, presetsToUpdate, updateDeviceList, deleteDeviceList, addDeviceList);
			}
		}

		// Add children
		//
		for (int i = 0; i < preset->childrenCount(); i++)
		{
			std::shared_ptr<Hardware::DeviceObject> presetChild = preset->childSharedPtr(i);
			std::shared_ptr<Hardware::DeviceObject> deviceChild = device->childSharedPtrByPresetUuid(presetChild->presetObjectUuid());

			if (deviceChild == nullptr)
			{
				// Child is not added yet, add it
				//
				addDeviceList->push_back(std::make_pair(device->fileId(), presetChild->fileId()));
			}
		}
	}

	// Update all non preset children
	//
	for (int i = 0; i < device->childrenCount(); i++)
	{
		std::shared_ptr<Hardware::DeviceObject> deviceChild = device->childSharedPtr(i);
		std::shared_ptr<Hardware::DeviceObject> preset;		// not intialized, as deviceChild is not preset

		if (deviceChild->preset() == false)
		{
			updateDeviceFromPreset(deviceChild, preset, forceUpdateProperties, presetsToUpdate, updateDeviceList, deleteDeviceList, addDeviceList);
		}
	}

	return true;
}

void EquipmentView::focusInEvent(QFocusEvent* /*event*/)
{
	QList<QAction*> acts = actions();

	for (QAction* a : acts)
	{
		assert(a);

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
			assert(a);

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
		}
	}

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

	// -----------------
	m_equipmentView->addAction(m_addFromPresetAction);
	m_equipmentView->addAction(m_replaceAction);

	// -----------------
	m_equipmentView->addAction(m_addNewPresetAction);

		m_addPresetMenu->addAction(m_addPresetRackAction);
		m_addPresetMenu->addAction(m_addPresetChassisAction);
		m_addPresetMenu->addAction(m_addPresetModuleAction);
		m_addPresetMenu->addAction(m_addPresetControllerAction);
		m_addPresetMenu->addAction(m_addPresetWorkstationAction);
		m_addPresetMenu->addAction(m_addPresetSoftwareAction);

	// -----------------
	m_equipmentView->addAction(m_separatorAction0);
	m_equipmentView->addAction(m_inOutsToSignals);
	m_equipmentView->addAction(m_showAppSignals);
	m_equipmentView->addAction(m_addAppSignal);

	// -----------------
	m_equipmentView->addAction(m_separatorSchemaLogic);
	m_equipmentView->addAction(m_addLogicSchemaToLm);
	m_equipmentView->addAction(m_showLmsLogicSchemas);

	// -----------------
	m_equipmentView->addAction(m_separatorOptoConnection);
	m_equipmentView->addAction(m_addOptoConnection);
	m_equipmentView->addAction(m_showModuleOptoConnections);

	// -----------------
	m_equipmentView->addAction(m_separatorAction01);
	m_equipmentView->addAction(m_copyObjectAction);
	m_equipmentView->addAction(m_pasteObjectAction);

	// -----------------
	m_equipmentView->addAction(m_separatorAction1);
	m_equipmentView->addAction(m_deleteObjectAction);
	// -----------------
	m_equipmentView->addAction(m_separatorAction2);
	m_equipmentView->addAction(m_checkOutAction);
	m_equipmentView->addAction(m_checkInAction);
	m_equipmentView->addAction(m_undoChangesAction);
	m_equipmentView->addAction(m_historyAction);
	m_equipmentView->addAction(m_compareAction);
	m_equipmentView->addAction(m_refreshAction);
	// -----------------
	m_equipmentView->addAction(m_separatorAction3);
	m_equipmentView->addAction(m_updateFromPresetAction);
	m_equipmentView->addAction(m_switchModeAction);
	m_equipmentView->addAction(m_connectionsAction);

	//m_equipmentView->addAction(m_pendingChangesAction);	// Not implemented, removed to be consistent with User Manual

	// -----------------
	//m_equipmentView->addAction(m_SeparatorAction4);
	//m_equipmentView->addAction(m_moduleConfigurationAction);

	// Property View
	//

	// Splitter
	//
	m_splitter = new QSplitter(this);

	m_propertyEditor = new IdePropertyEditor(m_splitter, dbcontroller);
    m_propertyEditor->setSplitterPosition(theSettings.m_equipmentTabPagePropertiesSplitterState);


	m_splitter->addWidget(m_equipmentView);
	m_splitter->addWidget(m_propertyEditor);


	m_splitter->setStretchFactor(0, 2);
	m_splitter->setStretchFactor(1, 1);

	m_splitter->restoreState(theSettings.m_equipmentTabPageSplitterState);

	// ToolBar
	//
	m_toolBar = new QToolBar(this);
	m_toolBar->addAction(m_addObjectAction);
	m_toolBar->addAction(m_addFromPresetAction);
	m_toolBar->addAction(m_addNewPresetAction);
	m_toolBar->addAction(m_replaceAction);

	m_separatorActionA = new QAction(tr("Preset"), this);
	m_separatorActionA->setSeparator(true);

	m_toolBar->addAction(m_separatorActionA);
	m_toolBar->addAction(m_refreshAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_switchModeAction);
	m_toolBar->addAction(m_connectionsAction);

	//m_toolBar->addAction(m_pendingChangesAction);		// Not implemented, removed to be consistent with User Manual

	//
	// Layouts
	//
	QVBoxLayout* pMainLayout = new QVBoxLayout();

	pMainLayout->addWidget(m_toolBar);
	pMainLayout->addWidget(m_splitter);

	setLayout(pMainLayout);

	// --
	//
	connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &EquipmentTabPage::clipboardChanged);

	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &EquipmentTabPage::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &EquipmentTabPage::projectClosed);

	connect(m_equipmentView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &EquipmentTabPage::selectionChanged);

	//connect(m_equipmentModel, &EquipmentModel::dataChanged, this, &EquipmentTabPage::modelDataChanged);
	connect(m_equipmentView, &EquipmentView::updateState, this, &EquipmentTabPage::setActionState);

    connect(m_propertyEditor, &ExtWidgets::PropertyEditor::propertiesChanged, this, &EquipmentTabPage::propertiesChanged);

	connect(m_equipmentModel, &EquipmentModel::objectVcsStateChanged, this, &EquipmentTabPage::objectVcsStateChanged);

	connect(GlobalMessanger::instance(), &GlobalMessanger::compareObject, this, &EquipmentTabPage::compareObject);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

EquipmentTabPage::~EquipmentTabPage()
{
	theSettings.m_equipmentTabPageSplitterState = m_splitter->saveState();
    theSettings.m_equipmentTabPagePropertiesSplitterState = m_propertyEditor->splitterPosition();
	theSettings.writeUserScope();
}

void EquipmentTabPage::CreateActions()
{
	//-------------------------------
	m_addObjectMenu = new QMenu(this);

	m_addObjectAction = new QAction(tr("Add Object"), this);
	m_addObjectAction->setEnabled(true);
	m_addObjectAction->setMenu(m_addObjectMenu);
	connect(m_addObjectAction, &QAction::triggered, this, &EquipmentTabPage::addObjectTriggered);

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
	m_addFromPresetAction = new QAction(tr("Add From Preset..."), this);
	m_addFromPresetAction->setStatusTip(tr("Add preset to the configuration..."));
	m_addFromPresetAction->setEnabled(true);
	connect(m_addFromPresetAction, &QAction::triggered, m_equipmentView, &EquipmentView::addPreset);

	m_replaceAction = new QAction(tr("Replace with..."), this);
	m_replaceAction->setStatusTip(tr("Replace selected object with selected one"));
	m_replaceAction->setEnabled(false);
	connect(m_replaceAction, &QAction::triggered, m_equipmentView, &EquipmentView::replaceObject);

	//----------------------------------
	m_addPresetMenu = new QMenu(this);

	m_addNewPresetAction = new QAction(tr("Add New Preset"), this);
	m_addNewPresetAction->setEnabled(true);
	m_addNewPresetAction->setMenu(m_addPresetMenu);
	connect(m_addNewPresetAction, &QAction::triggered, this, &EquipmentTabPage::addNewPresetTriggered);

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

	//-----------------------------------
	m_separatorAction0 = new QAction(tr("Application Signals"), this);
	m_separatorAction0->setSeparator(true);

	m_inOutsToSignals = new QAction(tr("Add Inputs/Outs to App Signals"), this);
	m_inOutsToSignals->setStatusTip(tr("Add intputs/outputs to application logic signals..."));
	m_inOutsToSignals->setEnabled(false);
	//m_inOutsToSignals->setVisible(false);
	connect(m_inOutsToSignals, &QAction::triggered, m_equipmentView, &EquipmentView::addInOutsToSignals);

	m_showAppSignals = new QAction(tr("Show Application Signals"), this);
	m_showAppSignals->setStatusTip(tr("Show application signals for object and all its children"));
	m_showAppSignals->setEnabled(false);
	connect(m_showAppSignals, &QAction::triggered, m_equipmentView, &EquipmentView::showAppSignals);

	m_addAppSignal = new QAction(tr("Add Appliaction Signal"), this);
	m_addAppSignal->setStatusTip(tr("Add new application signal to device"));
	m_addAppSignal->setEnabled(false);
	connect(m_addAppSignal, &QAction::triggered, m_equipmentView, &EquipmentView::addAppSignal);

	//-----------------------------------
	m_separatorSchemaLogic = new QAction(tr("Application Logic"), this);
	m_separatorSchemaLogic->setSeparator(true);

	m_addLogicSchemaToLm = new QAction(tr("Add AppLogic Schema..."), this);
	m_addLogicSchemaToLm->setStatusTip(tr("Add Application Logic Schema to selected module"));
	m_addLogicSchemaToLm->setEnabled(false);
	//m_addLogicSchemaToLm->setVisible(false);
	connect(m_addLogicSchemaToLm, &QAction::triggered, m_equipmentView, &EquipmentView::addLogicSchemaToLm);

	m_showLmsLogicSchemas = new QAction(tr("Show AppLogic Schemas..."), this);
	m_showLmsLogicSchemas->setStatusTip(tr("Show Application Logic Schema for selected module"));
	m_showLmsLogicSchemas->setEnabled(false);
	//m_showLmsLogicSchemas->setVisible(false);
	connect(m_showLmsLogicSchemas, &QAction::triggered, m_equipmentView, &EquipmentView::showLogicSchemaForLm);

	//-----------------------------------
	m_separatorOptoConnection = new QAction(tr("Connections"), this);
	m_separatorOptoConnection->setSeparator(true);

	m_addOptoConnection = new QAction(tr("Create Opto Connection..."), this);
	m_addOptoConnection->setStatusTip(tr("Create optical connection for selected opto port(s)"));
	m_addOptoConnection->setEnabled(false);
	//m_addOptoConnection->setVisible(false);
	connect(m_addOptoConnection, &QAction::triggered, m_equipmentView, &EquipmentView::addOptoConnection);


	m_showModuleOptoConnections = new QAction(tr("Show Connections..."), this);
	m_showModuleOptoConnections->setStatusTip(tr("Show module or opto port connections"));
	m_showModuleOptoConnections->setEnabled(false);
	//m_showModuleOptoConnections->setVisible(false);
	connect(m_showModuleOptoConnections, &QAction::triggered, m_equipmentView, &EquipmentView::showModuleOptoConnections);

	//-----------------------------------
	m_separatorAction01 = new QAction(this);
	m_separatorAction01->setSeparator(true);

	m_copyObjectAction = new QAction(tr("Copy"), this);
	m_copyObjectAction->setStatusTip(tr("Copy equipment to the clipboard..."));
	m_copyObjectAction->setEnabled(false);
	m_copyObjectAction->setShortcut(QKeySequence::Copy);
	m_copyObjectAction->setObjectName("I_am_a_Copy_Action");
	connect(m_copyObjectAction, &QAction::triggered, m_equipmentView, &EquipmentView::copySelectedDevices);

	m_pasteObjectAction = new QAction(tr("Paste"), this);
	m_pasteObjectAction->setStatusTip(tr("Paste equipment from the clipboard..."));
	m_pasteObjectAction->setEnabled(false);
	m_pasteObjectAction->setShortcut(QKeySequence::Paste);
	m_pasteObjectAction->setObjectName("I_am_a_Paste_Action");
	connect(m_pasteObjectAction, &QAction::triggered, m_equipmentView, &EquipmentView::pasteDevices);

	//-----------------------------------
	m_separatorAction1 = new QAction(this);
	m_separatorAction1->setSeparator(true);

	m_deleteObjectAction = new QAction(tr("Delete Equipment"), this);
	m_deleteObjectAction->setStatusTip(tr("Delete equipment from the configuration..."));
	m_deleteObjectAction->setEnabled(false);
	m_deleteObjectAction->setShortcut(QKeySequence::Delete);
	connect(m_deleteObjectAction, &QAction::triggered, m_equipmentView, &EquipmentView::deleteSelectedDevices);

	//-----------------------------------
	m_separatorAction2 = new QAction(this);
	m_separatorAction2->setSeparator(true);

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

	m_historyAction = new QAction(tr("History..."), this);
	m_historyAction->setStatusTip(tr("Show check in history"));
	m_historyAction->setEnabled(false);
	connect(m_historyAction, &QAction::triggered, m_equipmentView, &EquipmentView::showHistory);

	m_compareAction = new QAction(tr("Compare..."), this);
	m_compareAction->setStatusTip(tr("Compare file"));
	m_compareAction->setEnabled(false);
	connect(m_compareAction, &QAction::triggered, m_equipmentView, &EquipmentView::compare);

	m_refreshAction = new QAction(tr("Refresh"), this);
	m_refreshAction->setStatusTip(tr("Refresh object list"));
	m_refreshAction->setEnabled(false);
	m_refreshAction->setShortcut(QKeySequence::StandardKey::Refresh);
	connect(m_refreshAction, &QAction::triggered, m_equipmentView, &EquipmentView::refreshSelectedDevices);
	addAction(m_refreshAction);

	//-----------------------------------
	m_separatorAction3 = new QAction(this);
	m_separatorAction3->setSeparator(true);

	m_updateFromPresetAction = new QAction(tr("Update from Preset"), this);
	m_updateFromPresetAction->setStatusTip(tr("Update from all object from preset"));
	m_updateFromPresetAction->setEnabled(true);
	connect(m_updateFromPresetAction, &QAction::triggered, m_equipmentView, &EquipmentView::updateFromPreset);

	m_switchModeAction = new QAction(tr("Switch to Preset"), this);
	m_switchModeAction->setStatusTip(tr("Switch to preset/configuration mode"));
	m_switchModeAction->setEnabled(true);
	connect(m_switchModeAction, &QAction::triggered, m_equipmentModel, &EquipmentModel::switchMode);
	connect(m_switchModeAction, &QAction::triggered, this, &EquipmentTabPage::modeSwitched);

    m_connectionsAction = new QAction(tr("Connections..."), this);
    m_connectionsAction->setStatusTip(tr("Edit connections"));
    m_connectionsAction->setEnabled(true);
	connect(m_connectionsAction, &QAction::triggered, this, &EquipmentTabPage::showConnections);

    m_pendingChangesAction = new QAction(tr("Pending Changes..."), this);
	m_pendingChangesAction->setStatusTip(tr("Show pending changes"));
	m_pendingChangesAction->setEnabled(true);
	//connect(m_pendingChangesAction, &QAction::triggered, m_equipmentModel, &EquipmentModel::pendingChanges);
	connect(m_pendingChangesAction, &QAction::triggered, this, &EquipmentTabPage::pendingChanges);

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
	assert(m_addFromPresetAction);
	assert(m_replaceAction);
	assert(m_addNewPresetAction);
	assert(m_addSystemAction);
	assert(m_addSystemAction);
	assert(m_addChassisAction);
	assert(m_addModuleAction);
	assert(m_addControllerAction);
	assert(m_addSignalAction);
	assert(m_addWorkstationAction);
	assert(m_addSoftwareAction);
	assert(m_copyObjectAction);
	assert(m_pasteObjectAction);
	assert(m_deleteObjectAction);
	assert(m_checkOutAction);
	assert(m_checkInAction);
	assert(m_undoChangesAction);
	assert(m_historyAction);
	assert(m_compareAction);
	assert(m_refreshAction);
	assert(m_addPresetRackAction);
	assert(m_addPresetChassisAction);
	assert(m_addPresetModuleAction);
	assert(m_addPresetControllerAction);
	assert(m_addPresetWorkstationAction);
	assert(m_addPresetSoftwareAction);
	assert(m_inOutsToSignals);
	assert(m_showAppSignals);
	assert(m_addAppSignal);
	assert(m_addLogicSchemaToLm);
	assert(m_showLmsLogicSchemas);
	assert(m_addOptoConnection);
	assert(m_showModuleOptoConnections);

	// Check in is always true, as we perform check in is performed for the tree, and there is no iformation
	// about does parent have any checked out files
	//
	m_checkInAction->setEnabled(true);
	m_deleteObjectAction->setEnabled(true);		// Allow to TRY to delete always

	if (isPresetMode() == true)
	{
		m_addNewPresetAction->setVisible(true);
		m_addFromPresetAction->setVisible(false);
		m_replaceAction->setVisible(false);
	}
	else
	{
		m_addNewPresetAction->setVisible(false);
		m_addFromPresetAction->setVisible(true);
		m_replaceAction->setVisible(true);
	}

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

	//m_deleteObjectAction->setEnabled(false);
	m_checkOutAction->setEnabled(false);
	//m_checkInAction->setEnabled(false);			// Check in is always true, as we perform check in is performed for the tree, and there is no iformation
	m_undoChangesAction->setEnabled(false);
	m_historyAction->setEnabled(false);
	m_compareAction->setEnabled(false);
	m_refreshAction->setEnabled(false);

	m_addPresetRackAction->setEnabled(false);
	m_addPresetChassisAction->setEnabled(false);
	m_addPresetModuleAction->setEnabled(false);
	m_addPresetControllerAction->setEnabled(false);
	m_addPresetWorkstationAction->setEnabled(false);
	m_addPresetSoftwareAction->setEnabled(false);

	m_replaceAction->setEnabled(false);

	m_inOutsToSignals->setEnabled(false);
	//m_inOutsToSignals->setVisible(false);

	m_showAppSignals->setEnabled(false);

	m_addAppSignal->setEnabled(false);
	//m_addAppSignal->setVisible(false);

	m_addLogicSchemaToLm->setEnabled(false);
	//m_addLogicSchemaToLm->setVisible(false);

	m_showLmsLogicSchemas->setEnabled(false);
	//m_showLmsLogicSchemas->setVisible(false);

	m_addOptoConnection->setEnabled(false);
	//m_addOptoConnection->setVisible(false);

	m_showModuleOptoConnections->setEnabled(false);
	//m_showModuleOptoConnections->setVisible(false);

	m_copyObjectAction->setEnabled(false);

	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	QModelIndexList selectedIndexList = m_equipmentView->selectionModel()->selectedRows();

	m_addFromPresetAction->setEnabled(selectedIndexList.size() == 1);

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

		if (module != nullptr && (module->isIOModule() == true || module->isLogicModule() == true || module->isOptoModule() == true))
		{
			m_inOutsToSignals->setEnabled(true);
			m_inOutsToSignals->setVisible(true);
		}

		if (module != nullptr && module->isLogicModule())
		{
			m_addAppSignal->setEnabled(true);
			m_addAppSignal->setVisible(true);
		}

		if (device->presetRoot() == true)
		{
			m_replaceAction->setEnabled(true);
		}
	}

	// Add AppLogic Schema to LM
	//
	if (selectedIndexList.size() >= 1)
	{
		bool allSelectedAreLMs = true;
		QString lmDescriptioFile;
		bool lmDescriptioFileInitialized = false;

		for (const QModelIndex& mi : selectedIndexList)
		{
			const Hardware::DeviceObject* device = m_equipmentModel->deviceObject(mi);
			assert(device);

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
					allSelectedAreLMs = false;
					break;
				}

				continue;
			}
			else
			{
				allSelectedAreLMs = false;
				break;
			}
		}

		m_addLogicSchemaToLm->setEnabled(allSelectedAreLMs);
		m_addLogicSchemaToLm->setVisible(allSelectedAreLMs);
	}

	// Show logic schemas for selected LM
	//
	if (selectedIndexList.size() == 1)
	{
		const Hardware::DeviceObject* device = m_equipmentModel->deviceObject(selectedIndexList.front());
		assert(device);

		const Hardware::DeviceModule* module = dynamic_cast<const Hardware::DeviceModule*>(device);

		if (module != nullptr && module->isLogicModule() == true)
		{
			m_showLmsLogicSchemas->setEnabled(true);
			m_showLmsLogicSchemas->setVisible(true);
		}
	}

	// Add Opto Connection with TWO selected Opto Ports
	//
	if (selectedIndexList.size() == 1)
	{
		const Hardware::DeviceController* controller1 = dynamic_cast<const Hardware::DeviceController*>(m_equipmentModel->deviceObject(selectedIndexList.front()));

		if (controller1 != nullptr)
		{
			// If parent is LM or OCM
			//
			const Hardware::DeviceModule* parent1 = dynamic_cast<const Hardware::DeviceModule*>(controller1->parent());

			if (parent1 != nullptr &&
				(parent1->isLogicModule() == true || parent1->moduleFamily() == Hardware::DeviceModule::FamilyType::OCM))
			{
				// If id ends with _OPTOPORTXX
				//
				QString id1 = controller1->equipmentIdTemplate();
				if (id1.size() > 10)
				{
					id1.replace(id1.size() - 2, 2, QLatin1String("##"));
				}

				if (id1.endsWith("_OPTOPORT##") == true)
				{
					m_addOptoConnection->setEnabled(true);
					m_addOptoConnection->setVisible(true);
				}
			}
		}
	}

	if (selectedIndexList.size() == 2)
	{
		const Hardware::DeviceController* controller1 = dynamic_cast<const Hardware::DeviceController*>(m_equipmentModel->deviceObject(selectedIndexList.front()));
		const Hardware::DeviceController* controller2 = dynamic_cast<const Hardware::DeviceController*>(m_equipmentModel->deviceObject(selectedIndexList.back()));

		if (controller1 != nullptr && controller2 != nullptr)
		{
			// If parent is LM or OCM
			//
			const Hardware::DeviceModule* parent1 = dynamic_cast<const Hardware::DeviceModule*>(controller1->parent());
			const Hardware::DeviceModule* parent2 = dynamic_cast<const Hardware::DeviceModule*>(controller2->parent());

			if (parent1 != nullptr && parent2 != nullptr &&
				(parent1->isLogicModule() == true || parent1->moduleFamily() == Hardware::DeviceModule::FamilyType::OCM) &&
				(parent2->isLogicModule() == true || parent2->moduleFamily() == Hardware::DeviceModule::FamilyType::OCM))
			{
				// If id ends with _OPTOPORTXX
				//
				QString id1 = controller1->equipmentIdTemplate();
				if (id1.size() > 10)
				{
					id1.replace(id1.size() - 2, 2, QLatin1String("##"));
				}

				QString id2 = controller2->equipmentIdTemplate();
				if (id2.size() > 10)
				{
					id2.replace(id2.size() - 2, 2, QLatin1String("##"));
				}

				if (id1.endsWith("_OPTOPORT##") == true &&
					id2.endsWith("_OPTOPORT##") == true)
				{
					m_addOptoConnection->setEnabled(true);
					m_addOptoConnection->setVisible(true);
				}
			}
		}
	}

	// Show opto connections for selected LM/OCM or selected opto port
	//
	if (selectedIndexList.size() > 0)
	{
		m_showModuleOptoConnections->setEnabled(true);
		m_showModuleOptoConnections->setVisible(true);
	}

	// Show Application Logic Signal for current object
	//
	if (selectedIndexList.size() > 0)
	{
		m_showAppSignals->setEnabled(true);
	}

	// Delete Items action
	//

	// Allow to delete item always, even when it was already marked as deleted
	//

//	m_deleteObjectAction->setEnabled(false);
//	for (const QModelIndex& mi : selectedIndexList)
//	{
//		const Hardware::DeviceObject* device = m_equipmentModel->deviceObject(mi);
//		assert(device);

//		if (device->fileInfo().state() == VcsState::CheckedIn /*&&
//			device->fileInfo().action() != VcsItemAction::Deleted*/)
//		{
//			m_deleteObjectAction->setEnabled(true);
//			break;
//		}

//		if (device->fileInfo().state() == VcsState::CheckedOut &&
//			(device->fileInfo().userId() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator())
//			&& device->fileInfo().action() != VcsItemAction::Deleted)
//		{
//			m_deleteObjectAction->setEnabled(true);
//			break;
//		}
//	}

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

	//m_checkInAction->setEnabled(canAnyBeCheckedIn);
	m_checkOutAction->setEnabled(canAnyBeCheckedOut);
	m_undoChangesAction->setEnabled(canAnyBeCheckedIn);
	m_historyAction->setEnabled(selectedIndexList.size() == 1);
	m_compareAction->setEnabled(selectedIndexList.size() == 1);

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
				m_addChassisAction->setEnabled(true);
				m_addModuleAction->setEnabled(true);
				m_addControllerAction->setEnabled(true);
				m_addSignalAction->setEnabled(true);
				m_addWorkstationAction->setEnabled(true);

				if (isConfigurationMode() == true)
				{
					m_addPresetChassisAction->setEnabled(true);
					m_addPresetModuleAction->setEnabled(true);
					m_addPresetControllerAction->setEnabled(true);
					m_addPresetWorkstationAction->setEnabled(true);
				}
				break;

			case Hardware::DeviceType::Chassis:
				m_addModuleAction->setEnabled(true);
				m_addControllerAction->setEnabled(true);
				m_addSignalAction->setEnabled(true);
				m_addWorkstationAction->setEnabled(true);

				if (isConfigurationMode() == true)
				{
					m_addPresetModuleAction->setEnabled(true);
					m_addPresetControllerAction->setEnabled(true);
					m_addPresetWorkstationAction->setEnabled(true);
				}
				break;

			case Hardware::DeviceType::Module:
				m_addControllerAction->setEnabled(true);
				m_addSignalAction->setEnabled(true);

				if (isConfigurationMode() == true)
				{
					m_addPresetControllerAction->setEnabled(true);
				}
				break;

			case Hardware::DeviceType::Workstation:
				m_addSoftwareAction->setEnabled(true);
				m_addControllerAction->setEnabled(true);
				m_addSignalAction->setEnabled(true);

				if (isConfigurationMode() == true)
				{
					m_addPresetSoftwareAction->setEnabled(true);
					m_addPresetControllerAction->setEnabled(true);
				}
				break;

			case Hardware::DeviceType::Software:
				m_addControllerAction->setEnabled(true);
				m_addSignalAction->setEnabled(true);

				if (isConfigurationMode() == true)
				{
					m_addPresetControllerAction->setEnabled(true);
				}
				break;

			case Hardware::DeviceType::Controller:
				m_addSignalAction->setEnabled(true);
				break;

			case Hardware::DeviceType::Signal:
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

	// Copy to the clipboard
	//
	if (selectedIndexList.empty() == false)
	{

		bool allowCopyToClipboard = true;	// allow copy if all selected objects are the same type

		const Hardware::DeviceObject* device = m_equipmentModel->deviceObject(selectedIndexList.first());
		assert(device);

		Hardware::DeviceType type = device->deviceType();

		for (const QModelIndex& mi : selectedIndexList)
		{
			const Hardware::DeviceObject* device = m_equipmentModel->deviceObject(mi);
			assert(device);

			// System can be very big so forbid it's copying
			//
			if (device->isSystem() == true)
			{
				allowCopyToClipboard = false;
				break;
			}

			// all selected objects must be the same type
			//
			if (type != device->deviceType())
			{
				allowCopyToClipboard = false;
				break;
			}

			// In ConfigurationMode it is possible to copy only root items of preset items
			//
			if (isConfigurationMode() == true &&
				device->preset() == true &&
				device->presetRoot() == false)
			{
				allowCopyToClipboard = false;
				break;
			}
		}

		m_copyObjectAction->setEnabled(allowCopyToClipboard);
	}

	// Update paste
	//
	bool enablepaste = m_equipmentView->canPaste();
	m_pasteObjectAction->setEnabled(enablepaste);

	return;
}

void EquipmentTabPage::clipboardChanged()
{
	bool enablepaste = m_equipmentView->canPaste();
	m_pasteObjectAction->setEnabled(enablepaste);

	return;
}


void EquipmentTabPage::modeSwitched()
{
	if (m_equipmentModel->isPresetMode() == true)
	{
		m_switchModeAction->setText(tr("Switch to Configuration"));

		m_updateFromPresetAction->setEnabled(false);
	}
	else
	{
		m_switchModeAction->setText(tr("Switch to Preset"));

		m_updateFromPresetAction->setEnabled(true);
	}

	setActionState();

	return;
}

void EquipmentTabPage::showConnections()
{
    if (dbController()->isProjectOpened() == false)
    {
        return;
    }

    if (theDialogConnections == nullptr)
	{
        theDialogConnections = new DialogConnections(dbController(), this);
        theDialogConnections->show();
	}
	else
	{
        theDialogConnections->activateWindow();
	}

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

    QList<std::shared_ptr<PropertyObject>> checkedInList;
    QList<std::shared_ptr<PropertyObject>> checkedOutList;

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

	m_propertyEditor->setExpertMode(isPresetMode() || theSettings.isExpertMode());
	m_propertyEditor->setReadOnly(checkedOutList.isEmpty() == true);

	// Set objects to the PropertyEditor
	//
	if (checkedOutList.isEmpty() == false)
	{
		m_propertyEditor->setObjects(checkedOutList);
	}
	else
	{
		m_propertyEditor->setObjects(checkedInList);
	}

	return;
}

void EquipmentTabPage::propertiesChanged(QList<std::shared_ptr<PropertyObject>> objects)
{
	// Refresh property "EquipmentID", as it depends on "EquipmentIDTemplate"
	//
	if (m_propertyEditor != nullptr)
	{
		m_propertyEditor->updatePropertyValues("EquipmentID");
	}

	// --
	//
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

		file->setDetails(device->details());

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

void EquipmentTabPage::addObjectTriggered()
{
	if (m_toolBar == nullptr)
	{
		assert(m_toolBar);
		return;
	}

	QWidget* w = m_toolBar->widgetForAction(m_addObjectAction);

	if (w == nullptr)
	{
		assert(w);
		return;
	}

	QPoint pt = w->pos();
	pt.ry() += w->height();

	m_addObjectMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

void EquipmentTabPage::addNewPresetTriggered()
{
	if (m_toolBar == nullptr)
	{
		assert(m_toolBar);
		return;
	}

	QWidget* w = m_toolBar->widgetForAction(m_addNewPresetAction);

	if (w == nullptr)
	{
		assert(w);
		return;
	}

	QPoint pt = w->pos();
	pt.ry() += w->height();

	m_addPresetMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

void EquipmentTabPage::pendingChanges()
{
	EquipmentVcsDialog d(db(), this);
	d.exec();
}

void EquipmentTabPage::objectVcsStateChanged()
{
	setActionState();
	setProperties();
	return;
}

void EquipmentTabPage::compareObject(DbChangesetObject object, CompareData compareData)
{
	// Can compare only files which are EquipmentObjects
	//
	if (object.isFile() == false)
	{
		return;
	}

	// Check file extension
	//
	bool extFound = false;
	QString fileName = object.name();

	for (const QString& ext : Hardware::DeviceObjectExtensions)
	{
		if (fileName.endsWith(ext) == true)
		{
			extFound = true;
			break;
		}
	}

	if (extFound == false)
	{
		return;
	}

	// Get vesrions from the project database
	//
	std::shared_ptr<Hardware::DeviceObject> source = nullptr;

	switch (compareData.sourceVersionType)
	{
	case CompareVersionType::Changeset:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.sourceChangeset, &outFile, this);
			if (ok == true)
			{
				source = Hardware::DeviceObject::Create(outFile->data());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.sourceDate, &outFile, this);
			if (ok == true)
			{
				source = Hardware::DeviceObject::Create(outFile->data());
			}
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getLatestVersion(file, &outFile, this);
			if (ok == true)
			{
				source = Hardware::DeviceObject::Create(outFile->data());
			}
		}
		break;
		break;
	default:
		assert(false);
	}

	if (source == nullptr)
	{
		return;
	}

	// Get target file version
	//
	std::shared_ptr<Hardware::DeviceObject> target = nullptr;

	switch (compareData.targetVersionType)
	{
	case CompareVersionType::Changeset:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.targetChangeset, &outFile, this);
			if (ok == true)
			{
				target = Hardware::DeviceObject::Create(outFile->data());
			}
		}
		break;
	case CompareVersionType::Date:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getSpecificCopy(file, compareData.targetDate, &outFile, this);
			if (ok == true)
			{
				target = Hardware::DeviceObject::Create(outFile->data());
			}
		}
		break;
	case CompareVersionType::LatestVersion:
		{
			DbFileInfo file;
			file.setFileId(object.id());

			std::shared_ptr<DbFile> outFile;

			bool ok = db()->getLatestVersion(file, &outFile, this);
			if (ok == true)
			{
				target = Hardware::DeviceObject::Create(outFile->data());
			}
		}
		break;
	default:
		assert(false);
	}

	if (target == nullptr)
	{
		return;
	}

	// Compare
	//
	ComparePropertyObjectDialog::showDialog(object, compareData, source, target, this);

	return;
}
