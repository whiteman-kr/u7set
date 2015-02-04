#include "FilesTabPage.h"
#include "../include/DbController.h"

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

	connect(dbcontroller, &DbController::projectOpened, this, &FileTreeModel::projectOpened);
	connect(dbcontroller, &DbController::projectClosed, this, &FileTreeModel::projectClosed);
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

	bool ok = db()->getFileList(&files, parentFile->fileId(), m_parentWidget);
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

void FileTreeModel::projectOpened()
{
	beginResetModel();
	*m_root.get() = db()->systemFileInfo(db()->rootFileId());
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

	// Create Actions
	//
	CreateActions();

	//
	// Controls
	//
	m_fileView = new FileTreeView(dbcontroller);
	m_fileModel = new FileTreeModel(dbcontroller, this, this);
	m_fileView->setModel(m_fileModel);

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
	connect(dbController(), &DbController::projectOpened, this, &FilesTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &FilesTabPage::projectClosed);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

void FilesTabPage::CreateActions()
{
//	m_checkOutAction = new QAction(tr("Check Out"), this);
//	m_checkOutAction->setStatusTip(tr("Check Out for edit..."));
//	m_checkOutAction->setEnabled(false);
//	connect(m_checkOutAction, &QAction::triggered, this, &FilesTabPage::checkOutFiles);

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
