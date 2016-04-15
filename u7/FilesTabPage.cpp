#include "FilesTabPage.h"

#include <QDir>
#include <QMessageBox>

#include "../include/DbController.h"

#include "DialogFileEditor.h"
#include "CheckInDialog.h"
#include "GlobalMessanger.h"


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

	return std::distance(m_children.begin(), fr);
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

void FileTreeModelItem::sortChildrenByFileName()
{
	std::sort(std::begin(m_children), std::end(m_children),
		[](const std::shared_ptr<FileTreeModelItem>& f1, const std::shared_ptr<FileTreeModelItem>& f2)
		{
			return f1->fileName() < f2->fileName();
		});
	return;
}

//
//
// FileTreeModel
//
//
FileTreeModel::FileTreeModel(DbController* dbcontroller, QWidget* parentWidget, QObject* parent) :
	QAbstractItemModel(parent),
	m_dbc(dbcontroller),
	m_parentWidget(parentWidget),
	m_root(std::make_shared<FileTreeModelItem>())
{
	assert(m_dbc);

	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &FileTreeModel::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &FileTreeModel::projectClosed);
}

FileTreeModel::~FileTreeModel()
{
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
	return ColumnCount;		// Always the same
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

			switch (index.column())
			{
			case FileNameColumn:
				v.setValue<QString>(file->fileName());
				break;

			case FileSizeColumn:
				v.setValue<QString>(QString("%1").arg(file->size()));
				break;

			case FileStateColumn:
				{
					if (file->state() == VcsState::CheckedOut)
					{
						QString state = file->action().text();
						v.setValue<QString>(state);
					}
				}
				break;

			case FileUserColumn:
				if (file->state() == VcsState::CheckedOut)
				{
					v.setValue<qint32>(file->userId());
				}
				break;

			case FileIdColumn:
				v.setValue(file->fileId());
				break;

			case FileDetailsColumn:
				v.setValue<QString>(file->details());
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
	}

	return QVariant();
}

QVariant FileTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal) {
			switch (section)
			{
			case FileNameColumn:
				return QObject::tr("Name");

			case FileSizeColumn:
				return QObject::tr("Size");

			case FileStateColumn:
				return QObject::tr("State");

			case FileUserColumn:
				return QObject::tr("User");

			case FileIdColumn:
				return QObject::tr("FileID");

			case FileDetailsColumn:
				return QObject::tr("Details");

			default:
				assert(false);
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

	parentFile->sortChildrenByFileName();

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
	parentFile->sortChildrenByFileName();

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

	QModelIndex leftIndex = this->index(parentFile->childIndex(childFile), static_cast<int>(FileUserColumn), parentIndex);
	emit dataChanged(index, leftIndex);

	return;
}

void FileTreeModel::refresh()
{
	beginResetModel();
	*m_root.get() = FileTreeModelItem(db()->systemFileInfo(db()->rootFileId()));
	endResetModel();
}

void FileTreeModel::projectOpened()
{
	beginResetModel();
	*m_root.get() = FileTreeModelItem(db()->systemFileInfo(db()->rootFileId()));
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

void FileTreeView::addFile()
{
	// Find parent file
	//
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

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
	for (int i = 0; i < 65535; i++)
	{
		QModelIndex childIndex = parentIndex.child(i, 0);

		if (childIndex.isValid() == false)
		{
			break;
		}

		FileTreeModelItem* childFile = fileTreeModel()->fileItem(childIndex);

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
			selectionModel()->select(childIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		}
	}

	if (isExpanded(parentIndex) == false)
	{
		expand(parentIndex);
	}

	return;
}

void FileTreeView::editFile()
{
    QModelIndexList selectedIndexList = selectionModel()->selectedRows();

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


    std::shared_ptr<DbFile> f;
    if (db()->getLatestVersion(fileInfo, &f, this) == false)
    {
        QMessageBox::critical(this, "Error", "Get latest version error!");
        return;
    }

    QByteArray data;
    f->swapData(data);

    DialogFileEditor d(fileInfo.fileName(), &data, db(), false, this);
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

void FileTreeView::deleteFile()
{
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

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
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

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
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

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
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

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

void FileTreeView::getLatestVersion()
{
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

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
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

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



void FileTreeView::setWorkcopy()
{
	QModelIndexList selectedIndexList = selectionModel()->selectedRows();

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
	FileTreeModel* result = dynamic_cast<FileTreeModel*>(model());
	assert(result);
	return result;
}

FileTreeModel* FileTreeView::fileTreeModel() const
{
	FileTreeModel* result = dynamic_cast<FileTreeModel*>(model());
	assert(result);
	return result;
}

DbController* FileTreeView::db()
{
	return m_dbc;
}


//
//
//	FilesTabPage
//
//
FilesTabPage::FilesTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	assert(dbcontroller != nullptr);

    m_editableExtensions << tr("afb");
	m_editableExtensions << tr("xml");
	m_editableExtensions << tr("xsd");
    m_editableExtensions << tr("descr");

	//
	// Controls
	//
	m_fileView = new FileTreeView(dbcontroller);
	m_fileModel = new FileTreeModel(dbcontroller, this, this);
	m_fileView->setModel(m_fileModel);

	// Create Actions
	//
	createActions();


	//
	// Set context menu to Equipment View
	//
	m_fileView->setContextMenuPolicy(Qt::ActionsContextMenu);

	// -----------------
	m_fileView->addAction(m_addFileAction);
    m_fileView->addAction(m_editFileAction);
	m_fileView->addAction(m_deleteFileAction);

	// -----------------
	m_fileView->addAction(m_SeparatorAction1);
	m_fileView->addAction(m_checkOutAction);
	m_fileView->addAction(m_checkInAction);
	m_fileView->addAction(m_undoChangesAction);
	// -----------------
	m_fileView->addAction(m_SeparatorAction2);
	m_fileView->addAction(m_getLatestVersionAction);
	m_fileView->addAction(m_getLatestTreeVersionAction);
	m_fileView->addAction(m_importWorkingcopyAction);
	// -----------------
	m_fileView->addAction(m_SeparatorAction3);
	m_fileView->addAction(m_refreshAction);
	// -----------------

	//
	// Layouts
	//

	// Left layout (project list)
	//
	QVBoxLayout* pLeftLayout = new QVBoxLayout();
	pLeftLayout->addWidget(m_fileView);

	// Right layout (buttons)
	//
	QVBoxLayout* pRightLayout = new QVBoxLayout();

//	pRightLayout->addWidget(m_pNewProject);

	// Main Layout
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout();
	pMainLayout->addLayout(pLeftLayout);
	pMainLayout->addLayout(pRightLayout);

	setLayout(pMainLayout);

	// --
	//
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &FilesTabPage::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &FilesTabPage::projectClosed);

	connect(m_fileView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FilesTabPage::selectionChanged);
	connect(m_fileModel, &FileTreeModel::dataChanged, this, &FilesTabPage::modelDataChanged);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

void FilesTabPage::createActions()
{

	m_addFileAction = new QAction(tr("Add file"), this);
	m_addFileAction->setStatusTip(tr("Add file..."));
	m_addFileAction->setEnabled(false);
	connect(m_addFileAction, &QAction::triggered, m_fileView, &FileTreeView::addFile);

    m_editFileAction = new QAction(tr("Edit file"), this);
    m_editFileAction->setStatusTip(tr("Edit file..."));
    m_editFileAction->setEnabled(false);
    connect(m_editFileAction, &QAction::triggered, m_fileView, &FileTreeView::editFile);

    m_deleteFileAction = new QAction(tr("Delete file"), this);
	m_deleteFileAction->setStatusTip(tr("Delete file..."));
	m_deleteFileAction->setEnabled(false);
	connect(m_deleteFileAction, &QAction::triggered, m_fileView, &FileTreeView::deleteFile);

	//----------------------------------
	m_SeparatorAction1 = new QAction(this);
	m_SeparatorAction1->setSeparator(true);

	m_checkOutAction = new QAction(tr("CheckOut"), this);
	m_checkOutAction->setStatusTip(tr("Check out file for edit"));
	m_checkOutAction->setEnabled(false);
	connect(m_checkOutAction, &QAction::triggered, m_fileView, &FileTreeView::checkOutFile);

	m_checkInAction = new QAction(tr("CheckIn"), this);
	m_checkInAction->setStatusTip(tr("Check in changes"));
	m_checkInAction->setEnabled(false);
	connect(m_checkInAction, &QAction::triggered, m_fileView, &FileTreeView::checkInFile);

	m_undoChangesAction = new QAction(tr("Undo Changes..."), this);
	m_undoChangesAction->setStatusTip(tr("Undo all pending changes for the object"));
	m_undoChangesAction->setEnabled(false);
	connect(m_undoChangesAction, &QAction::triggered, m_fileView, &FileTreeView::undoChangesFile);

	//----------------------------------
	m_SeparatorAction2 = new QAction(this);
	m_SeparatorAction2->setSeparator(true);

	m_getLatestVersionAction = new QAction(tr("Get Latest Version"), this);
	m_getLatestVersionAction->setStatusTip(tr("Get the latest version (workcopy if cheked out)"));
	m_getLatestVersionAction->setEnabled(false);
	connect(m_getLatestVersionAction, &QAction::triggered, m_fileView, &FileTreeView::getLatestVersion);

	m_getLatestTreeVersionAction = new QAction(tr("Get Latest Tree Version"), this);
	m_getLatestTreeVersionAction->setStatusTip(tr("Get the latest tree version (workcopy if cheked out)"));
	m_getLatestTreeVersionAction->setEnabled(false);
	connect(m_getLatestTreeVersionAction, &QAction::triggered, m_fileView, &FileTreeView::getLatestTreeVersion);

	m_importWorkingcopyAction = new QAction(tr("Import Workingcopy..."), this);
	m_importWorkingcopyAction->setStatusTip(tr("Import workingcopy disk file to project file..."));
	m_importWorkingcopyAction->setEnabled(false);
	connect(m_importWorkingcopyAction, &QAction::triggered, m_fileView, &FileTreeView::setWorkcopy);


	//----------------------------------
	m_SeparatorAction3 = new QAction(this);
	m_SeparatorAction3->setSeparator(true);

	m_refreshAction = new QAction(tr("Refresh"), this);
	m_refreshAction->setStatusTip(tr("Refresh object list"));
	m_refreshAction->setEnabled(false);
	connect(m_refreshAction, &QAction::triggered, m_fileView, &FileTreeView::refreshFileTree);

	return;
}

void FilesTabPage::setActionState()
{
	// Disable all
	//
	m_addFileAction->setEnabled(false);
    m_editFileAction->setEnabled(false);
	m_deleteFileAction->setEnabled(false);
	m_checkOutAction->setEnabled(false);
	m_checkInAction->setEnabled(false);
	m_undoChangesAction->setEnabled(false);
	m_getLatestVersionAction->setEnabled(false);
	m_getLatestTreeVersionAction->setEnabled(false);
	m_importWorkingcopyAction->setEnabled(false);
	m_refreshAction->setEnabled(false);

	if (dbController()->isProjectOpened() == false)
	{
		return;
	}

	// Refresh
	//
	m_refreshAction->setEnabled(true);

	// --
	//
	QModelIndexList selectedIndexList = m_fileView->selectionModel()->selectedRows();

	// Add Action
	//
	m_addFileAction->setEnabled(selectedIndexList.size() == 1);

	// Delete Items action
	//
	m_deleteFileAction->setEnabled(false);
	for (const QModelIndex& mi : selectedIndexList)
	{
		const FileTreeModelItem* file = m_fileModel->fileItem(mi);
		assert(file);

		if (file->state() == VcsState::CheckedIn /*&&
			file->action() != VcsItemAction::Deleted*/)
		{
			m_deleteFileAction->setEnabled(true);
			break;
		}

		if (file->state() == VcsState::CheckedOut &&
			(file->userId() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator())
			&& file->action() != VcsItemAction::Deleted)
		{
			m_deleteFileAction->setEnabled(true);
			break;
		}
	}

	// CheckIn, CheckOut, Undo, Get/set Workcopy
	//
	bool canAnyBeCheckedIn = false;
	bool canAnyBeCheckedOut = false;

	for (const QModelIndex& mi : selectedIndexList)
	{
		const FileTreeModelItem* file = m_fileModel->fileItem(mi);
		assert(file);

		if (file->state() == VcsState::CheckedOut &&
			(file->userId() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator()))
		{
			canAnyBeCheckedIn = true;
		}

		if (file->state() == VcsState::CheckedIn)
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

	m_getLatestVersionAction->setEnabled(selectedIndexList.isEmpty() == false);
	m_getLatestTreeVersionAction->setEnabled(selectedIndexList.isEmpty() == false);
	m_importWorkingcopyAction->setEnabled(canAnyBeCheckedIn && selectedIndexList.size() == 1);

    // Enable edit only files with several extensions!
    //
    bool editableExtension = false;
    for (const QModelIndex& mi : selectedIndexList)
    {
        const FileTreeModelItem* file = m_fileModel->fileItem(mi);
        assert(file);

        QString ext = QFileInfo(file->fileName()).suffix();
        if (m_editableExtensions.contains(ext))
        {
            editableExtension = true;
            break;
        }
    }
    m_editFileAction->setEnabled(editableExtension && canAnyBeCheckedIn && selectedIndexList.size() == 1);

	return;
}

void FilesTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void FilesTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}

void FilesTabPage::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	Q_UNUSED(selected);
	Q_UNUSED(deselected);

	setActionState();

	return;
}

void FilesTabPage::modelDataChanged(const QModelIndex& topLeft,
									const QModelIndex& bottomRight, const QVector<int>& roles /*= QVector<int>()*/)
{
	Q_UNUSED(topLeft);
	Q_UNUSED(bottomRight);
	Q_UNUSED(roles);

	setActionState();

	return;
}
