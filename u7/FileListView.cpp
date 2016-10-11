#include "Stable.h"
#include "FileListView.h"
#include "CheckInDialog.h"
#include "GlobalMessanger.h"
#include "../lib/DbController.h"


FileListModel::FileListModel(QObject* parent) :
	QAbstractTableModel(parent)
{
}

FileListModel::~FileListModel()
{
}

int FileListModel::rowCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
	return static_cast<int>(m_files.size());
}

int FileListModel::columnCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
	return ColumnCount;
}

QVariant FileListModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
	unsigned int row = index.row();
	unsigned int col = index.column();

	switch (role)
	{
	case Qt::DisplayRole:
		{
			if (row >= m_files.size())
			{
				assert(false);
				return QVariant();
			}

			const std::shared_ptr<DbFileInfo>& fileInfo = m_files[row];

			switch (col)
			{
			case FileNameColumn:
				return QVariant(fileInfo->fileName());

			case FileSizeColumn:
				return QVariant(fileInfo->size());

			case FileStateColumn:
				if (fileInfo->state() == VcsState::CheckedIn)
				{
					return QVariant();
				}
				else
				{
					return QVariant(fileInfo->state().text());
				}

			case FileUserColumn:
				return QVariant(usernameById(fileInfo->userId()));

			case FileActionColumn:
				return QVariant(fileInfo->action().text());

			case FileLastCheckInColumn:
				return QVariant(fileInfo->lastCheckIn().toString());

//			case FileIdColumn:
//				return QVariant(fileInfo->fileId());

			case FileIssuesColumn:
				{
					QStringList fn = fileInfo->fileName().split('.');

					if (fn.isEmpty() == false)
					{
						auto gm = GlobalMessanger::instance();
						auto issueCount = gm->issueForSchema(fn.front());

						if (issueCount.errors == 0 && issueCount.warnings == 0)
						{
							return QString();
						}

						if (issueCount.errors > 0 && issueCount.warnings == 0)
						{
							return QString("ERR: %1").arg(issueCount.errors);
						}

						if (issueCount.errors > 0 && issueCount.warnings > 0)
						{
							return QString("ERR: %1, WRN: %2").arg(issueCount.errors).arg(issueCount.warnings);
						}

						if (issueCount.errors == 0 && issueCount.warnings > 0)
						{
							return QString("WRN: %2").arg(issueCount.warnings);
						}

						assert(false);
						return QVariant();
					}
					else
					{
						assert(fn.isEmpty() == false);		// Empty file name?
						return QVariant();
					}

					assert(false);
				}

//			case FileDetailsColumn:
//				return QVariant(fileInfo->details());

			default:
				return QVariant();
			}
		}
		break;

	case Qt::BackgroundRole:
		{
			if (row >= m_files.size())
			{
				assert(false);
				return QVariant();
			}

			const std::shared_ptr<DbFileInfo>& fileInfo = m_files[row];

			if (fileInfo->state() == VcsState::CheckedOut)
			{
				QBrush b(QColor(0xFF, 0xFF, 0xFF));

				switch (static_cast<VcsItemAction::VcsItemActionType>(fileInfo->action().toInt()))
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

	case Qt::TextColorRole:
		{
			if (row >= m_files.size())
			{
				assert(false);
				return QVariant();
			}

			const std::shared_ptr<DbFileInfo>& fileInfo = m_files[row];

			if (col == FileIssuesColumn)
			{
				QStringList fn = fileInfo->fileName().split('.');

				if (fn.isEmpty() == false)
				{
					auto gm = GlobalMessanger::instance();
					auto issueCount = gm->issueForSchema(fn.front());

					if (issueCount.errors > 0)
					{
						return QBrush(QColor(0xE0, 0x33, 0x33, 0xFF));
					}

					if (issueCount.warnings > 0)
					{
						return QBrush(QColor(0xE8, 0x72, 0x17, 0xFF));
					}

					return QVariant();
				}
				else
				{
					assert(fn.isEmpty() == false);		// Empty file name?
					return QVariant();
				}
			}
			else
			{
				return QVariant();
			}
		}
		break;
	}

	return QVariant();
}

QVariant FileListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal) {
			switch (section)
			{
			case FileNameColumn:
				return QObject::tr("File Name");

			case FileSizeColumn:
				return QObject::tr("Size");

			case FileStateColumn:
				return QObject::tr("State");

			case FileUserColumn:
				return QObject::tr("User");

			case FileActionColumn:
				return QObject::tr("Action");

			case FileLastCheckInColumn:
				return QObject::tr("Last Check In");

//			case FileIdColumn:
//				return QObject::tr("FileID");

			case FileIssuesColumn:
				return QObject::tr("Issues");

//			case FileDetailsColumn:
//				return QObject::tr("Details");

			default:
				assert(false);
			}
		}
	}
	return QVariant();
}

void FileListModel::sort(int column, Qt::SortOrder order/* = Qt::AscendingOrder*/)
{
	if (m_files.empty() == true)
	{
		return;
	}

	emit layoutAboutToBeChanged(QList<QPersistentModelIndex>(), QAbstractItemModel::VerticalSortHint);

	QModelIndexList pers = persistentIndexList();
	std::vector<std::shared_ptr<DbFileInfo>> oldFileOrder(m_files);

	switch (column)
	{
	case FileNameColumn:
		std::sort(m_files.begin(), m_files.end(),
				  [order](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
		{
			QString n1 = f1->fileName();
			QString n2 = f2->fileName();

			if (order == Qt::AscendingOrder)
			{
				return n2 < n1;
			}
			else
			{
				return n1 < n2;
			}
		});
		break;

	case FileSizeColumn:
		std::sort(m_files.begin(), m_files.end(),
				  [order](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
		{
			int s1 = f1->size();
			int s2 = f2->size();

			if (order == Qt::AscendingOrder)
			{
				return s2 < s1;
			}
			else
			{
				return s1 < s2;
			}
		});
		break;

	case FileStateColumn:
		std::sort(m_files.begin(), m_files.end(),
				  [order](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
		{
			QString s1 = f1->state().text();
			QString s2 = f2->state().text();

			if (order == Qt::AscendingOrder)
			{
				return s2 < s1;
			}
			else
			{
				return s1 < s2;
			}
		});
		break;

	case FileUserColumn:
		std::sort(m_files.begin(), m_files.end(),
				  [order, this](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
		{
			int uid1 = f1->userId();
			int uid2 = f2->userId();

			QString u1 = this->usernameById(uid1);
			QString u2 = this->usernameById(uid2);

			if (order == Qt::AscendingOrder)
			{
				return u2 < u1;
			}
			else
			{
				return u1 < u2;
			}
		});
		break;

	case FileActionColumn:
		std::sort(m_files.begin(), m_files.end(),
				  [order](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
		{
			QString a1 = f1->action().text();
			QString a2 = f2->action().text();

			if (order == Qt::AscendingOrder)
			{
				return a2 < a1;
			}
			else
			{
				return a1 < a2;
			}
		});
		break;

	case FileLastCheckInColumn:
		std::sort(m_files.begin(), m_files.end(),
				  [order](std::shared_ptr<DbFileInfo> f1, std::shared_ptr<DbFileInfo> f2)
		{
			QDateTime c1 = f1->created();
			QDateTime c2 = f2->created();

			if (order == Qt::AscendingOrder)
			{
				return c2 < c1;
			}
			else
			{
				return c1 < c2;
			}
		});
		break;

	default:
		assert(false);
	}

	// Move pers indexes
	//
	for (QModelIndex& oldIndex : pers)
	{
		int oldIndexRow = oldIndex.row();
		if (oldIndexRow < 0 || oldIndex.row() >= static_cast<int>(oldFileOrder.size()))
		{
			assert(oldIndex.row() < static_cast<int>(oldFileOrder.size()));
			continue;
		}

		std::shared_ptr<DbFileInfo> oldFile = oldFileOrder.at(oldIndexRow);
		QModelIndex newIndex = index(getFileRow(oldFile->fileId()), oldIndex.column());

		if (oldIndex != newIndex)
		{
			changePersistentIndex(oldIndex, newIndex);
		}
	}

	emit layoutChanged();
}

void FileListModel::addFile(std::shared_ptr<DbFileInfo> file)
{
	if (file->fileName().endsWith(m_filter, Qt::CaseInsensitive) == true)
	{
		emit layoutAboutToBeChanged();
		m_files.push_back(file);
		emit layoutChanged();
	}
}

void FileListModel::setFiles(const std::vector<DbFileInfo> &files, const std::vector<DbUser>& users)
{
	emit layoutAboutToBeChanged();

	QModelIndexList pers = persistentIndexList();
	std::vector<std::shared_ptr<DbFileInfo>> oldFileOrder(m_files);

	// --
	//
	m_files.clear();

	for (auto f = files.begin(); f != files.end(); ++f)
	{
		std::shared_ptr<DbFileInfo> spf = std::make_shared<DbFileInfo>(*f);

		if (spf->fileName().endsWith(m_filter, Qt::CaseInsensitive) == true)
		{
			m_files.push_back(spf);
		}
	}

	// Move pers indexes
	//
	for (QModelIndex& oldIndex : pers)
	{
		int oldIndexRow = oldIndex.row();
		if (oldIndexRow < 0 || oldIndex.row() >= static_cast<int>(oldFileOrder.size()))
		{
			assert(oldIndex.row() < static_cast<int>(oldFileOrder.size()));
			continue;
		}

		std::shared_ptr<DbFileInfo> oldFile = oldFileOrder.at(oldIndexRow);
		QModelIndex newIndex = index(getFileRow(oldFile->fileId()), oldIndex.column());

		if (oldIndex != newIndex)
		{
			changePersistentIndex(oldIndex, newIndex);
		}
	}

	emit layoutChanged();

	// Set users
	//
	m_users.clear();
	for (const DbUser& u : users)
	{
		m_users[u.userId()] = u.username();
	}

	return;
}

void FileListModel::clear()
{
	beginResetModel();
	m_files.clear();
	endResetModel();

	m_users.clear();
	return;
}

std::shared_ptr<DbFileInfo> FileListModel::fileByRow(int row)
{
	if (row < 0 || row >= (int)m_files.size())
	{
		assert(row >= 0);
		assert(row < (int)m_files.size());
		return std::shared_ptr<DbFileInfo>();
	}

	return m_files[row];
}

std::shared_ptr<DbFileInfo> FileListModel::fileByFileId(int fileId)
{
	if (fileId == -1)
	{
		return std::shared_ptr<DbFileInfo>();
	}

    auto it = std::find_if(m_files.begin(), m_files.end(),
		[fileId](const std::shared_ptr<DbFileInfo>& f)
		{
			return f->fileId() == fileId;
		});

	if (it == m_files.end())
	{
		return std::shared_ptr<DbFileInfo>();
	}
	else
	{
		return *it;
	}
}

int FileListModel::getFileRow(int fileId) const
{
	int row = 0;
	for (auto it = m_files.begin(); it != m_files.end(); ++it)
	{
		if (it->get()->fileId() == fileId)
		{
			return row;
		}

		row ++;
	}
	return -1;
}

QString FileListModel::filter() const
{
	return m_filter;
}

void FileListModel::setFilter(const QString& value)
{
	m_filter = value;
}

const std::vector<std::shared_ptr<DbFileInfo>>& FileListModel::files() const
{
	return m_files;
}

QString FileListModel::usernameById(int userId) const
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

//
//
// FileView
//
//

FileListView::FileListView()
{
	assert(false);
}

FileListView::FileListView(const FileListView&) :
	QTableView()
{
	assert(false);
}

FileListView::FileListView(DbController* pDbStore, const QString& parentFileName) :
	m_dbController(pDbStore),
	m_parentFileName(parentFileName)
{
	assert(m_dbController != nullptr);
	assert(m_parentFileName.isEmpty() == false);

	// --
	//
	CreateActions();

	// Adjust view
	//
	setModel(&m_filesModel);

	setShowGrid(false);
	setSortingEnabled(true);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::ExtendedSelection);

	// Adjust headers
	//
	verticalHeader()->hide();
	verticalHeader()->setDefaultSectionSize(static_cast<int>(fontMetrics().height() * 1.4));
	horizontalHeader()->setHighlightSections(false);

	// Set context menu
	//
	setContextMenuPolicy(Qt::ActionsContextMenu);

	addAction(m_openFileAction);
	addAction(m_viewFileAction);
	addAction(m_separatorAction0);

	addAction(m_checkOutAction);
	addAction(m_checkInAction);
	addAction(m_undoChangesAction);

	addAction(m_separatorAction1);
	addAction(m_addFileAction);
	addAction(m_deleteFileAction);

	addAction(m_separatorAction2);
	addAction(m_exportWorkingcopyAction);
	addAction(m_importWorkingcopyAction);

	addAction(m_separatorAction3);
	addAction(m_refreshFileAction);

	// --
	//
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &FileListView::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &FileListView::projectClosed);

	connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileListView::filesViewSelectionChanged);

	connect(this, &QTableView::doubleClicked, this, &FileListView::slot_doubleClicked);

	return;
}

FileListView::~FileListView()
{
}

void FileListView::CreateActions()
{
	m_openFileAction = new QAction(tr("Open..."), this);
	m_openFileAction->setStatusTip(tr("Open file for edit..."));
	m_openFileAction->setEnabled(false);
	connect(m_openFileAction, &QAction::triggered, this, &FileListView::slot_OpenFile);

	m_viewFileAction = new QAction(tr("View..."), this);
	m_viewFileAction->setStatusTip(tr("View file..."));
	m_viewFileAction->setEnabled(false);
	connect(m_viewFileAction, &QAction::triggered, this, &FileListView::slot_ViewFile);

	m_separatorAction0 = new QAction(this);
	m_separatorAction0->setSeparator(true);

	m_checkOutAction = new QAction(tr("Check Out"), this);
	m_checkOutAction->setStatusTip(tr("Check Out for edit..."));
	m_checkOutAction->setEnabled(false);
	connect(m_checkOutAction, &QAction::triggered, this, &FileListView::slot_CheckOut);

	m_checkInAction = new QAction(tr("Check In"), this);
	m_checkInAction->setStatusTip(tr("Check In changes..."));
	m_checkInAction->setEnabled(false);
	connect(m_checkInAction, &QAction::triggered, this, &FileListView::slot_CheckIn);

	m_undoChangesAction = new QAction(tr("Undo Changes..."), this);
	m_undoChangesAction->setStatusTip(tr("Undo Pending Changes..."));
	m_undoChangesAction->setEnabled(false);
	connect(m_undoChangesAction, &QAction::triggered, this, &FileListView::slot_UndoChanges);

	m_separatorAction1 = new QAction(this);
	m_separatorAction1->setSeparator(true);

	m_addFileAction = new QAction(tr("Add File..."), this);
	m_addFileAction->setStatusTip(tr("Add file to version control..."));
	m_addFileAction->setEnabled(false);
	connect(m_addFileAction, &QAction::triggered, this, &FileListView::slot_AddFile);

	m_deleteFileAction = new QAction(tr("Delete File..."), this);
	m_deleteFileAction ->setStatusTip(tr("Mark file as deleted..."));
	m_deleteFileAction ->setEnabled(false);
	connect(m_deleteFileAction , &QAction::triggered, this, &FileListView::slot_DeleteFile);

	m_separatorAction2 = new QAction(this);
	m_separatorAction2->setSeparator(true);

	m_exportWorkingcopyAction = new QAction(tr("Export Workingcopy..."), this);
	m_exportWorkingcopyAction->setStatusTip(tr("Export workingcopy file to disk..."));
	m_exportWorkingcopyAction->setEnabled(false);
	connect(m_exportWorkingcopyAction, &QAction::triggered, this, &FileListView::slot_GetWorkcopy);

	m_importWorkingcopyAction = new QAction(tr("Import Workingcopy..."), this);
	m_importWorkingcopyAction->setStatusTip(tr("Import workingcopy from disk file to project file..."));
	m_importWorkingcopyAction->setEnabled(false);
	connect(m_importWorkingcopyAction, &QAction::triggered, this, &FileListView::slot_SetWorkcopy);

	m_separatorAction3 = new QAction(this);
	m_separatorAction3->setSeparator(true);

	m_refreshFileAction = new QAction(tr("Refresh"), this);
	m_refreshFileAction->setStatusTip(tr("Refresh file list..."));
	m_refreshFileAction->setEnabled(false);
	connect(m_refreshFileAction, &QAction::triggered, this, &FileListView::slot_RefreshFiles);

	return;
}

void FileListView::setFiles(const std::vector<DbFileInfo>& files)
{
	// Save old selection
	//
	QItemSelectionModel* selModel = this->selectionModel();
	QModelIndexList	s = selModel->selectedRows();

	std::vector<int> filesIds;
	filesIds.reserve(s.size());

	for (int i = 0; i < s.size(); i++)
	{
		std::shared_ptr<DbFileInfo> file = filesModel().fileByRow(s[i].row());
		if (file.get() == nullptr)
		{
			continue;
		}

		int fileId = file->fileId();
		if (fileId != -1)
		{
			filesIds.push_back(fileId);
		}
	}

	selModel->clear();

	// Get file list from the DB
	//
	std::vector<DbUser> users;
	dbController()->getUserList(&users, this);

	filesModel().setFiles(files, users);

	// Restore selection
	//
	for (unsigned int i = 0; i < filesIds.size(); i++)
	{
		int selectRow = filesModel().getFileRow(filesIds[i]);

		if (selectRow != -1)
		{
			QModelIndex md = filesModel().index(selectRow, 0);
			selModel->select(md, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		}
	}

	return;
}

void FileListView::clear()
{
	m_filesModel.clear();
}

void FileListView::getSelectedFiles(std::vector<DbFileInfo>* out)
{
	if (out == nullptr)
	{
		assert(out != nullptr);
		return;
	}

	out->clear();

	QItemSelectionModel* selModel = selectionModel();
	if (selModel->hasSelection() == false)
	{
		return;
	}

	QModelIndexList	sel = selModel->selectedRows();

	out->reserve(sel.size());

	for (int i = 0; i < sel.size(); i++)
	{
		QModelIndex mi = sel[i];

		auto file = m_filesModel.fileByRow(mi.row());

		if (file.get() != nullptr)
		{
			out->push_back(*file.get());
		}
		else
		{
			assert(file.get() != nullptr);
		}
	}

	return;
}

void FileListView::openFile(std::vector<DbFileInfo> /*files*/)
{
}

void FileListView::viewFile(std::vector<DbFileInfo> /*files*/)
{
}

void FileListView::checkOut(std::vector<DbFileInfo> files)
{
	dbController()->checkOut(files, this);
	refreshFiles();
}

void FileListView::checkIn(std::vector<DbFileInfo> files)
{
	std::vector<DbFileInfo> updatedFiles;

	CheckInDialog::checkIn(files, false, &updatedFiles, dbController(), this);

	refreshFiles();

	return;
}

void FileListView::undoChanges(std::vector<DbFileInfo> files)
{
	dbController()->undoChanges(files, this);
	refreshFiles();
}

void FileListView::addFile()
{
	assert(false);
	// Add Parent ID!!!!!!!!!!!!!!!!!!!!


	/*QFileDialog fd(this);
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
	dbController()->addFiles(&files, this);

	// Add files to the FileModel and select them
	//
	for (int i = 0; i < selectedFiles.size(); i++)
	{
		std::shared_ptr<DbFileInfo> file = std::make_shared<DbFileInfo>(*files[i].get());

		if (file->fileId() != -1)
		{
			filesModel().addFile(file);

			int fileRow = filesModel().getFileRow(file->fileId());

			if (fileRow != -1)
			{
				QModelIndex md = filesModel().index(fileRow, 0);		// m_filesModel.columnCount()
				selectionModel()->select(md, QItemSelectionModel::Select | QItemSelectionModel::Rows);
			}
		}
	}

	filesViewSelectionChanged(QItemSelection(), QItemSelection());*/
}

void FileListView::deleteFile(std::vector<DbFileInfo> /*files*/)
{
	assert(false);
}

void FileListView::getWorkcopy(std::vector<DbFileInfo> files)
{
	// Select destination folder
	//
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty() == true)
	{
		return;
	}

	// Get files from the database
	//
	std::vector<std::shared_ptr<DbFile>> out;
	dbController()->getWorkcopy(files, &out, this);

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

void FileListView::setWorkcopy(std::vector<DbFileInfo> files)
{
	if (files.size() != 1)
	{
		return;
	}

	auto fileInfo = files[0];

	if (fileInfo.state() != VcsState::CheckedOut || fileInfo.userId() != dbController()->currentUser().userId())
	{
		return;
	}

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

	std::vector<std::shared_ptr<DbFile>> workcopyFiles;
	workcopyFiles.push_back(file);

	dbController()->setWorkcopy(workcopyFiles, this);

	refreshFiles();

	return;
}

void FileListView::refreshFiles()
{
	// Get file list from the DB
	//
	std::vector<DbFileInfo> files;

	dbController()->getFileList(&files, parentFile().fileId(), filesModel().filter(), true, this);

	// Set files to the view
	//
	setFiles(files);

	setSortingEnabled(true);	// it triggers setSortingEnabled() with the current sort section and order.

	return;
}

void FileListView::fileDoubleClicked(DbFileInfo /*file*/)
{
}


void FileListView::projectOpened()
{
	this->setEnabled(true);

	filesViewSelectionChanged(QItemSelection(), QItemSelection());

	m_addFileAction->setEnabled(true);
	m_refreshFileAction->setEnabled(true);

	m_parentFile = dbController()->systemFileInfo(m_parentFileName);
	assert(m_parentFile.fileId() != -1);

	// Refresh file list in FileModel
	//
	refreshFiles();

	return;
}

void FileListView::projectClosed()
{
	this->setEnabled(false);

	m_addFileAction->setEnabled(false);
	m_refreshFileAction->setEnabled(false);

	m_parentFile = DbFileInfo();

	clear();
}

void FileListView::slot_OpenFile()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file.state() == VcsState::CheckedOut &&
			(file.fileId() == dbController()->currentUser().userId() || dbController()->currentUser().isAdminstrator()))
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	openFile(files);

	return;
}

void FileListView::slot_ViewFile()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];
		files.push_back(file);
	}

	if (files.empty() == true)
	{
		return;
	}

	viewFile(files);

	return;
}


void FileListView::slot_CheckOut()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file.state() == VcsState::CheckedIn)
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	checkOut(files);

	return;
}

void FileListView::slot_CheckIn()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file.userId() == dbController()->currentUser().userId())
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	checkIn(files);

	return;
}

void FileListView::slot_UndoChanges()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file.userId() == dbController()->currentUser().userId())
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	undoChanges(files);

	return;
}

void FileListView::slot_AddFile()
{
	addFile();

	//  setSortingEnabled() triggers a call to sortByColumn() with the current sort section and order.
	//
	setSortingEnabled(true);

	return;
}

void FileListView::slot_DeleteFile()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file.state() == VcsState::CheckedIn ||
			(file.state() == VcsState::CheckedOut && file.userId() == dbController()->currentUser().userId()))
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	deleteFile(files);
	return;
}

void FileListView::slot_GetWorkcopy()
{
	// Get files workcopies form the database
	//
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file.state() == VcsState::CheckedOut && file.userId() == dbController()->currentUser().userId())
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	getWorkcopy(files);

	return;
}

void FileListView::slot_SetWorkcopy()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file.state() == VcsState::CheckedOut && file.userId() == dbController()->currentUser().userId())
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	setWorkcopy(files);

	return;
}

void FileListView::slot_RefreshFiles()
{
	refreshFiles();

	return;
}

void FileListView::slot_doubleClicked(const QModelIndex& index)
{
	if (index.isValid() == true)
	{
		std::shared_ptr<DbFileInfo> file = m_filesModel.fileByRow(index.row());

		if (file.get() != nullptr)
		{
			fileDoubleClicked(*(file.get()));
		}
		else
		{
			assert(file.get() != nullptr);
		}
	}

	return;
}

void FileListView::filesViewSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
	QModelIndexList	s = selectionModel()->selectedRows();

	bool hasOpenPossibility = false;
	int hasViewPossibility = false;
	bool hasCheckOutPossibility = false;
	bool hasCheckInPossibility = false;
	bool hasUndoPossibility = false;
	bool canGetWorkcopy = false;
	int canSetWorkcopy = 0;
	bool hasDeletePossibility = false;

	int currentUserId = dbController()->currentUser().userId();;

	for (auto i = s.begin(); i != s.end(); ++i)
	{
		const std::shared_ptr<DbFileInfo> fileInfo = filesModel().fileByRow(i->row());

		// hasViewPossibility -- almost any file SINGLE can be opened
		//
		hasViewPossibility ++;

		// hasOpenPossibility -- almost any file can be opened
		//
		if (fileInfo->state() == VcsState::CheckedOut && fileInfo->userId() == currentUserId)
		{
			hasOpenPossibility = true;
		}

		// hasCheckInPossibility
		//
		if (fileInfo->state() == VcsState::CheckedOut && fileInfo->userId() == currentUserId)
		{
			hasCheckInPossibility = true;
		}

		// hasUndoPossibility
		//
		if (fileInfo->state() == VcsState::CheckedOut && fileInfo->userId() == currentUserId)
		{
			hasUndoPossibility = true;
		}

		// hasCheckOutPossibility
		//
		if (fileInfo->state() == VcsState::CheckedIn)
		{
			hasCheckOutPossibility = true;
		}

		// canGetWorkcopy, canSetWorkcopy
		//
		if (fileInfo->state() == VcsState::CheckedOut && fileInfo->userId() == currentUserId)
		{
			canGetWorkcopy = true;
			canSetWorkcopy ++;
		}

		// hasDeletePossibility
		if ((fileInfo->state() == VcsState::CheckedOut && fileInfo->userId() == currentUserId) ||
			fileInfo->state() == VcsState::CheckedIn)
		{
			hasDeletePossibility = true;
		}
	}

	m_openFileAction->setEnabled(hasOpenPossibility);
	m_viewFileAction->setEnabled(hasViewPossibility == 1);

	m_checkOutAction->setEnabled(hasCheckOutPossibility);
	m_checkInAction->setEnabled(hasCheckInPossibility);
	m_undoChangesAction->setEnabled(hasUndoPossibility);

	m_exportWorkingcopyAction->setEnabled(canGetWorkcopy);
	m_importWorkingcopyAction->setEnabled(canSetWorkcopy == 1);			// can set work copy just for one file

	m_deleteFileAction->setEnabled(hasDeletePossibility);

	return;

}

FileListModel& FileListView::filesModel()
{
	return m_filesModel;
}

const std::vector<std::shared_ptr<DbFileInfo>>& FileListView::files() const
{
	return m_filesModel.files();
}

const DbFileInfo& FileListView::parentFile() const
{
	return m_parentFile;
}

int FileListView::parentFileId() const
{
	return m_parentFile.fileId();
}

DbController* FileListView::dbController()
{
	return m_dbController;
}
