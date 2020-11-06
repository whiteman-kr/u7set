#include "FilesTreeView.h"
#include "../lib/DbController.h"
#include "GlobalMessanger.h"
#include "DialogFileEditor.h"
#include "CheckInDialog.h"
#include "Forms/FileHistoryDialog.h"
#include "Forms/CompareDialog.h"
#include "../lib/StandardColors.h"
#include "../lib/Ui/DbControllerTools.h"

//
//
// FileTreeModelItem
//
//
FileTreeModelItem::FileTreeModelItem()
{
}

FileTreeModelItem::FileTreeModelItem(const DbFileInfo& file) :
	DbFileInfo(file)
{
}

FileTreeModelItem::~FileTreeModelItem()
{
}

FileTreeModelItem* FileTreeModelItem::parent()
{
	return m_parent;
}

int FileTreeModelItem::childrenCount() const
{
	return static_cast<int>(m_children.size());
}

FileTreeModelItem* FileTreeModelItem::child(int index) const
{
	return m_children.at(index).get();
}

int FileTreeModelItem::childIndex(FileTreeModelItem* child) const
{
	auto fr = std::find_if(m_children.begin(), m_children.end(),
						   [child](const std::shared_ptr<FileTreeModelItem>& v)
	{
		return v.get() == child;
	});

	if (fr == m_children.end())
	{
		return -1;
	}

	int result = static_cast<int>(std::distance(m_children.begin(), fr));
	return result;
}

FileTreeModelItem* FileTreeModelItem::childByFileId(int fileId) const
{
	auto fr = std::find_if(m_children.begin(), m_children.end(),
						   [fileId](const std::shared_ptr<FileTreeModelItem>& v)
	{
		return v.get()->fileId() == fileId;
	});

	if (fr == m_children.end())
	{
		return nullptr;
	}

	return fr->get();
}

std::shared_ptr<FileTreeModelItem> FileTreeModelItem::childSharedPtr(int index)
{
	std::shared_ptr<FileTreeModelItem> sp = m_children.at(index);
	return sp;
}

void FileTreeModelItem::expandChildFilesToArray(std::vector<FileTreeModelItem*>& result)
{
	for (std::shared_ptr<FileTreeModelItem> child : m_children)
	{
		if (child->isFolder() == false)
		{
			result.push_back(child.get());
		}

		child->expandChildFilesToArray(result);
	}
}

void FileTreeModelItem::addChild(std::shared_ptr<FileTreeModelItem> child)
{
	child->m_parent = this;
	m_children.push_back(child);
}

void FileTreeModelItem::deleteChild(FileTreeModelItem* child)
{
	auto found = std::find_if(m_children.begin(), m_children.end(), [child](decltype(m_children)::const_reference c)
		{
			return c.get() == child;
		});

	if (found == m_children.end())
	{
		assert(found != m_children.end());
		return;
	}

	m_children.erase(found);
	return;
}

void FileTreeModelItem::deleteAllChildren()
{
	m_children.clear();
}

bool FileTreeModelItem::fetched() const
{
	return m_fetched;
}

void FileTreeModelItem::setFetched()
{
	m_fetched = true;
}

//
// FileTreeProxyModel
//

FileTreeProxyModel::FileTreeProxyModel(FileTreeModel* sourceModel, QObject *parent):
	QSortFilterProxyModel(parent),
	m_sourceModel(sourceModel)
{

}

QModelIndexList FileTreeProxyModel::getPersistentIndexList() const
{
	return persistentIndexList();
}

bool FileTreeProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	QString filter = filterRegExp().pattern();
	if (filter.isEmpty() == true)
	{
		return true;
	}

	QModelIndex index = m_sourceModel->childIndex(sourceRow, filterKeyColumn(), sourceParent);

	FileTreeModelItem* item = m_sourceModel->fileItem(index);
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return false;
	}


	if (item->isFolder() == true)
	{
		std::vector<FileTreeModelItem*> recursiveChildren;
		item->expandChildFilesToArray(recursiveChildren);

		for (FileTreeModelItem* recursiveItem : recursiveChildren)
		{
			if (recursiveItem->fileName().contains(filter, Qt::CaseInsensitive) == true)
			{
				return true;
			}
		}
	}
	else
	{
		if (item->fileName().contains(filter, Qt::CaseInsensitive) == true)
		{
			return true;
		}
	}

	return false;
}

bool FileTreeProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	if (left.isValid() == false || right.isValid() == false)
	{
		return false;
	}

	const FileTreeModelItem* f1 = m_sourceModel->fileItem(left);
	const FileTreeModelItem* f2 = m_sourceModel->fileItem(right);

	if (f1 == nullptr || f2 == nullptr)
	{
		Q_ASSERT(f1);
		Q_ASSERT(f2);
		return false;
	}

	// Folders first

	if (f1->isFolder() != f2->isFolder())
	{
		if (sortOrder() == Qt::AscendingOrder)
		{
			return f1->isFolder() > f2->isFolder();
		}
		else
		{
			return f1->isFolder() < f2->isFolder();
		}
	}

	FileTreeModel::Columns column = m_sourceModel->columnAtIndex(left.column());

	switch (static_cast<FileTreeModel::Columns>(column))
	{
	case FileTreeModel::Columns::FileIdColumn:
		{
			if (f1->fileId() != f2->fileId())
			{
				return f1->fileId() < f2->fileId();
			}
			Q_ASSERT(false);
		}
		//break;
	case FileTreeModel::Columns::FileAttributesColumn:
		{
			if (f1->attributes() != f2->attributes())
			{
				return f1->attributes() < f2->attributes();
			}
		}
		//break;
	case FileTreeModel::Columns::FileSizeColumn:
		{
			if (f1->size() != f2->size())
			{
				return f1->size() < f2->size();
			}
		}
		//break;
	case FileTreeModel::Columns::FileUserColumn:
		{
			if ( m_sourceModel->db()->username(f1->userId()) !=  m_sourceModel->db()->username(f2->userId()))
			{
				return m_sourceModel->db()->username(f1->userId()) < m_sourceModel->db()->username(f2->userId());
			}
		}
		//break;
	case FileTreeModel::Columns::FileStateColumn:
		{
			if (f1->state() != f2->state())
			{
				return f1->state() < f2->state();
			}

			if (f1->action().toInt() != f2->action().toInt())
			{
				return f1->action().toInt() < f2->action().toInt();
			}
		}
		//break;
	case FileTreeModel::Columns::FileNameColumn:
		{
			return f1->fileName() < f2->fileName();
		}
		break;
	}

	// Custom column
	return QSortFilterProxyModel::lessThan(left, right);
}

//
//
// FileTreeModel
//
//
FileTreeModel::FileTreeModel(DbController* dbcontroller, QString rootFilePath, QWidget* parentWidget, QObject* parent) :
	QAbstractItemModel(parent),
	m_dbc(dbcontroller),
	m_rootFilePath(rootFilePath),
	m_parentWidget(parentWidget),
	m_root(std::make_shared<FileTreeModelItem>())
{
	assert(m_dbc);

	for (int c = static_cast<int>(Columns::FileNameColumn); c < static_cast<int>(Columns::StandardColumnCount); c++)
	{
		m_columns.push_back(static_cast<Columns>(c));
	}

	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &FileTreeModel::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &FileTreeModel::projectClosed);
}

FileTreeModel::~FileTreeModel()
{
}

void FileTreeModel::setColumns(std::vector<Columns> columns)
{
	if (m_columns.size() > 0)
	{
		beginRemoveColumns(QModelIndex(), 0, static_cast<int>(m_columns.size() - 1));
		m_columns.clear();
		endRemoveColumns();
	}

	if (columns.size() > 0)
	{
		beginInsertColumns(QModelIndex(), 0, static_cast<int>(columns.size() - 1));
		m_columns = columns;
		endInsertColumns();
	}
}

FileTreeModel::Columns FileTreeModel::columnAtIndex(int index) const
{
	if (index < 0 || index >= static_cast<int>(m_columns.size()))
	{
		Q_ASSERT(false);
		return Columns::FileNameColumn;
	}

	return m_columns[index];
}

int FileTreeModel::childCount(const QModelIndex& parentIndex) const
{
	return rowCount(parentIndex);
}

QModelIndex FileTreeModel::childIndex(int row, int column, const QModelIndex& parentIndex) const
{
	return index(row, column, parentIndex);

}

QModelIndex FileTreeModel::childParent(const QModelIndex& childIndex) const
{
	return parent(childIndex);
}

QString FileTreeModel::customColumnText(Columns column, const FileTreeModelItem* item) const
{
	Q_UNUSED(column);
	Q_UNUSED(item);
	return QString();
}

QString FileTreeModel::customColumnName(Columns column) const
{
	Q_UNUSED(column);
	return QString();
}

QVariant FileTreeModel::columnIcon(const QModelIndex& index, FileTreeModelItem* file) const
{
	if(file->isFolder() == true && index.column() == static_cast<int>(Columns::FileNameColumn))
	{
		return QIcon(":/Images/Images/SchemaFolder.svg");
	}

	return QVariant();
}

QModelIndex FileTreeModel::index(int row, int column, const QModelIndex& parentIndex) const
{
	if (hasIndex(row, column, parentIndex) == false)
	{
		return QModelIndex();
	}

	// Is it request for the root's items?
	//
	if (parentIndex.isValid() == false)
	{
		return createIndex(row, column, const_cast<FileTreeModelItem*>(m_root->child(row)));
	}

	FileTreeModelItem* parent = static_cast<FileTreeModelItem*>(parentIndex.internalPointer());
	if (parent == nullptr)
	{
		assert(parent);
		return QModelIndex();
	}

	QModelIndex resultIndex = createIndex(row, column, parent->child(row));
	return resultIndex;
}

QModelIndex FileTreeModel::parent(const QModelIndex& childIndex) const
{
	if (childIndex.isValid() == false)
	{
		return QModelIndex();
	}

	FileTreeModelItem* child = static_cast<FileTreeModelItem*>(childIndex.internalPointer());
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

int FileTreeModel::rowCount(const QModelIndex& parentIndex) const
{
	const FileTreeModelItem* parent = fileItem(parentIndex);

	if (parent == nullptr)
	{
		assert(false);
		return 0;
	}

	return parent->childrenCount();
}

int FileTreeModel::columnCount(const QModelIndex& parentIndex) const
{
	Q_UNUSED(parentIndex);

	return static_cast<int>(m_columns.size());
}

QVariant FileTreeModel::data(const QModelIndex& index, int role) const
{
	if (index.isValid() == false)
	{
		return QVariant();
	}

	FileTreeModelItem* file = static_cast<FileTreeModelItem*>(index.internalPointer());
	assert(file != nullptr);

	switch (role)
	{
	case Qt::DisplayRole:
		{
			QVariant v;

			if (index.column() >= m_columns.size())
			{
				Q_ASSERT(false);
				return QVariant();
			}

			Columns column = m_columns[index.column()];

			switch (column)
			{
			case Columns::FileNameColumn:
				v.setValue<QString>(file->fileName());
				break;

			case Columns::FileSizeColumn:
				v.setValue<QString>(QString("%1").arg(file->size()));
				break;

			case Columns::FileStateColumn:
				{
					if (file->state() == VcsState::CheckedOut)
					{
						QString state = file->action().text();
						v.setValue<QString>(state);
					}
				}
				break;

			case Columns::FileUserColumn:
				if (file->state() == VcsState::CheckedOut)
				{
					v = db()->username(file->userId());
				}
				break;

			case Columns::FileIdColumn:
				v.setValue(file->fileId());
				break;

			case Columns::FileAttributesColumn:
				v.setValue(QString::number(file->attributes(), 2));
				break;

			default:
				v.setValue(customColumnText(column, file));
			}

			return v;
		}
		break;

	case Qt::UserRole:
		{
			return file->fileId();
		}
		break;

	case Qt::TextAlignmentRole:
		{
			return Qt::AlignLeft + Qt::AlignVCenter;
		}
		break;

	case Qt::BackgroundRole:
		{
			if (file->state() == VcsState::CheckedOut)
			{

				QBrush b(StandardColors::VcsCheckedIn);

				switch (static_cast<VcsItemAction::VcsItemActionType>(file->action().toInt()))
				{
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
	case Qt::DecorationRole:
			{
				return columnIcon(index, file);
			}
		break;
	}

	return QVariant();
}

QVariant FileTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal) {

			if (section >= m_columns.size())
			{
				Q_ASSERT(false);
				return QVariant();
			}

			Columns column = m_columns[section];

			switch (column)
			{
			case Columns::FileNameColumn:
				return QObject::tr("Name");

			case Columns::FileSizeColumn:
				return QObject::tr("Size");

			case Columns::FileStateColumn:
				return QObject::tr("State");

			case Columns::FileUserColumn:
				return QObject::tr("User");

			case Columns::FileIdColumn:
				return QObject::tr("FileID");

			case Columns::FileAttributesColumn:
				return QObject::tr("Attributes");

			default:
				return customColumnName(column);
			}
		}
	}

	return QVariant();
}

bool FileTreeModel::hasChildren(const QModelIndex& parentIndex) const
{
	if (db()->isProjectOpened() == false)
	{
		return false;
	}

	if (parentIndex.column() > 0)
	{
		return 0;
	}

	const FileTreeModelItem* file = fileItem(parentIndex);
	if (file == nullptr)
	{
		Q_ASSERT(file);
		return false;
	}

	if (file->childrenCount() > 0)
	{
		// seems that we already got file list for this object
		return true;
	}

	bool hasChildren = false;
	DbFileInfo fi(*file);

	bool result = db()->fileHasChildren(&hasChildren, fi, m_parentWidget);
	if (result == false)
	{
		return false;
	}

	if (m_addFileInProgress == true &&
		hasChildren == true &&
		file->childrenCount() == 0)
	{
		// seems that file has been added to filesystem but not yet added to child array
		return false;
	}

	return hasChildren;
}

bool FileTreeModel::canFetchMore(const QModelIndex& parentIndex) const
{
	if (db()->isProjectOpened() == false)
	{
		return false;
	}

	const FileTreeModelItem* file = fileItem(parentIndex);
	if (file == nullptr)
	{
		Q_ASSERT(file);
		return false;
	}

	if (file->childrenCount() > 0)
	{
		return false;	// seems that we already got file list for this object
	}

	return file->fetched() == false;
}

void FileTreeModel::fetchMore(const QModelIndex& parentIndex)
{
	if (db()->isProjectOpened() == false)
	{
		return;
	}

	FileTreeModelItem* parentFile = fileItem(const_cast<QModelIndex&>(parentIndex));
	if (parentFile == nullptr)
	{
		Q_ASSERT(parentFile);
		return;
	}

	if (parentFile->fetched() == true)
	{
		return;
	}

	parentFile->setFetched();

	std::vector<DbFileInfo> files;

	bool ok = db()->getFileList(&files, parentFile->fileId(), true, m_parentWidget);
	if (ok == false)
	{
		Q_ASSERT(false);
		return;
	}

	int oldChildCount = parentFile->childrenCount();
	int newChildCount = static_cast<int>(files.size());

	if (newChildCount > oldChildCount)
	{
		beginInsertRows(parentIndex, oldChildCount, newChildCount - 1);
	}
	else
	{
		if (newChildCount < oldChildCount)
		{
			beginRemoveRows(parentIndex, newChildCount, oldChildCount - 1);
		}
	}

	for (const DbFileInfo& fi : files)
	{
		std::shared_ptr childFile = std::make_shared<FileTreeModelItem>(fi);

		if (parentFile->childByFileId(childFile->fileId()) == nullptr)
		{
			parentFile->addChild(childFile);
		}

		// fetch child files also

		std::vector<DbFileInfo> childFiles;

		ok = db()->getFileList(&childFiles, fi.fileId(), true, m_parentWidget);
		if (ok == false)
		{
			Q_ASSERT(false);
			return;
		}

		for (const DbFileInfo& childFi : childFiles)
		{
			childFile->addChild(std::make_shared<FileTreeModelItem>(childFi));
		}

		childFile->setFetched();
	}

	if (newChildCount > oldChildCount)
	{
		endInsertRows();
	}
	else
	{
		if (newChildCount < oldChildCount)
		{
			endRemoveRows();
		}
	}

	return;
}

void FileTreeModel::fetch(const QModelIndex& parent)
{
	fetchMore(parent);

	return;
}

void FileTreeModel::fetchRecursively(QModelIndex parentIndex, const std::vector<int>& fileIdSet)
{
	FileTreeModelItem* f = fileItem(parentIndex);
	if (f == nullptr)
	{
		Q_ASSERT(f);
		return;
	}

	if (f->fetched() == false)
	{
		fetch(parentIndex);
	}

	int rows = rowCount(parentIndex);

	for (int i = 0; i < rows; i++)
	{
		QModelIndex childFileIndex = childIndex(i, 0, parentIndex);

		FileTreeModelItem* childFile = fileItem(childFileIndex);
		if (childFile == nullptr)
		{
			Q_ASSERT(childFile);
			return;
		}

		if (std::find(fileIdSet.begin(), fileIdSet.end(), childFile->fileId()) != fileIdSet.end())
		{
			fetchRecursively(childFileIndex, fileIdSet);
		}
	}

	return;
}

FileTreeModelItem* FileTreeModel::fileItem(QModelIndex& index)
{
	FileTreeModelItem* f = nullptr;

	if (index.isValid() == false)
	{
		f = m_root.get();
	}
	else
	{
		f = static_cast<FileTreeModelItem*>(index.internalPointer());
	}

	assert(f != nullptr);
	return f;
}

const FileTreeModelItem* FileTreeModel::fileItem(const QModelIndex& index) const
{
	const FileTreeModelItem* f = nullptr;

	if (index.isValid() == false)
	{
		f = m_root.get();
	}
	else
	{
		f = static_cast<const FileTreeModelItem*>(index.internalPointer());
	}

	assert(f != nullptr);
	return f;
}

std::shared_ptr<FileTreeModelItem> FileTreeModel::fileItemSharedPtr(QModelIndex& index)
{
	FileTreeModelItem* rawPtr = nullptr;

	if (index.isValid() == false)
	{
		rawPtr = m_root.get();
	}
	else
	{
		rawPtr = static_cast<FileTreeModelItem*>(index.internalPointer());
	}

	if (rawPtr == nullptr)
	{
		assert(rawPtr);
		return std::shared_ptr<FileTreeModelItem>();
	}

	if (rawPtr->parent() == nullptr)
	{
		assert(rawPtr->parent() != nullptr);
		return std::shared_ptr<FileTreeModelItem>();
	}

	int childIndex = rawPtr->parent()->childIndex(rawPtr);
	if (childIndex == -1)
	{
		assert(childIndex != -1);
		return std::shared_ptr<FileTreeModelItem>();
	}

	std::shared_ptr<FileTreeModelItem> object = rawPtr->parent()->childSharedPtr(childIndex);

	assert(object != nullptr);
	return object;
}

void FileTreeModel::addFile(QModelIndex& parentIndex, std::shared_ptr<FileTreeModelItem>& file)
{
	assert(parentIndex.isValid());

	FileTreeModelItem* parentFile = fileItem(parentIndex);
	if (parentFile == nullptr)
	{
		assert(parentFile);
		return;
	}

	if (parentFile->childByFileId(file->fileId()) != nullptr)
	{
		// File with this fileId already exists
		//
		return;
	}

	m_addFileInProgress = true;

	beginInsertRows(parentIndex, parentFile->childrenCount(), parentFile->childrenCount());
	parentFile->addChild(file);
	endInsertRows();

	m_addFileInProgress = false;

	return;
}

void FileTreeModel::removeFile(QModelIndex index)
{
	assert(index.isValid());

	QModelIndex parentIndex = index.parent();
	if (parentIndex.isValid() == false)
	{
		return;
	}

	FileTreeModelItem* parentFile = fileItem(parentIndex);
	if (parentFile == nullptr)
	{
		assert(parentFile);
		return;
	}

	FileTreeModelItem* file = fileItem(index);
	if (file == nullptr)
	{
		assert(file);
		return;
	}

	int childIndex = parentFile->childIndex(file);
	if (childIndex == -1)
	{
		assert(childIndex != -1);
		return;
	}

	beginRemoveRows(index.parent(), childIndex, childIndex);
	parentFile->deleteChild(file);
	endRemoveRows();
}

void FileTreeModel::updateFile(QModelIndex index, const DbFileInfo& file)
{
	if (m_columns.empty() == true)
	{
		Q_ASSERT(false);
		return;
	}

	if (index.isValid() == false || file.fileId() == -1)
	{
		assert(index.isValid());
		assert(file.fileId() != -1);
		return;
	}

	if (file.deleted() == true || (file.state() == VcsState::CheckedIn && file.action() == VcsItemAction::Deleted))
	{
		removeFile(index);
		return;
	}

	FileTreeModelItem* treeFile = fileItem(index);

	if (treeFile == nullptr || treeFile->fileId() != file.fileId())
	{
		assert(treeFile);
		assert(treeFile->fileId() == file.fileId());
		return;
	}

	QModelIndex parentIndex = index.parent();
	if (parentIndex.isValid() == false)
	{
		return;
	}

	FileTreeModelItem* parentFile = fileItem(parentIndex);
	if (parentFile == nullptr)
	{
		assert(parentFile);
		return;
	}

	FileTreeModelItem* childFile = parentFile->childByFileId(file.fileId());

	if (childFile == nullptr)
	{
		assert(childFile);
		return;
	}

	*(static_cast<DbFileInfo*>(childFile)) = file;

	QModelIndex leftIndex = this->index(parentFile->childIndex(childFile), 0, parentIndex);
	QModelIndex rightIndex = this->index(parentFile->childIndex(childFile), static_cast<int>(m_columns.size()) - 1, parentIndex);

	emit dataChanged(leftIndex, rightIndex);

	return;
}

bool FileTreeModel::moveFiles(const QModelIndexList& selectedIndexes,
                                        int movedToParnetId,
                                        const std::vector<DbFileInfo>& movedFiles,
                                        std::vector<QModelIndex>* addedFilesIndexes)
{
	if (addedFilesIndexes == nullptr)
	{
		Q_ASSERT(addedFilesIndexes);
		refresh();
		return false;
	}

	if (movedFiles.empty() == true)
	{
		Q_ASSERT(movedFiles.empty() == false);
		refresh();
		return false;
	}

	if (movedToParnetId == DbFileInfo::Null)
	{
		Q_ASSERT(movedToParnetId != DbFileInfo::Null);
		refresh();
		return false;
	}

	// Remove moved files
	//
	for (QModelIndex index : selectedIndexes)
	{
		FileTreeModelItem* file = fileItem(index);
		if (file == nullptr)
		{
			Q_ASSERT(file);
			return false;
		}

		FileTreeModelItem* parentFile = file->parent();
		if (parentFile == nullptr)
		{
			Q_ASSERT(parentFile);
			return false;
		}

		int childIndex = parentFile->childIndex(file);

		QModelIndex pi = index.parent();

		beginRemoveRows(pi, childIndex, childIndex);
		parentFile->deleteChild(file);
		endRemoveRows();
	}


	// Get parent index where files were moved
	//
	QModelIndexList matched = match(index(0, 0, QModelIndex()),
	                                Qt::UserRole,
	                                QVariant::fromValue(movedToParnetId),
	                                1,
	                                Qt::MatchExactly | Qt::MatchRecursive);

	if (matched.size() != 1)
	{
		// Cant find ModelIndex for parent
		//
		Q_ASSERT(matched.size() == 1);

		// Mitigate error
		//
		refresh();
		return false;
	}

	QModelIndex movedToParentIndex = matched.front();
	Q_ASSERT(movedToParentIndex.isValid());

	FileTreeModelItem* movedToFile = fileItem(movedToParentIndex);
	if (movedToFile == nullptr)
	{
		Q_ASSERT(movedToFile);
		return false;
	}

	if (movedToParnetId != movedToFile->fileId())
	{
		Q_ASSERT(movedToParnetId == movedToFile->fileId());

		refresh();
		return false;
	}

	// Add moved files to destination index
	//

	if (movedToFile->fetched() == true)
	{
		for (const DbFileInfo& f : movedFiles)
		{
			std::shared_ptr<FileTreeModelItem> fi = std::make_shared<FileTreeModelItem>(f);
			addFile(movedToParentIndex, fi);

			QModelIndex addedIndex = childIndex(rowCount(movedToParentIndex) - 1, 0, movedToParentIndex);
			addedFilesIndexes->push_back(addedIndex);
		}
	}

	return true;
}

void FileTreeModel::refresh()
{
	beginResetModel();
	*m_root.get() =  FileTreeModelItem(db()->systemFileInfo(m_rootFileId));
	endResetModel();
}

void FileTreeModel::projectOpened()
{
	DbFileInfo rootFileInfo = db()->systemFileInfo(m_rootFilePath);
	if (rootFileInfo.isNull() == true)
	{
		Q_ASSERT(false);
		m_rootFileId = db()->rootFileId();
	}
	else
	{
		m_rootFileId = rootFileInfo.fileId();
	}

	beginResetModel();
	*m_root.get() = FileTreeModelItem(db()->systemFileInfo(m_rootFileId));
	endResetModel();

	return;
}

void FileTreeModel::projectClosed()
{
	// Release all children
	//
	beginResetModel();
	m_root->deleteAllChildren();
	endResetModel();

	return;
}

DbController* FileTreeModel::db()
{
	return m_dbc;
}

DbController* FileTreeModel::db() const
{
	return m_dbc;
}


//
//
//	FileTreeView
//
//

FileTreeView::FileTreeView(DbController* dbc, FileTreeModel* model) :
	m_dbc(dbc),
	m_model(model)
{
	if (m_model == nullptr || m_dbc == nullptr)
	{
		Q_ASSERT(m_dbc);
		Q_ASSERT(m_model);
		return;
	}

	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::ExtendedSelection);

	m_proxyModel = new FileTreeProxyModel(model, this);
	m_proxyModel->setSourceModel(model);
	m_proxyModel->setFilterKeyColumn(static_cast<int>(FileTreeModel::Columns::FileNameColumn));

	setModel(m_proxyModel);
}

FileTreeView::~FileTreeView()
{
}

// Returns selected rows mapped to source model
//
QModelIndexList FileTreeView::selectedSourceRows() const
{

	FileTreeProxyModel* proxyModel = dynamic_cast<FileTreeProxyModel*>(model());

	if (proxyModel != nullptr)
	{
		const QModelIndexList proxySelection = selectionModel()->selectedRows();

		QModelIndexList selectedIndexList;

		for (QModelIndex mi : proxySelection)
		{
			selectedIndexList.push_back(proxyModel->mapToSource(mi));
		}

		return selectedIndexList;
	}
	else
	{
		return selectionModel()->selectedRows();
	}
}


bool FileTreeView::newFile(const QString& fileName, const QByteArray& data)
{
	// Find parent file
	//
	QModelIndexList selectedRows = selectedSourceRows();	// Indexes from source model

	if (selectedRows.size() != 1 )
	{
		assert(selectedRows.size() == 1);
		return false;
	}

	// Create files vector
	//
	std::vector<std::shared_ptr<DbFile>> files;

	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
	file->setFileName(fileName);
	file->setData(data);

	files.push_back(file);

	bool result = createFiles(files, true/*createInParentFolder*/);

	return result;

}

void FileTreeView::moveFile(int parentFileId)
{
	QModelIndexList	selectedIndexes = selectedSourceRows();

	std::vector<DbFileInfo> filesToMove;
	filesToMove.reserve(selectedIndexes.size());

	for (QModelIndex& mi : selectedIndexes)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root items deleting
			//
			continue;
		}
		FileTreeModelItem* f = m_model->fileItem(mi);

		if (db()->isSystemFile(f->fileId()) == true ||
			f->state() != VcsState::CheckedOut)
		{
			continue;
		}

		if (db()->currentUser().isAdminstrator() == true || db()->currentUser().userId() == f->userId())
		{
			filesToMove.push_back(*f);
		}
	}

	if (filesToMove.empty() == true)
	{
		Q_ASSERT(filesToMove.empty() == false);
		return;
	}

	// Get destination folder
	//

	std::pair<int, std::vector<int>> result = DbControllerTools::showSelectFolderDialog(db(), parentFileId, filesToMove.front().parentId(), false, this);

	int moveToFileId = result.first;
	if (moveToFileId == -1)
	{
		return;
	}

	const std::vector<int>& moveToFileIdParents = result.second;

	// Move files in DB
	//
	std::vector<DbFileInfo> movedFiles;

	if (bool ok = db()->moveFiles(filesToMove, moveToFileId, &movedFiles, this);
		ok == false)
	{
		return;
	}

	// Fetch all items that are parents of moveToFileId. This will enable search of added item

	m_model->fetchRecursively(QModelIndex(), moveToFileIdParents);

	// Update model/view
	//
	std::vector<QModelIndex> addedIndexes;
	addedIndexes.reserve(selectedIndexes.size());

	if (bool ok = m_model->moveFiles(selectedIndexes, moveToFileId, movedFiles, &addedIndexes);
		ok == false)
	{
		return;
	}

	// Expand parent
	//
	QModelIndexList matched = m_model->match(m_model->childIndex(0, 0, QModelIndex()),
															  Qt::UserRole,
															  QVariant::fromValue(moveToFileId),
															  1,
															  Qt::MatchExactly | Qt::MatchRecursive);
	Q_ASSERT(matched.size() == 1);

	if (matched.size() == 1)
	{
		QModelIndex fileModelIndex = matched.front();
		QModelIndex mappedModelIndex = m_proxyModel->mapFromSource(fileModelIndex);

		QModelIndex expandParent = mappedModelIndex;
		while (expandParent.isValid() == true)
		{
			expand(expandParent);
			expandParent = expandParent.parent();
		}
	}


	// Select moved files
	//
	selectionModel()->reset();

	for (const QModelIndex& mi : addedIndexes)
	{
		QModelIndex mappedToProxy = m_proxyModel->mapFromSource(mi);

		selectionModel()->select(mappedToProxy, QItemSelectionModel::Select | QItemSelectionModel::Rows);
	}

	return;
}

void FileTreeView::checkOutFilesById(std::vector<int> fileIds)
{
	QModelIndexList indexList;

	for (int fileId : fileIds)
	{
		QModelIndexList matched = m_model->match(m_model->childIndex(0, 0, QModelIndex()),
															  Qt::UserRole,
															  QVariant::fromValue(fileId),
															  1,
															  Qt::MatchExactly | Qt::MatchRecursive);
		Q_ASSERT(matched.size() == 1);

		if (matched.size() == 1)
		{
			indexList.push_back(matched.front());
		}
	}

	if (indexList.empty() == true)
	{
		return;
	}

	checkOutFiles(indexList);

	return;
}

void FileTreeView::checkInFilesById(std::vector<int> fileIds, std::vector<int>* deletedFileIds)
{
	if (deletedFileIds == nullptr)
	{
		Q_ASSERT(deletedFileIds);
		return;
	}

	QModelIndexList indexList;

	for (int fileId : fileIds)
	{
		QModelIndexList matched = m_model->match(m_model->childIndex(0, 0, QModelIndex()),
												 Qt::UserRole,
												 QVariant::fromValue(fileId),
												 1,
												 Qt::MatchExactly | Qt::MatchRecursive);
		Q_ASSERT(matched.size() == 1);

		if (matched.size() == 1)
		{
			indexList.push_back(matched.front());
		}
	}

	if (indexList.empty() == true)
	{
		return;
	}

	checkInFiles(indexList);

	for (int fileId : fileIds)
	{
		QModelIndexList matchedAfter = m_model->match(m_model->childIndex(0, 0, QModelIndex()),
													  Qt::UserRole,
													  QVariant::fromValue(fileId),
													  1,
													  Qt::MatchExactly | Qt::MatchRecursive);

		if (matchedAfter.empty() == true)
		{
			deletedFileIds->push_back(fileId);

		}
	}

	return;
}

// Function returns false if user has clicked No on confirmation dialog
//
bool FileTreeView::undoChangesFilesById(std::vector<int> fileIds, std::vector<int>* deletedFileIds)
{
	if (deletedFileIds == nullptr)
	{
		Q_ASSERT(deletedFileIds);
		return false;
	}

	QModelIndexList indexList;

	for (int fileId : fileIds)
	{
		QModelIndexList matched = m_model->match(m_model->childIndex(0, 0, QModelIndex()),
												 Qt::UserRole,
												 QVariant::fromValue(fileId),
												 1,
												 Qt::MatchExactly | Qt::MatchRecursive);
		Q_ASSERT(matched.size() == 1);

		if (matched.size() == 1)
		{
			indexList.push_back(matched.front());
		}
	}

	if (indexList.empty() == true)
	{
		return true;
	}

	if (undoChangesFiles(indexList) == false)
	{
		return false;
	}


	for (int fileId : fileIds)
	{
		QModelIndexList matchedAfter = m_model->match(m_model->childIndex(0, 0, QModelIndex()),
													  Qt::UserRole,
													  QVariant::fromValue(fileId),
													  1,
													  Qt::MatchExactly | Qt::MatchRecursive);

		if (matchedAfter.empty() == true)
		{
			deletedFileIds->push_back(fileId);

		}
	}

	return true;
}

void FileTreeView::setFileNameFilter(const QString& filterText)
{
	m_proxyModel->setFilterFixedString(filterText);
}

void FileTreeView::addFile()
{
	createFiles({}, false/*createInParentFolder*/);
	return;
}

void FileTreeView::addFileToFolder()
{
	createFiles({}, true/*createInParentFolder*/);
	return;
}

void FileTreeView::addFolder(const QString& folderName)
{
	// Find parent file
	//
	QModelIndexList selectedRows = selectedSourceRows();	// Indexes from source model

	if (selectedRows.size() != 1 )
	{
		assert(selectedRows.size() == 1);
		return;
	}

	// Create files vector
	//
	std::vector<std::shared_ptr<DbFile>> files;

	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
	file->setFileName(folderName);
	file->setDetails("{}");
	file->setDirectoryAttribute(true);
	file->clearData();

	files.push_back(file);

	createFiles(files, true/*createInParentFolder*/);

	return;
}

void FileTreeView::viewFile()
{
	runFileEditor(true);
}

void FileTreeView::editFile()
{
	runFileEditor(false);
}

void FileTreeView::renameFile()
{
	QModelIndexList selectedIndexList = selectedSourceRows();

	if (selectedIndexList.isEmpty() == true)
	{
		return;
	}

	std::vector<DbFileInfo> files;
	files.reserve(static_cast<size_t>(selectedIndexList.size()));

	std::vector<QModelIndex> indexes;

	for (QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			continue;
		}

		FileTreeModelItem* f = m_model->fileItem(mi);
		assert(f);


		if (db()->currentUser().isAdminstrator() == true || db()->currentUser().userId() == f->userId())
		{
			files.push_back(*f);
			indexes.push_back(mi);
		}
	}

	if (files.size() != 1)
	{
		return;
	}

	QString newFileName = QInputDialog::getText(this, qAppName(), tr("Enter file name:"), QLineEdit::Normal, files[0].fileName());
	if (newFileName.isEmpty() == true || newFileName == files[0].fileName())
	{
		return;
	}

	if (newFileName.endsWith(".js") == false)
	{
		newFileName += ".js";
	}

	DbFileInfo newFi;

	if (db()->renameFile(files[0], newFileName, &newFi, this) == false)
	{
		return;
	}

	m_model->updateFile(indexes[0], newFi);

	return;

}

void FileTreeView::deleteFile()
{
	QModelIndexList selectedIndexList = selectedSourceRows();
	if (selectedIndexList.isEmpty() == true)
	{
		return;
	}

	std::vector<DbFileInfo> files;
	files.reserve(static_cast<size_t>(selectedIndexList.size()));

	for (QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid root items deleting
			//
			continue;
		}

		FileTreeModelItem* f = m_model->fileItem(mi);
		assert(f);

		if (db()->currentUser().isAdminstrator() == true || db()->currentUser().userId() == f->userId())
		{
			files.push_back(*f);
		}
	}

	if (files.empty() == true)
	{
		// Nothing to delete
		//
		return;
	}

	// Delete from database
	//
	db()->deleteFiles(&files, this);

	// Delete from model or update
	//
	for (const DbFileInfo& fi : files)
	{
		auto mipos = std::find_if(selectedIndexList.begin(), selectedIndexList.end(),
			[&fi, this](QModelIndex& mi)
			{
				FileTreeModelItem* f = m_model->fileItem(mi);
				assert(f);
				return f->fileId() == fi.fileId();
			});

		assert(mipos != selectedIndexList.end());

		if (mipos != selectedIndexList.end())
		{
			m_model->updateFile(*mipos, fi);
		}
	}

	setFocus();

	// Select current index after deleting

	if (currentIndex().isValid() == true)
	{
		selectionModel()->select(currentIndex(), QItemSelectionModel::Select | QItemSelectionModel::Rows);
	}

	return;
}

void FileTreeView::checkOutSelectedFiles()
{
	QModelIndexList selectedIndexList = selectedSourceRows();

	checkOutFiles(selectedIndexList);

	return;
}

void FileTreeView::checkInSelectedFiles()
{
	QModelIndexList selectedIndexList = selectedSourceRows();

	checkInFiles(selectedIndexList);

	return;
}

// Function returns false if user has clicked No on confirmation dialog
//
bool FileTreeView::undoChangesSelectedFiles()
{
	QModelIndexList selectedIndexList = selectedSourceRows();

	return undoChangesFiles(selectedIndexList);
}

void FileTreeView::showHistory()
{
	QModelIndexList selected = selectedSourceRows();

	if (selected.size() != 1)
	{
		return;
	}

	// --
	//
	FileTreeModelItem* file = m_model->fileItem(selected.first());

	if (file == nullptr)
	{
		assert(file);
		return;
	}

	// Get file history
	//
	std::vector<DbChangeset> fileHistory;

	bool ok = db()->getFileHistoryRecursive(*file, &fileHistory, this);
	if (ok == false)
	{
		return;
	}

	// Show history dialog
	//
	FileHistoryDialog::showHistory(db(), file->fileName(), fileHistory, this);

	return;
}

void FileTreeView::showCompare()
{
	QModelIndexList selected = selectedSourceRows();

	if (selected.size() != 1)
	{
		return;
	}

	// --
	//
	FileTreeModelItem* file = m_model->fileItem(selected.first());

	if (file == nullptr)
	{
		assert(file);
		return;
	}

	CompareDialog::showCompare(db(), DbChangesetObject(*file), -1, this);

	return;
}

void FileTreeView::getLatestVersion()
{
	QModelIndexList selectedIndexList = selectedSourceRows();

	if (selectedIndexList.isEmpty() == true)
	{
		return;
	}

	std::vector<DbFileInfo> files;
	files.reserve(static_cast<size_t>(selectedIndexList.size()));

	for (QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid any actions to root items
			//
			continue;
		}

		FileTreeModelItem* f = m_model->fileItem(mi);
		assert(f);

		files.push_back(*f);
	}

	if (files.empty() == true)
	{
		// Nothing to do
		//
		return;
	}

	// Select destination folder
	//
	QString dir = QDir().toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Select Directory"), QString(),
													QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));

	if (dir.isEmpty() == true)
	{
		return;
	}

	// Get files from the database
	//
	std::vector<std::shared_ptr<DbFile>> out;

	bool ok = db()->getLatestVersion(files, &out, this);
	if (ok == false)
	{
		return;
	}

	// Save files to disk
	//
	for (unsigned int i = 0; i < out.size(); i++)
	{
		bool writeResult = out[i]->writeToDisk(dir);

		if (writeResult == false)
		{
			QMessageBox msgBox;
			msgBox.setText(tr("Write file error."));
			msgBox.setInformativeText(tr("Cannot write file %1.").arg(out[i]->fileName()));
			msgBox.exec();
		}
	}

	return;
}

void FileTreeView::getLatestTreeVersion()
{
	QModelIndexList selectedIndexList = selectedSourceRows();

	if (selectedIndexList.isEmpty() == true)
	{
		return;
	}

	std::vector<DbFileInfo> files;
	files.reserve(static_cast<size_t>(selectedIndexList.size()));

	for (QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid any actions to root items
			//
			continue;
		}

		FileTreeModelItem* f = m_model->fileItem(mi);
		assert(f);

		files.push_back(*f);
	}

	if (files.empty() == true)
	{
		// Nothing to do
		//
		return;
	}

	// Select destination folder
	//
	QString dir = QDir().toNativeSeparators(QFileDialog::getExistingDirectory(this, tr("Select Directory"), QString(),
													QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));

	if (dir.isEmpty() == true)
	{
		return;
	}

	for (auto f : files)
	{
		// Get files from the database
		//

		if (getLatestFileVersionRecursive(f, dir) == false)
		{
			break;
		}
	}

	return;
}

bool FileTreeView::createFiles(std::vector<std::shared_ptr<DbFile>> files, bool createInParentFolder)
{
	QModelIndexList selectedRows = selectedSourceRows();	// Indexes from source model

	if (selectedRows.size() != 1)
	{
		assert(selectedRows.size() == 1);
		return false;
	}

	// Select and read files if files array is empty
	//
	if (files.empty() == true)
	{

		QFileDialog fd(this);
		fd.setFileMode(QFileDialog::ExistingFiles);

		if (fd.exec() == QDialog::Rejected)
		{
			return false;
		}

		QStringList selectedFiles = fd.selectedFiles();

		for (int i = 0; i < selectedFiles.size(); i++)
		{
			std::shared_ptr<DbFile> file = std::make_shared<DbFile>();

			bool ok = file->readFromDisk(selectedFiles[i]);
			if (ok == false)
			{
				QMessageBox msgBox;

				msgBox.setText(tr("File %1 cannot be read.").arg(selectedFiles[i]));
				msgBox.setInformativeText(tr("The operation is terminated."));

				msgBox.exec();
				return false;
			}

			files.push_back(file);
		}
	}

	//

	QModelIndex selectedIndex = selectedRows[0];

	FileTreeModelItem* parentFile = m_model->fileItem(selectedIndex);
	if (parentFile == nullptr || parentFile->fileId() == -1)
	{
		assert(parentFile);
		assert(parentFile->fileId() != -1);
		return false;
	}

	if (createInParentFolder == true)
	{
		// If file is selected, change selectedIndex to parent folder
		//
		while (parentFile->isFolder() == false)
		{
			selectedIndex = m_model->childParent(selectedIndex);

			if (selectedIndex.isValid() == false)
			{
				// Root is reached
				break;
			}

			parentFile = m_model->fileItem(selectedIndex);
			if (parentFile == nullptr || parentFile->fileId() == -1)
			{
				assert(parentFile);
				assert(parentFile->fileId() != -1);
				return false;
			}
		}

		if (parentFile == nullptr || parentFile->isFolder() == false)
		{
			QMessageBox msgBox;

			msgBox.setText(tr("Can't find the parent folder for selected file."));
			msgBox.setInformativeText(tr("The operation is terminated."));
			msgBox.exec();
			return false;
		}
	}

	//

	for (std::shared_ptr<DbFile> file : files)
	{
		// Add files to the DB
		//

		bool ok = db()->addFile(file, parentFile->fileId(), this);
		if (ok == false)
		{
			return false;
		}

		if (parentFile->fetched() == true)
		{
			std::shared_ptr<FileTreeModelItem> fi = std::make_shared<FileTreeModelItem>(*file);
			m_model->addFile(selectedIndex, fi);
		}
	}

	// Expand parent
	//
	QModelIndex selectedProxyIndex = m_proxyModel->mapFromSource(selectedIndex);

	if (isExpanded(selectedProxyIndex) == false)
	{
		expand(selectedProxyIndex);
	}

	// Select added files
	//
	selectionModel()->clear();				// clear selction. New selection will be set after files added to db

	for (std::shared_ptr<DbFile> file : files)
	{
		for (int i = 0; i < 65535; i++)
		{
			QModelIndex childIndex = m_model->childIndex(i, 0, selectedIndex);

			if (childIndex.isValid() == false)
			{
				break;
			}

			FileTreeModelItem* childFile = m_model->fileItem(childIndex);

			if (childFile == nullptr)
			{
				assert(childFile);
				break;
			}

			if (file->fileId() == childFile->fileId())
			{
				QModelIndex selectIndex = m_proxyModel->mapFromSource(childIndex);

				if (selectIndex.isValid() == true)
				{
					selectionModel()->select(selectIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
				}

				break;
			}
		}
	}

	return true;

}

void FileTreeView::checkOutFiles(QModelIndexList indexList)
{
	if (indexList.isEmpty() == true)
	{
		return;
	}

	std::vector<DbFileInfo> files;
	files.reserve(static_cast<size_t>(indexList.size()));

	for (QModelIndex& mi : indexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid to check out root "folders"
			//
			continue;
		}

		FileTreeModelItem* f = m_model->fileItem(mi);
		assert(f);

		if (f->state() == VcsState::CheckedOut)
		{
			continue;
		}

		files.push_back(*f);
	}

	if (files.empty() == true)
	{
		// Nothing to checkout
		//
		return;
	}

	// CheckOut from database
	//
	db()->checkOut(files, this);

	// Update files state
	//
	for (const DbFileInfo& fi : files)
	{
		auto mipos = std::find_if(indexList.begin(), indexList.end(),
			[&fi, this](QModelIndex& mi)
			{
				FileTreeModelItem* f = m_model->fileItem(mi);
				assert(f);
				return f->fileId() == fi.fileId();
			});

		assert(mipos != indexList.end());

		if (mipos != indexList.end())
		{
			m_model->updateFile(*mipos, fi);
		}
	}

	return;
}

void FileTreeView::checkInFiles(QModelIndexList indexList)
{
	if (indexList.isEmpty() == true)
	{
		return;
	}

	std::vector<DbFileInfo> files;
	files.reserve(static_cast<size_t>(indexList.size()));

	for (QModelIndex& mi : indexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid to check in root "folders"
			//
			continue;
		}

		FileTreeModelItem* f = m_model->fileItem(mi);
		assert(f);

		if (f->state() == VcsState::CheckedOut &&
			(db()->currentUser().isAdminstrator() == true || db()->currentUser().userId() == f->userId()))
		{
			files.push_back(*f);
		}
	}

	if (files.empty() == true)
	{
		// Nothing to checkIn
		//
		return;
	}

	// As some rows can be deleted during update model,
	// files must be sorted in FileID descending order,
	// to delete first children and then their parents
	//
	std::sort(files.begin(), files.end(),
		[](DbFileInfo& m1, DbFileInfo m2)
		{
			return m1.fileId() >= m2.fileId();
		});

	// CheckIn changes to the database
	//
	std::vector<DbFileInfo> checkedInFiles;

	CheckInDialog::checkIn(files, false, &checkedInFiles, db(), this);

	// Update files state
	//
	for (const DbFileInfo& fi : checkedInFiles)
	{
		auto mipos = std::find_if(indexList.begin(), indexList.end(),
			[&fi, this](QModelIndex& mi)
			{
				FileTreeModelItem* f = m_model->fileItem(mi);
				assert(f);
				return f->fileId() == fi.fileId();
			});

		assert(mipos != indexList.end());

		if (mipos != indexList.end())
		{
			m_model->updateFile(*mipos, fi);
		}
	}

	return;
}

bool FileTreeView::undoChangesFiles(QModelIndexList indexList)
{
	if (indexList.isEmpty() == true)
	{
		return true;
	}

	std::vector<DbFileInfo> files;
	files.reserve(static_cast<size_t>(indexList.size()));

	for (QModelIndex& mi : indexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid any actions to root items
			//
			continue;
		}

		FileTreeModelItem* f = m_model->fileItem(mi);
		assert(f);

		if (f->state() == VcsState::CheckedOut &&
			(db()->currentUser().isAdminstrator() == true || db()->currentUser().userId() == f->userId()))
		{
			files.push_back(*f);
		}
	}

	if (files.empty() == true)
	{
		// Nothing to undo
		//
		return true;
	}

	auto mb = QMessageBox::question(
				  this,
				  tr("Undo Changes"),
				  tr("Do you want to undo pending changes? All selected objects' changes will be lost!"));

	if (mb == QMessageBox::No)
	{
		return false;
	}

	// Undo changes
	//
	db()->undoChanges(files, this);

	// Update files state
	//
	for (const DbFileInfo& fi : files)
	{
		auto mipos = std::find_if(indexList.begin(), indexList.end(),
								  [&fi, this](QModelIndex& mi)
		{
			FileTreeModelItem* f = m_model->fileItem(mi);
			assert(f);
			return f->fileId() == fi.fileId();
		});

		assert(mipos != indexList.end());

		if (mipos != indexList.end())
		{
			m_model->updateFile(*mipos, fi);
		}
	}

	setFocus();

	return true;
}

bool FileTreeView::getLatestFileVersionRecursive(const DbFileInfo& f, const QString& dir)
{
	std::shared_ptr<DbFile> out;

	bool ok = db()->getLatestVersion(f, &out, this);
	if (ok == false)
	{
		QMessageBox::critical(this, "Error", "Can't get the latest version of " + f.fileName());
		return false;
	}

	if (out->writeToDisk(dir) == false)
	{
		QMessageBox::critical(this, "Error", "Can't write file " + f.fileName() + " to disk");
		return false;
	}

	std::vector<DbFileInfo> childFiles;

	if (db()->getFileList(&childFiles, f.fileId(), true, this) == false)
	{
		QMessageBox::critical(this, "Error", "Can't get child files list of the file " + f.fileName());
		return false;
	}

	if (childFiles.empty() == true)
	{
		return true;
	}

	QString dirFiles = dir + QDir::separator() + f.fileName() + ".files";
	if (QDir().exists(dirFiles) == false)
	{
		if (QDir().mkdir(dirFiles) == false)
		{
			QMessageBox::critical(this, "Error", "Can't create the directory " + dirFiles);
			return false;
		}
	}

	for (DbFileInfo& child : childFiles)
	{
		if (getLatestFileVersionRecursive(child, dirFiles) == false)
		{
			return false;
		}
	}

	return true;
}

void FileTreeView::runFileEditor(bool viewOnly)
{
	QModelIndexList selectedIndexList = selectedSourceRows();

	if (selectedIndexList.isEmpty() == true)
	{
		return;
	}

	std::vector<DbFileInfo> files;
	files.reserve(static_cast<size_t>(selectedIndexList.size()));

	for (QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid any actions to root items
			//
			continue;
		}

		FileTreeModelItem* f = m_model->fileItem(mi);
		assert(f);

		if (viewOnly == true ||
			(f->state() == VcsState::CheckedOut &&
			(db()->currentUser().isAdminstrator() == true || db()->currentUser().userId() == f->userId())))
		{
			files.push_back(*f);
		}
	}

	if (files.size() != 1)
	{
		// Which file?
		//
		return;
	}

	DbFileInfo fileInfo = files[0];

	bool readOnly = true;

	if (viewOnly == false)
	{
		if (fileInfo.state() == VcsState::CheckedOut)
		{
			readOnly = false;
		}
	}


	std::shared_ptr<DbFile> f;
	if (db()->getLatestVersion(fileInfo, &f, this) == false)
	{
		QMessageBox::critical(this, "Error", "Get latest version error!");
		return;
	}

	QByteArray data;
	f->swapData(data);

	DialogFileEditor d(fileInfo.fileName(), &data, db(), readOnly, this);
	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

	f->swapData(data);

	if (db()->setWorkcopy(f, this) == false)
	{
		QMessageBox::critical(this, "Error", "Set work copy error!");
		return;
	}
}



void FileTreeView::setWorkcopy()
{
	QModelIndexList selectedIndexList = selectedSourceRows();

	if (selectedIndexList.isEmpty() == true)
	{
		return;
	}

	std::vector<DbFileInfo> files;
	files.reserve(static_cast<size_t>(selectedIndexList.size()));

	for (QModelIndex& mi : selectedIndexList)
	{
		if (mi.parent().isValid() == false)
		{
			// Forbid any actions to root items
			//
			continue;
		}

		FileTreeModelItem* f = m_model->fileItem(mi);
		assert(f);

		if (f->state() == VcsState::CheckedOut &&
			(db()->currentUser().isAdminstrator() == true || db()->currentUser().userId() == f->userId()))
		{
			files.push_back(*f);
		}
	}

	if (files.size() != 1)
	{
		// Which file?
		//
		return;
	}

	auto fileInfo = files[0];

	// Select file
	//
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
	static_cast<DbFileInfo*>(file.get())->operator=(fileInfo);

	bool readResult = file->readFromDisk(fileName);
	if (readResult == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Can't read file %1.").arg(fileName));
		mb.exec();
		return;
	}

	// Set file id for DbStore setWorkcopy
	//
	file->setFileId(fileInfo.fileId());
	file->setFileName(fileInfo.fileName());

	std::vector<std::shared_ptr<DbFile>> workcopyFiles;
	workcopyFiles.push_back(file);

	db()->setWorkcopy(workcopyFiles, this);

	// Update files state
	//
	for (const std::shared_ptr<DbFile>& fi : workcopyFiles)
	{
		auto mipos = std::find_if(selectedIndexList.begin(), selectedIndexList.end(),
			[&fi, this](QModelIndex& mi)
			{
				FileTreeModelItem* f = m_model->fileItem(mi);
				assert(f);
				return f->fileId() == fi->fileId();
			});

		assert(mipos != selectedIndexList.end());

		if (mipos != selectedIndexList.end())
		{
			m_model->updateFile(*mipos, *fi);
		}
	}


	return;
}

void FileTreeView::refreshFileTree()
{
	// Save old selection
	//
	std::vector<int> selectedFilesIds;

	{
		const QItemSelection proxySelection = selectionModel()->selection();
		const QItemSelection mappedSelection = m_proxyModel->mapSelectionToSource(proxySelection);

		selectedFilesIds.reserve(mappedSelection.size());

		for (QModelIndex mi : mappedSelection.indexes())
		{
			if (mi.column() > 0)
			{
				continue;
			}

			FileTreeModelItem* file = m_model->fileItem(mi);
			if (file == nullptr)
			{
				Q_ASSERT(file);
				return;
			}

			if (file != nullptr && file->isNull() == false)
			{

				selectedFilesIds.push_back(file->fileId());
			}
		}
	}

	// Save expansion
	//
	std::vector<int> expandedFileIds;

	{
		QModelIndexList indexes = m_proxyModel->getPersistentIndexList();

		expandedFileIds.reserve(indexes.size());

		for (QModelIndex& mi : indexes)
		{
			if (mi.column() > 0)
			{
				continue;
			}

			const FileTreeModelItem* file = m_model->fileItem(m_proxyModel->mapToSource(mi));
			if (file == nullptr)
			{
				Q_ASSERT(file);
				return;
			}

			int fileId = file->fileId();

			if (isExpanded(mi) == true)
			{
				if (fileId != DbFileInfo::Null)
				{
					expandedFileIds.push_back(fileId);
				}
			}
		}
	}

	// Save Header

	QByteArray headerState = header()->saveState();

	// Save Scroll Pos

	int xScroll = horizontalScrollBar()->value();
	int yScroll = verticalScrollBar()->value();

	// Update model
	//

	setVisible(false);

	selectionModel()->reset();

	m_proxyModel->setSourceModel(nullptr);

	m_model->refresh();

	m_proxyModel->setSourceModel(m_model);

	header()->restoreState(headerState);

	// Restore expansion and selection
	//
	selectionModel()->blockSignals(true);

	m_model->fetch(QModelIndex());

	int rootChildCount = m_model->childCount(QModelIndex());

	for (int i = 0; i < rootChildCount; i++)
	{
		expandAndSelect(m_model->childIndex(i, 0, QModelIndex()), expandedFileIds, selectedFilesIds);
	}

	selectionModel()->blockSignals(false);

	selectionChanged({}, {});					// To update actions

	setVisible(true);

	// Restore scroll

	QTimer::singleShot(1, [this, xScroll, yScroll](){
		if (xScroll <= horizontalScrollBar()->maximum())
		{
			horizontalScrollBar()->setValue(xScroll);
		}
		if (yScroll <= verticalScrollBar()->maximum())
		{
			verticalScrollBar()->setValue(yScroll);
		}
	});

	setFocus();

	return;
}

bool FileTreeView::expandAndSelect(const QModelIndex& mi, std::vector<int> expandedFileIds, std::vector<int> selectedFilesIds)
{
	const FileTreeModelItem* file = m_model->fileItem(mi);
	if (file == nullptr)
	{
		Q_ASSERT(file);
		return false;
	}

	bool parentIsExpanded = false;

	if (std::find(expandedFileIds.begin(), expandedFileIds.end(), file->fileId()) != expandedFileIds.end())
	{
		setExpanded(m_proxyModel->mapFromSource(mi), true);

		parentIsExpanded = true;
	}

	if (std::find(selectedFilesIds.begin(), selectedFilesIds.end(), file->fileId()) != selectedFilesIds.end())
	{
		selectionModel()->select(m_proxyModel->mapFromSource(mi), QItemSelectionModel::Select | QItemSelectionModel::Rows);
	}

	if (parentIsExpanded == true)
	{
		if (file->fetched() == false)
		{
			m_model->fetch(mi);	// Fetch children because parent is expanded and they can be selected
		}

		int childCount = m_model->childCount(mi);
		for (int i = 0; i < childCount; i++)
		{
			QModelIndex childIndex = m_model->childIndex(i, 0, mi);
			if (childIndex.isValid() == false)
			{
				Q_ASSERT(false);
				return false;
			}

			expandAndSelect(childIndex, expandedFileIds, selectedFilesIds);
		}
	}

	return true;
}

// Protected props
//

DbController* FileTreeView::db()
{
	return m_dbc;
}

