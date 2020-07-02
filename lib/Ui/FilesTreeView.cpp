#include "FilesTreeView.h"
#include "../lib/DbController.h"
#include "GlobalMessanger.h"
#include "DialogFileEditor.h"
#include "CheckInDialog.h"
#include "Forms/FileHistoryDialog.h"
#include "Forms/CompareDialog.h"

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

//
// FileTreeProxyModel
//

FileTreeProxyModel::FileTreeProxyModel(QObject *parent):
	QSortFilterProxyModel(parent)
{

}

bool FileTreeProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	QString filter = filterRegExp().pattern();
	if (filter.isEmpty() == true)
	{
		return true;
	}

	QModelIndex index = sourceModel()->index(sourceRow, filterKeyColumn(), sourceParent);

	FileTreeModel* treeModel = dynamic_cast<FileTreeModel*>(sourceModel());
	if (treeModel == nullptr)
	{
		Q_ASSERT(treeModel);
		return false;
	}

	FileTreeModelItem* item = treeModel->fileItem(index);
	if (item == nullptr)
	{
		Q_ASSERT(item);
		return false;
	}


	if (item->isFolder() == true || item->fileName().contains(filter, Qt::CaseInsensitive) == true)
	{
		return true;
	}

	return false;
}

bool FileTreeProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	FileTreeModel* treeModel = dynamic_cast<FileTreeModel*>(sourceModel());
	if (treeModel == nullptr)
	{
		Q_ASSERT(treeModel);
		return false;
	}

	const FileTreeModelItem* f1 = treeModel->fileItem(left);
	const FileTreeModelItem* f2 = treeModel->fileItem(right);

	if (f1 == nullptr || f2 == nullptr)
	{
		Q_ASSERT(f1);
		Q_ASSERT(f2);
		return false;
	}

	FileTreeModel::Columns column = treeModel->columnAtIndex(left.column());

	switch (static_cast<FileTreeModel::Columns>(column))
	{
	case FileTreeModel::Columns::FileNameColumn:
		{
			return f1->fileName() < f2->fileName();
		}
		break;
	case FileTreeModel::Columns::FileSizeColumn:
		{
			return f1->size() < f2->size();
		}
		break;
	case FileTreeModel::Columns::FileStateColumn:
		{
			return f1->state() < f2->state();
		}
		break;
	case FileTreeModel::Columns::FileUserColumn:
		{
			return f1->userId() < f2->userId();
		}
		break;
	case FileTreeModel::Columns::FileIdColumn:
		{
			return f1->fileId() < f2->fileId();
		}
		break;
	case FileTreeModel::Columns::FileAttributesColumn:
		{
			return f1->attributes() < f2->attributes();
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
	beginRemoveColumns(QModelIndex(), 0, static_cast<int>(m_columns.size() - 1));
	endRemoveColumns();

	m_columns = columns;

	beginInsertColumns(QModelIndex(), 0, static_cast<int>(m_columns.size() - 1));
	endInsertColumns();
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

QModelIndex FileTreeModel::childIndex(int row, int column, const QModelIndex& parentIndex) const
{
	return index(row, column, parentIndex);

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
	Q_UNUSED(index);
	Q_UNUSED(file);
	return QVariant();
}

QModelIndex FileTreeModel::index(int row, const QModelIndex& parentIndex) const
{
	return index(row, 0, parentIndex);
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
					v.setValue<qint32>(file->userId());
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

	case Qt::TextAlignmentRole:
		{
			return Qt::AlignLeft + Qt::AlignVCenter;
		}
		break;

	case Qt::BackgroundRole:
		{
			if (file->state() == VcsState::CheckedOut)
			{
				QBrush b(QColor(0xFF, 0xFF, 0xFF));

				switch (static_cast<VcsItemAction::VcsItemActionType>(file->action().toInt()))
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

	const FileTreeModelItem* file = fileItem(parentIndex);

	if (file->childrenCount() > 0)
	{
		return true;	// seems that we already got file list for this object
	}

	bool hasChildren = false;
	DbFileInfo fi(*file);

	bool result = db()->fileHasChildren(&hasChildren, fi, m_parentWidget);
	if (result == false)
	{
		return false;
	}

	return hasChildren;
}

bool FileTreeModel::canFetchMore(const QModelIndex& parent) const
{
	if (db()->isProjectOpened() == false)
	{
		return false;
	}

	const FileTreeModelItem* file = fileItem(parent);

	if (file->childrenCount() > 0)
	{
		return false;	// seems that we already got file list for this object
	}

	bool hasChildren = false;
	DbFileInfo fi(*file);

	bool result = db()->fileHasChildren(&hasChildren, fi, m_parentWidget);

	if (result == false)
	{
		return false;
	}

	return hasChildren;
}

void FileTreeModel::fetchMore(const QModelIndex& parentIndex)
{
	if (db()->isProjectOpened() == false)
	{
		return;
	}

	FileTreeModelItem* parentFile = fileItem(const_cast<QModelIndex&>(parentIndex));

	std::vector<DbFileInfo> files;

	bool ok = db()->getFileList(&files, parentFile->fileId(), true, m_parentWidget);
	if (ok == false)
		return;

	beginInsertRows(parentIndex, 0, static_cast<int>(files.size()) - 1);

	parentFile->deleteAllChildren();

	for (const DbFileInfo& fi : files)
	{
		parentFile->addChild(std::make_shared<FileTreeModelItem>(fi));
	}

	endInsertRows();

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

	beginInsertRows(parentIndex, parentFile->childrenCount(), parentFile->childrenCount());

	parentFile->addChild(file);

	endInsertRows();

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

	QModelIndex leftIndex = this->index(parentFile->childIndex(childFile), static_cast<int>(m_columns[0]), parentIndex);
	emit dataChanged(index, leftIndex);

	return;
}

void FileTreeModel::refresh()
{
	beginResetModel();
	*m_root.get() = FileTreeModelItem(db()->systemFileInfo(m_rootFileId));
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

FileTreeView::FileTreeView(DbController* dbc) :
	m_dbc(dbc)
{
	assert(m_dbc);

	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setUniformRowHeights(true);
	setIndentation(10);
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


void FileTreeView::newFile(const QString& fileName)
{
	// Find parent file
	//
	QModelIndexList selectedIndexList = selectedSourceRows();

	if (selectedIndexList.size() != 1)
	{
		assert(selectedIndexList.size() == 1);
		return;
	}

	QModelIndex parentIndex = selectedIndexList[0];
	FileTreeModelItem* parentFile = fileTreeModel()->fileItem(parentIndex);

	if (parentFile == nullptr || parentFile->fileId() == -1)
	{
		assert(parentFile);
		assert(parentFile->fileId() != -1);
		return;
	}

	selectionModel()->clear();				// clear selction. New selection will be set after files added to db

	// Create files vector
	//
	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
	file->setFileName(fileName);

	// Add files to the DB
	//
	bool ok = db()->addFile(file, parentFile->fileId(), this);

	if (ok == false)
	{
		return;
	}

	// Add files to the FileModel and select them
	//
	std::shared_ptr<FileTreeModelItem> fi = std::make_shared<FileTreeModelItem>(*file);
	fileTreeModel()->addFile(parentIndex, fi);

	// Find and select
	//

	FileTreeModel* sourceModel = fileTreeModel();

	FileTreeProxyModel* proxyModel = fileTreeProxyModel();

	for (int i = 0; i < 65535; i++)
	{
		QModelIndex childIndex = sourceModel->childIndex(i, 0, parentIndex);

		if (childIndex.isValid() == false)
		{
			break;
		}

		FileTreeModelItem* childFile = sourceModel->fileItem(childIndex);

		if (childFile == nullptr)
		{
			assert(childFile);
			break;
		}

		if (file->fileId() == childFile->fileId())
		{
			if (proxyModel == nullptr)
			{
				selectionModel()->select(childIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
			}
			else
			{
				QModelIndex proxyIndex = proxyModel->mapFromSource(childIndex);

				if (proxyIndex.isValid() == true)
				{
					selectionModel()->select(proxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
				}
			}

			break;
		}
	}

	if (isExpanded(parentIndex) == false)
	{
		expand(parentIndex);
	}

	return;
}

void FileTreeView::addFile()
{
	// Find parent file
	//
	QModelIndexList selectedIndexList = selectedSourceRows();

	if (selectedIndexList.size() != 1)
	{
		assert(selectedIndexList.size() == 1);
		return;
	}

	QModelIndex parentIndex = selectedIndexList[0];
	FileTreeModelItem* parentFile = fileTreeModel()->fileItem(parentIndex);

	if (parentFile == nullptr || parentFile->fileId() == -1)
	{
		assert(parentFile);
		assert(parentFile->fileId() != -1);
		return;
	}

	// Select and read files
	//
	QFileDialog fd(this);
	fd.setFileMode(QFileDialog::ExistingFiles);

	if (fd.exec() == QDialog::Rejected)
	{
		return;
	}

	QStringList selectedFiles = fd.selectedFiles();

	selectionModel()->clear();				// clear selction. New selection will be set after files added to db

	// Create files vector
	//
	std::vector<std::shared_ptr<DbFile>> files;

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
			return;
		}

		files.push_back(file);
	}

	// Add files to the DB
	//
	bool ok = db()->addFiles(&files, parentFile->fileId(), this);

	if (ok == false)
	{
		return;
	}

	// Add files to the FileModel and select them
	//
	for (const std::shared_ptr<DbFile>& f : files)
	{
		if (f->fileId() == -1)
		{
			continue;
		}

		std::shared_ptr<FileTreeModelItem> fi = std::make_shared<FileTreeModelItem>(*f);
		fileTreeModel()->addFile(parentIndex, fi);
	}

	// Find and select
	//
	FileTreeModel* sourceModel = fileTreeModel();

	FileTreeProxyModel* proxyModel = fileTreeProxyModel();

	for (int i = 0; i < 65535; i++)
	{
		QModelIndex childIndex = sourceModel->childIndex(i, 0, parentIndex);

		if (childIndex.isValid() == false)
		{
			break;
		}

		FileTreeModelItem* childFile = sourceModel->fileItem(childIndex);

		if (childFile == nullptr)
		{
			assert(childFile);
			break;
		}

		auto findResult = std::find_if(files.begin(), files.end(),
				[childFile](const std::shared_ptr<DbFile>& f)
				{
					return f->fileId() == childFile->fileId();
				}
			);

		if (findResult != files.end())
		{
			QModelIndex proxyIndex = proxyModel->mapFromSource(childIndex);

			if (proxyIndex.isValid() == true)
			{
				selectionModel()->select(proxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
			}
		}
	}

	if (isExpanded(parentIndex) == false)
	{
		expand(parentIndex);
	}

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

		FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
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
				FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
				assert(f);
				return f->fileId() == fi.fileId();
			});

		assert(mipos != selectedIndexList.end());

		if (mipos != selectedIndexList.end())
		{
			fileTreeModel()->updateFile(*mipos, fi);
		}
	}

	return;
}

void FileTreeView::checkOutFile()
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
			// Forbid to check out root "folders"
			//
			continue;
		}

		FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
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
		auto mipos = std::find_if(selectedIndexList.begin(), selectedIndexList.end(),
			[&fi, this](QModelIndex& mi)
			{
				FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
				assert(f);
				return f->fileId() == fi.fileId();
			});

		assert(mipos != selectedIndexList.end());

		if (mipos != selectedIndexList.end())
		{
			fileTreeModel()->updateFile(*mipos, fi);
		}
	}

	return;
}

void FileTreeView::checkInFile()
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
			// Forbid to check in root "folders"
			//
			continue;
		}

		FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
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

	// CheckIn changes to the database
	//
	std::vector<DbFileInfo> checkedInFiles;

	CheckInDialog::checkIn(files, false, &checkedInFiles, db(), this);

	// Update files state
	//
	for (const DbFileInfo& fi : checkedInFiles)
	{
		auto mipos = std::find_if(selectedIndexList.begin(), selectedIndexList.end(),
			[&fi, this](QModelIndex& mi)
			{
				FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
				assert(f);
				return f->fileId() == fi.fileId();
			});

		assert(mipos != selectedIndexList.end());

		if (mipos != selectedIndexList.end())
		{
			fileTreeModel()->updateFile(*mipos, fi);
		}
	}

	return;
}

void FileTreeView::undoChangesFile()
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

		FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
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
		return;
	}

	auto mb = QMessageBox::question(
		this,
		tr("Undo Changes"),
		tr("Do you want to undo pending changes? All selected objects' changes will be lost!"));

	if (mb == QMessageBox::No)
	{
		return;
	}

	// Undo changes
	//
	db()->undoChanges(files, this);

	// Update files state
	//
	for (const DbFileInfo& fi : files)
	{
		auto mipos = std::find_if(selectedIndexList.begin(), selectedIndexList.end(),
			[&fi, this](QModelIndex& mi)
			{
				FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
				assert(f);
				return f->fileId() == fi.fileId();
			});

		assert(mipos != selectedIndexList.end());

		if (mipos != selectedIndexList.end())
		{
			fileTreeModel()->updateFile(*mipos, fi);
		}
	}

	return;
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
	FileTreeModelItem* file = fileTreeModel()->fileItem(selected.first());

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
	FileTreeModelItem* file = fileTreeModel()->fileItem(selected.first());

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

		FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
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

		FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
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

		FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
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

		FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
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
				FileTreeModelItem* f = fileTreeModel()->fileItem(mi);
				assert(f);
				return f->fileId() == fi->fileId();
			});

		assert(mipos != selectedIndexList.end());

		if (mipos != selectedIndexList.end())
		{
			fileTreeModel()->updateFile(*mipos, *fi);
		}
	}


	return;
}

void FileTreeView::refreshFileTree()
{
	fileTreeModel()->refresh();
}

// Protected props
//
FileTreeModel* FileTreeView::fileTreeModel()
{
	FileTreeModel* result  = nullptr;

	FileTreeProxyModel* proxyModel = dynamic_cast<FileTreeProxyModel*>(model());
	if (proxyModel != nullptr)
	{
		result =  dynamic_cast<FileTreeModel*>(proxyModel->sourceModel());
	}
	else
	{
		result = dynamic_cast<FileTreeModel*>(model());
	}

	assert(result);
	return result;
}

FileTreeModel* FileTreeView::fileTreeModel() const
{
	FileTreeModel* result  = nullptr;

	FileTreeProxyModel* proxyModel = dynamic_cast<FileTreeProxyModel*>(model());
	if (proxyModel != nullptr)
	{
		result =  dynamic_cast<FileTreeModel*>(proxyModel->sourceModel());
	}
	else
	{
		result = dynamic_cast<FileTreeModel*>(model());
	}

	assert(result);
	return result;
}

FileTreeProxyModel* FileTreeView::fileTreeProxyModel()
{
	FileTreeProxyModel* proxyModel = dynamic_cast<FileTreeProxyModel*>(model());
	return proxyModel;
}

FileTreeProxyModel* FileTreeView::fileTreeProxyModel() const
{
	FileTreeProxyModel* proxyModel = dynamic_cast<FileTreeProxyModel*>(model());
	return proxyModel;
}

DbController* FileTreeView::db()
{
	return m_dbc;
}

