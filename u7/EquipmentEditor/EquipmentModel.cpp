#include "EquipmentModel.h"
#include "../../lib/DbController.h"
#include "../../lib/StandardColors.h"
#include "../GlobalMessanger.h"
#include "../CheckInDialog.h"


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

	{
		auto fileInfo = std::make_shared<DbFileInfo>();
		m_configuration->setData(fileInfo);
	}

	{
		auto fileInfo = std::make_shared<DbFileInfo>();
		m_preset->setData(fileInfo);
	}

	m_root = m_configuration;	// Edit configuration default mode

	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &EquipmentModel::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &EquipmentModel::projectClosed);
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
		return createIndex(row, column, const_cast<Hardware::DeviceObject*>(m_root->child(row).get()));
	}

	Hardware::DeviceObject* parent = static_cast<Hardware::DeviceObject*>(parentIndex.internalPointer());

	if (parent == nullptr)
	{
		assert(parent);
		return QModelIndex();
	}

	QModelIndex resultIndex = createIndex(row, column, parent->child(row).get());
	return resultIndex;
}

QModelIndex EquipmentModel::parent(const QModelIndex& childIndex) const
{
	if (childIndex.isValid() == false)
	{
		return QModelIndex();
	}

	std::shared_ptr<Hardware::DeviceObject> child = static_cast<Hardware::DeviceObject*>(childIndex.internalPointer())->sharedPtr();
	if (child == nullptr)
	{
		assert(child != nullptr);
		return QModelIndex();
	}

	if (child->parent() == nullptr || child->parent() == m_root)
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

		return createIndex(row, 0, child->parent().get());
	}
	else
	{
		int row = child->parent()->parent()->childIndex(child->parent());
		if (row == -1)
		{
			assert(row != -1);
			return QModelIndex();
		}

		return createIndex(row, 0, child->parent().get());
	}
}

int EquipmentModel::rowCount(const QModelIndex& parentIndex) const
{
	std::shared_ptr<const Hardware::DeviceObject> parent = deviceObject(parentIndex);

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

	const DbFileInfo* devieFileInfo = fileInfo(device);
	Q_ASSERT(devieFileInfo);

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

			case ObjectTypeColumn:
				v.setValue<QString>(Hardware::DeviceTypeNames[static_cast<size_t>(device->deviceType())]);
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
				if (devieFileInfo->state() == VcsState::CheckedOut)
				{
					QString state = devieFileInfo->action().text();
					v.setValue<QString>(state);
				}
				break;

			case ObjectUserColumn:
				if (devieFileInfo->state() == VcsState::CheckedOut)
				{
					v.setValue<QString>(usernameById(devieFileInfo->userId()));
				}
				break;

			default:
				assert(false);
			}

			return v;
		}
		break;

	case Qt::TextAlignmentRole:
		return Qt::AlignLeft + Qt::AlignVCenter;

	case Qt::ForegroundRole:
		return QBrush{index.column() == ObjectTypeColumn ?
						Qt::darkGray :
						Qt::black};

	case Qt::BackgroundRole:
		{
			if (devieFileInfo->state() == VcsState::CheckedOut)
			{
				QBrush b(StandardColors::VcsCheckedIn);

				switch (static_cast<VcsItemAction::VcsItemActionType>(devieFileInfo->action().toInt()))
				{
				case VcsItemAction::Unknown:
					b.setColor(StandardColors::VcsCheckedIn);
					break;
				case VcsItemAction::Added:
					b.setColor(StandardColors::VcsAdded);
					break;
				case VcsItemAction::Modified:
					b.setColor(StandardColors::VcsModified);
					break;
				case VcsItemAction::Deleted:
					b.setColor(StandardColors::VcsDeleted);
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

			case ObjectTypeColumn:
				return QObject::tr("Type");

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

	std::shared_ptr<const Hardware::DeviceObject> object = deviceObject(parentIndex);
	if (object->childrenCount() > 0)
	{
		return true;	// Already have file list for this object
	}

	if (object->deviceType() == Hardware::DeviceType::AppSignal ||
		object->deviceType() == Hardware::DeviceType::DiagSignal)
	{
		return false;	// DeviceType::DiagSignal cannot have children
	}

	bool hasChildren = false;


	DbFileInfo fi = *fileInfo(object.get());

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

	std::shared_ptr<const Hardware::DeviceObject> object = deviceObject(parent);

	if (object->childrenCount() > 0)
	{
		return false;	// seems that we already got file list for this object
	}

	if (object->deviceType() == Hardware::DeviceType::AppSignal ||
		object->deviceType() == Hardware::DeviceType::DiagSignal)
	{
		return false;	// AppSignal/DiagSignal cannot have children
	}

	bool hasChildren = false;
	DbFileInfo fi = *fileInfo(object.get());

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

	std::shared_ptr<Hardware::DeviceObject> parentObject = deviceObject(const_cast<QModelIndex&>(parentIndex));

	const DbFileInfo* parentObjectFileInfo = parentObject->data();
	Q_ASSERT(parentObjectFileInfo);

	std::vector<DbFileInfo> files;

	bool ok = dbController()->getFileList(&files, parentObjectFileInfo->fileId(), true, m_parentWidget);
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

		auto objectFileInfo = std::make_shared<DbFileInfo>(fi, object->details());
		object->setData(objectFileInfo);

		parentObject->addChild(object);
	}

	sortDeviceObject(parentObject, m_sortColumn, m_sortOrder);

	endInsertRows();

	return;
}

void EquipmentModel::sortDeviceObject(std::shared_ptr<Hardware::DeviceObject>& object, int column, Qt::SortOrder order)
{
	if (object == nullptr)
	{
		assert(object);
		return;
	}

	switch (column)
	{
	case ObjectNameColumn:
		sortChildrenByCaption(object, order);
		break;
	case ObjectTypeColumn:
		sortChildrenByType(object, order);
		break;
	case ObjectEquipmentIdColumn:
		sortChildrenByEquipmentId(object, order);
		break;
	case ObjectPlaceColumn:
		sortChildrenByPlace(object, order);
		break;
	case ObjectStateColumn:
		sortChildrenByState(object, order);
		break;
	case ObjectUserColumn:
		sortChildrenByUser(object, order);
		break;
	default:
		assert(false);
	}

	int childCont = object->childrenCount();

	for (int i = 0; i < childCont; i++)
	{
		std::shared_ptr<Hardware::DeviceObject> child = object->child(i);

		if (child->deviceType() != Hardware::DeviceType::AppSignal &&
			child->deviceType() != Hardware::DeviceType::DiagSignal)
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
	sortDeviceObject(m_configuration, column, order);
	sortDeviceObject(m_preset, column, order);

	// Move pers indexes
	//
	for (QModelIndex& oldIndex : pers)
	{
		std::shared_ptr<Hardware::DeviceObject> device = deviceObject(oldIndex);
		assert(device);

		std::shared_ptr<Hardware::DeviceObject> parentDevice = device->parent();
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
	std::shared_ptr<Hardware::DeviceObject> parent = deviceObject(parentIndex);

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

void EquipmentModel::deleteDeviceObject(const QModelIndexList& rowList)
{
	std::vector<Hardware::DeviceObject*> devices;
	devices.reserve(16);

	for (QModelIndex index : rowList)
	{
		std::shared_ptr<Hardware::DeviceObject> d = deviceObject(index);
		assert(d);

		devices.push_back(d.get());
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

	std::sort(sortedRowList.begin(), sortedRowList.end(),
			  [this](QModelIndex& m1, QModelIndex m2)
			  {
					std::shared_ptr<Hardware::DeviceObject> d1 = deviceObject(m1);
					std::shared_ptr<Hardware::DeviceObject> d2 = deviceObject(m2);

					const DbFileInfo* fileInfo1 = d1->data();
					const DbFileInfo* fileInfo2 = d2->data();
					Q_ASSERT(fileInfo1);
					Q_ASSERT(fileInfo2);

					return fileInfo1->fileId() >= fileInfo2->fileId();
			  });

	// Update model
	//
	for (QModelIndex& index : sortedRowList)
	{
		std::shared_ptr<Hardware::DeviceObject> d = deviceObject(index);
		assert(d);

		const DbFileInfo* fileInfo = d->data();
		Q_ASSERT(fileInfo);

		if (fileInfo->deleted() == true)
		{
			QModelIndex pi = index.parent();
			std::shared_ptr<Hardware::DeviceObject> po = deviceObject(pi);
			assert(po);

			int childIndex = po->childIndex(d);
			assert(childIndex != -1);

			beginRemoveRows(pi, childIndex, childIndex);
			po->deleteChild(d);
			endRemoveRows();
		}
		else
		{
			QModelIndex bottomRightIndex = this->index(index.row(), ColumnCount - 1, index.parent());
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
	std::shared_ptr<Hardware::DeviceObject> device = this->deviceObject(modelIndex);
	assert(device);

	const DbFileInfo* deviceFileInfo = device->data();
	Q_ASSERT(deviceFileInfo);
	assert(deviceFileInfo->fileId() != -1);

	// Update children first, as items can be deleted
	//
	for (int childRow = device->childrenCount() - 1; childRow >= 0; childRow--)
	{
		if (updatedModelIndexes.find(device->child(childRow).get()) != updatedModelIndexes.end())
		{
			// skip, it was already processed
			//
			continue;
		}

		QModelIndex childIndex = this->index(childRow, 0, modelIndex);
		if (childIndex.isValid() == false)
		{
			assert(childIndex.isValid() == true);
			break;
		}

#ifdef QT_DEBUG
		std::shared_ptr<Hardware::DeviceObject> childDevice = this->deviceObject(childIndex);
		assert(childDevice);
		assert(childDevice == device->child(childRow));
#endif

		updateRowFuncOnCheckIn(childIndex, updateFiles, updatedModelIndexes);
	}

	// Update current ModelIndex, Check if thes device was cgecked out
	//
	auto foundFileInfo = updateFiles.find(deviceFileInfo->fileId());

	if (foundFileInfo != std::end(updateFiles))
	{
		assert(foundFileInfo->second.fileId() == deviceFileInfo->fileId());

		auto newFileInfo = std::make_shared<DbFileInfo>(foundFileInfo->second, device->details());
		device->setData(newFileInfo);

		if (newFileInfo->deleted() == true ||
			(newFileInfo->action() == VcsItemAction::Deleted && newFileInfo->state() == VcsState::CheckedIn))
		{
			QModelIndex pi = modelIndex.parent();
			std::shared_ptr<Hardware::DeviceObject> po = this->deviceObject(pi);
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
		std::shared_ptr<Hardware::DeviceObject> d = deviceObject(index);
		assert(d);

		const DbFileInfo* fi = d->data();
		Q_ASSERT(fi);

		files.push_back(*fi);
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

	for (QModelIndex& index : rowList)
	{
		std::shared_ptr<Hardware::DeviceObject> d = deviceObject(index);
		Q_ASSERT(d);

		const DbFileInfo* fi = d->data();
		Q_ASSERT(fi);

		if (fi->state() == VcsState::CheckedIn)
		{
			files.push_back(*fi);
			checkedInList.push_back(index);

			objects.push_back(d);
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
		std::shared_ptr<Hardware::DeviceObject> d = deviceObject(index);
		if (d == nullptr)
		{
			Q_ASSERT(d);
			continue;
		}

		const DbFileInfo* dfi = d->data();
		Q_ASSERT(dfi);

		// Update object
		//
		auto freshFileIt = std::find_if(freshFiles.begin(), freshFiles.end(),
				[dfi](const std::shared_ptr<DbFile>& f)
				{
					return dfi->fileId() == f->fileId();
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
			if (fi.fileId() == dfi->fileId())
			{
				auto newFileInfo = std::make_shared<DbFileInfo>(fi, d->details());
				d->setData(newFileInfo);		// Update file info record in the DeviceOubject

				if (newFileInfo->state() == VcsState::CheckedOut)
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
	std::sort(rowList.begin(), rowList.end(),
		[this](const QModelIndex& m1, const QModelIndex& m2)
		{
			std::shared_ptr<const Hardware::DeviceObject> d1 = this->deviceObject(m1);
			std::shared_ptr<const Hardware::DeviceObject> d2 = this->deviceObject(m2);

			const DbFileInfo* fileInfo1 = d1->data();
			const DbFileInfo* fileInfo2 = d2->data();

			Q_ASSERT(fileInfo1);
			Q_ASSERT(fileInfo2);

			return fileInfo1->fileId() >= fileInfo2->fileId();
		});

	// --
	//
	std::vector<DbFileInfo> files;
	QModelIndexList checkedOutList;
	DbUser currentUser = dbController()->currentUser();

	for (QModelIndex& index : rowList)
	{
		std::shared_ptr<Hardware::DeviceObject> d = deviceObject(index);
		assert(d);

		const DbFileInfo* fileInfo = d->data();
		Q_ASSERT(fileInfo);

		if (fileInfo->state() == VcsState::CheckedOut &&
			(fileInfo->userId() == currentUser.userId() || currentUser.isAdminstrator() == true))
		{
			files.push_back(*fileInfo);
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
		std::shared_ptr<Hardware::DeviceObject> d = deviceObject(index);
		assert(d);

		const DbFileInfo* fileInfo = d->data();
		Q_ASSERT(fileInfo);

		// Set latest version to the object
		//
		auto foundFile = std::find_if(latestFilesVersion.begin(), latestFilesVersion.end(),
					[fileInfo](const std::shared_ptr<DbFile>& f)
					{
						return f->fileId() == fileInfo->fileId();
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
			if (fi.fileId() == fileInfo->fileId())
			{
				auto newFileInfo = std::make_shared<DbFileInfo>(fi, d->details());
				d->setData(newFileInfo);

				if (fi.deleted() == true)
				{
					QModelIndex pi = index.parent();
					std::shared_ptr<Hardware::DeviceObject> po = deviceObject(pi);
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
		std::shared_ptr<Hardware::DeviceObject> d = deviceObject(index);
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
	std::set<std::shared_ptr<Hardware::DeviceObject>> updatedParents;

	for (QModelIndex& i : rowList)
	{
		QModelIndex parentIndex = i.parent();

		std::shared_ptr<Hardware::DeviceObject> parentDevice = deviceObject(parentIndex);
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
		std::shared_ptr<Hardware::DeviceObject> device = deviceObject(oldIndex);
		assert(device);

		std::shared_ptr<Hardware::DeviceObject> parentDevice = device->parent();
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

std::shared_ptr<Hardware::DeviceObject> EquipmentModel::deviceObject(QModelIndex& index)
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
	return object->sharedPtr();
}

std::shared_ptr<const Hardware::DeviceObject> EquipmentModel::deviceObject(const QModelIndex& index) const
{
	std::shared_ptr<const Hardware::DeviceObject> object;

	if (index.isValid() == false)
	{
		object = m_root;
	}
	else
	{
		object = static_cast<const Hardware::DeviceObject*>(index.internalPointer())->sharedPtr();
	}

	assert(object != nullptr);
	assert(object->isRoot() == true || (object->isRoot() == false && object->parent() != nullptr));
	assert(object->isRoot() == true || (object->isRoot() == false && object->parent()->childIndex(object) != -1));

	return object->sharedPtr();
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

void EquipmentModel::sortChildrenByCaption(std::shared_ptr<Hardware::DeviceObject> deviceObject, Qt::SortOrder order)
{
	auto children = deviceObject->children();

	std::sort(std::begin(children), std::end(children),
		[order](const auto& o1, const auto& o2)
		{
			const auto& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
			const auto& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

			if (ref1->caption() < ref2->caption())
			{
				return true;
			}
			else
			{
				if (ref1->caption() == ref2->caption())
				{
					return ref1->place() < ref2->place();
				}
				else
				{
					return false;
				}
			}
		});

	// Set children back in new order
	//
	deviceObject->deleteAllChildren();

	std::ranges::for_each(children, [&deviceObject](auto ch){	deviceObject->addChild(ch);	});

	return;
}

void EquipmentModel::sortChildrenByType(std::shared_ptr<Hardware::DeviceObject> deviceObject, Qt::SortOrder order)
{
	auto children = deviceObject->children();

	std::sort(std::begin(children), std::end(children),
		[order](const auto& o1, const auto& o2)
		{
			const auto& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
			const auto& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

			if (ref1->deviceType() < ref2->deviceType())
			{
				return true;
			}
			else
			{
				if (ref1->deviceType() == ref2->deviceType())
				{
					return ref1->equipmentIdTemplate() < ref2->equipmentIdTemplate();
				}
				else
				{
					return false;
				}
			}
		});

	// Set children back in new order
	//
	deviceObject->deleteAllChildren();

	std::ranges::for_each(children, [&deviceObject](auto ch){	deviceObject->addChild(ch);	});

	return;
}

void EquipmentModel::sortChildrenByEquipmentId(std::shared_ptr<Hardware::DeviceObject> deviceObject, Qt::SortOrder order)
{
	auto children = deviceObject->children();

	std::sort(std::begin(children), std::end(children),
		[order](const auto& o1, const auto& o2)
		{
			const auto& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
			const auto& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

			if (ref1->equipmentIdTemplate() < ref2->equipmentIdTemplate())
			{
				return true;
			}
			else
			{
				if (ref1->equipmentIdTemplate() == ref2->equipmentIdTemplate())
				{
					return ref1->place() < ref2->place();
				}
				else
				{
					return false;
				}
			}
		});

	// Set children back in new order
	//
	deviceObject->deleteAllChildren();

	std::ranges::for_each(children, [&deviceObject](auto ch){	deviceObject->addChild(ch);	});

	return;
}

void EquipmentModel::sortChildrenByPlace(std::shared_ptr<Hardware::DeviceObject> deviceObject, Qt::SortOrder order)
{
	auto children = deviceObject->children();

	std::sort(std::begin(children), std::end(children),
		[order](const auto& o1, const auto& o2)
		{
			const auto& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
			const auto& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

			if (ref1->place() < ref2->place())
			{
				return true;
			}
			else
			{
				if (ref1->place() == ref2->place())
				{
					return ref1->equipmentIdTemplate() < ref2->equipmentIdTemplate();
				}
				else
				{
					return false;
				}
			}
		});

	// Set children back in new order
	//
	deviceObject->deleteAllChildren();

	std::ranges::for_each(children, [&deviceObject](auto ch){	deviceObject->addChild(ch);	});

	return;
}

void EquipmentModel::sortChildrenByState(std::shared_ptr<Hardware::DeviceObject> deviceObject, Qt::SortOrder order)
{
	auto children = deviceObject->children();

	std::sort(std::begin(children), std::end(children),
		[order](const auto& o1, const auto& o2)
		{
			const auto& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
			const auto& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

			const DbFileInfo* fileInfo1 = ref1->data();
			const DbFileInfo* fileInfo2 = ref2->data();
			Q_ASSERT(fileInfo1);
			Q_ASSERT(fileInfo2);

			if (fileInfo1->state() < fileInfo2->state())
			{
				return true;
			}
			else
			{
				if (fileInfo1->state() == fileInfo2->state())
				{
					return ref1->equipmentIdTemplate() < ref2->equipmentIdTemplate();
				}
				else
				{
					return false;
				}
			}
		});

	// Set children back in new order
	//
	deviceObject->deleteAllChildren();

	std::ranges::for_each(children, [&deviceObject](auto ch){	deviceObject->addChild(ch);	});

	return;
}

void EquipmentModel::sortChildrenByUser(std::shared_ptr<Hardware::DeviceObject> deviceObject, Qt::SortOrder order)
{
	auto children = deviceObject->children();

	std::sort(std::begin(children), std::end(children),
		[order, this](const auto& o1, const auto& o2)
		{
			const auto& ref1 = (order == Qt::AscendingOrder ? o1 : o2);
			const auto& ref2 = (order == Qt::AscendingOrder ? o2 : o1);

			const DbFileInfo* fileInfo1 = ref1->data();
			const DbFileInfo* fileInfo2 = ref2->data();
			Q_ASSERT(fileInfo1);
			Q_ASSERT(fileInfo2);

			auto uit1 = this->m_users.find(fileInfo1->userId());
			auto uit2 = this->m_users.find(fileInfo2->userId());

			QString u1 =  uit1 == this->m_users.end() ? "Unknown" : uit1->second;
			QString u2 =  uit2 == this->m_users.end() ? "Unknown" : uit2->second;

			if (u1 < u2)
			{
				return true;
			}
			else
			{
				if (u1 == u2)
				{
					return ref1->equipmentIdTemplate() < ref2->equipmentIdTemplate();
				}
				else
				{
					return false;
				}
			}

		});

	// Set children back in new order
	//
	deviceObject->deleteAllChildren();

	std::ranges::for_each(children, [&deviceObject](auto ch){	deviceObject->addChild(ch);	});

	return;
}

void EquipmentModel::projectOpened()
{
	if (dbController()->isProjectOpened() == false)
	{
		assert(dbController()->isProjectOpened() == true);
		return;
	}

	beginResetModel();

	DbFileInfo* confFileInfo = m_configuration->data();
	DbFileInfo* presetFileInfo = m_preset->data();

	Q_ASSERT(confFileInfo);
	Q_ASSERT(presetFileInfo);

	int hcFileId = dbController()->systemFileId(DbDir::HardwareConfigurationDir);
	int hpFileId = dbController()->systemFileId(DbDir::HardwarePresetsDir);

	confFileInfo->setFileId(hcFileId);
	presetFileInfo->setFileId(hpFileId);

	// Fill user list
	//
	updateUserList();

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

void EquipmentModel::updateUserList()
{
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

}

const DbFileInfo* EquipmentModel::fileInfo(const Hardware::DeviceObject* deviceObject)
{
	Q_ASSERT(deviceObject);

	auto fileInfo = deviceObject->data();
	Q_ASSERT(fileInfo);

	return fileInfo;
}

const DbFileInfo* EquipmentModel::fileInfo(const std::shared_ptr<Hardware::DeviceObject>& deviceObject)
{
	return fileInfo(deviceObject.get());
}

void EquipmentModel::setFileInfo(Hardware::DeviceObject* deviceObject, const DbFileInfo& fileInfo)
{
	Q_ASSERT(deviceObject);

	auto deviceFileInfo = deviceObject->data();
	Q_ASSERT(deviceFileInfo);

	if (deviceFileInfo != nullptr)
	{
		*deviceFileInfo = fileInfo;
	}

	return;
}

void EquipmentModel::setFileInfo(std::shared_ptr<Hardware::DeviceObject> deviceObject, const DbFileInfo& fileInfo)
{
	return setFileInfo(deviceObject.get(), fileInfo);
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
