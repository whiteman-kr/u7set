#include "Stable.h"
#include "FileView.h"
#include "CheckInDialog.h"
#include "../include/DbController.h"


FilesModel::FilesModel(QObject* parent) :
	QAbstractTableModel(parent)
{
}

FilesModel::~FilesModel()
{
}

int FilesModel::rowCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
	return static_cast<int>(m_files.size());
}

int FilesModel::columnCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
	return ColumnCount;
}

QVariant FilesModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
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
				return QVariant(fileInfo->state().text());

			case FileUserColumn:
				return QVariant(fileInfo->userId());

			case FileActionColumn:
				return QVariant(fileInfo->action().text());

			case FileLastCheckInColumn:
				return QVariant(fileInfo->lastCheckIn().toString());

			default:
				return QVariant();
			}
		}
		break;
	}

	return QVariant();
}

QVariant FilesModel::headerData(int section, Qt::Orientation orientation, int role) const
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
			}
		}
	}
	return QVariant();
}

void FilesModel::addFile(std::shared_ptr<DbFileInfo> file)
{
	if (file->fileName().endsWith(m_filter, Qt::CaseInsensitive) == true)
	{
		m_files.push_back(file);
		emit layoutChanged();
	}
}

void FilesModel::setFiles(const std::vector<DbFileInfo> &files)
{
	m_files.clear();

	for (auto f = files.begin(); f != files.end(); ++f)
	{
		std::shared_ptr<DbFileInfo> spf = std::make_shared<DbFileInfo>(*f);

		if (spf->fileName().endsWith(m_filter, Qt::CaseInsensitive) == true)
		{
			m_files.push_back(spf);
		}
	}

	emit layoutChanged();
	return;
}

void FilesModel::clear()
{
	m_files.clear();
	emit layoutChanged();
	return;
}

std::shared_ptr<DbFileInfo> FilesModel::fileByRow(int row)
{
	if (row < 0 || row >= (int)m_files.size())
	{
		assert(row >= 0);
		assert(row < (int)m_files.size());
		return std::shared_ptr<DbFileInfo>();
	}

	return m_files[row];
}

std::shared_ptr<DbFileInfo> FilesModel::fileByFileId(int fileId)
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

int FilesModel::getFileRow(int fileId) const
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

QString FilesModel::filter() const
{
	return m_filter;
}

void FilesModel::setFilter(const QString& value)
{
	m_filter = value;
}

//
//
// FileView
//
//

FileView::FileView()
{
	assert(false);
}

FileView::FileView(const FileView&) :
	QTableView()
{
	assert(false);
}

FileView::FileView(DbController* pDbStore, const QString& parentFileName) :
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
	addAction(m_getWorkcopyAction);
	addAction(m_setWorkcopyAction);

	addAction(m_separatorAction3);
	addAction(m_refreshFileAction);

	// --
	//
	connect(dbController(), &DbController::projectOpened, this, &FileView::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &FileView::projectClosed);

	connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileView::filesViewSelectionChanged);

	return;
}

FileView::~FileView()
{
}

void FileView::CreateActions()
{
	m_openFileAction = new QAction(tr("Open..."), this);
	m_openFileAction->setStatusTip(tr("Open file for edit..."));
	m_openFileAction->setEnabled(false);
	connect(m_openFileAction, &QAction::triggered, this, &FileView::slot_OpenFile);

	m_viewFileAction = new QAction(tr("View..."), this);
	m_viewFileAction->setStatusTip(tr("View file..."));
	m_viewFileAction->setEnabled(false);
	connect(m_viewFileAction, &QAction::triggered, this, &FileView::slot_ViewFile);

	m_separatorAction0 = new QAction(this);
	m_separatorAction0->setSeparator(true);

	m_checkOutAction = new QAction(tr("Check Out"), this);
	m_checkOutAction->setStatusTip(tr("Check Out for edit..."));
	m_checkOutAction->setEnabled(false);
	connect(m_checkOutAction, &QAction::triggered, this, &FileView::slot_CheckOut);

	m_checkInAction = new QAction(tr("Check In"), this);
	m_checkInAction->setStatusTip(tr("Check In changes..."));
	m_checkInAction->setEnabled(false);
	connect(m_checkInAction, &QAction::triggered, this, &FileView::slot_CheckIn);

	m_undoChangesAction = new QAction(tr("Undo Changes..."), this);
	m_undoChangesAction->setStatusTip(tr("Undo Pending Changes..."));
	m_undoChangesAction->setEnabled(false);
	connect(m_undoChangesAction, &QAction::triggered, this, &FileView::slot_UndoChanges);

	m_separatorAction1 = new QAction(this);
	m_separatorAction1->setSeparator(true);

	m_addFileAction = new QAction(tr("Add File..."), this);
	m_addFileAction->setStatusTip(tr("Add file to version control..."));
	m_addFileAction->setEnabled(false);
	connect(m_addFileAction, &QAction::triggered, this, &FileView::slot_AddFile);

	m_deleteFileAction = new QAction(tr("Delete File..."), this);
	m_deleteFileAction ->setStatusTip(tr("Mark file as deleted..."));
	m_deleteFileAction ->setEnabled(false);
	connect(m_deleteFileAction , &QAction::triggered, this, &FileView::slot_DeleteFile);

	m_separatorAction2 = new QAction(this);
	m_separatorAction2->setSeparator(true);

	m_getWorkcopyAction = new QAction(tr("Get Workcopy..."), this);
	m_getWorkcopyAction->setStatusTip(tr("Get file workcopy..."));
	m_getWorkcopyAction->setEnabled(false);
	connect(m_getWorkcopyAction, &QAction::triggered, this, &FileView::slot_GetWorkcopy);

	m_setWorkcopyAction = new QAction(tr("Set Workcopy..."), this);
	m_setWorkcopyAction->setStatusTip(tr("Set file workcopy..."));
	m_setWorkcopyAction->setEnabled(false);
	connect(m_setWorkcopyAction, &QAction::triggered, this, &FileView::slot_SetWorkcopy);

	m_separatorAction3 = new QAction(this);
	m_separatorAction3->setSeparator(true);

	m_refreshFileAction = new QAction(tr("Refresh"), this);
	m_refreshFileAction->setStatusTip(tr("Refresh file list..."));
	m_refreshFileAction->setEnabled(false);
	connect(m_refreshFileAction, &QAction::triggered, this, &FileView::slot_RefreshFiles);

	return;
}

void FileView::setFiles(const std::vector<DbFileInfo>& files)
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
	filesModel().setFiles(files);

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

void FileView::clear()
{
	m_filesModel.clear();
}

void FileView::getSelectedFiles(std::vector<DbFileInfo>* out)
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

void FileView::openFile(std::vector<DbFileInfo> /*files*/)
{
}

void FileView::viewFile(std::vector<DbFileInfo> /*files*/)
{
}

void FileView::checkOut(std::vector<DbFileInfo> files)
{
	dbController()->checkOut(files, this);
	refreshFiles();
}

void FileView::checkIn(std::vector<DbFileInfo> files)
{
	CheckInDialog::checkIn(files, dbController(), this);

	refreshFiles();

	return;
}

void FileView::undoChanges(std::vector<DbFileInfo> files)
{
	dbController()->undoChanges(files, this);
	refreshFiles();
}

void FileView::addFile()
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

void FileView::deleteFile(std::vector<DbFileInfo> files)
{
	assert(false);
}

void FileView::getWorkcopy(std::vector<DbFileInfo> files)
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

void FileView::setWorkcopy(std::vector<DbFileInfo> files)
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

void FileView::refreshFiles()
{
	// Get file list from the DB
	//
	std::vector<DbFileInfo> files;

	dbController()->getFileList(&files, parentFile().fileId(), filesModel().filter(), this);

	// Set files to the view
	//
	setFiles(files);

	return;
}

void FileView::projectOpened()
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

void FileView::projectClosed()
{
	this->setEnabled(false);

	m_addFileAction->setEnabled(false);
	m_refreshFileAction->setEnabled(false);

	m_parentFile = DbFileInfo();

	clear();
}

void FileView::slot_OpenFile()
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

void FileView::slot_ViewFile()
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


void FileView::slot_CheckOut()
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

void FileView::slot_CheckIn()
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

void FileView::slot_UndoChanges()
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

void FileView::slot_AddFile()
{
	addFile();
	return;
}

void FileView::slot_DeleteFile()
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

void FileView::slot_GetWorkcopy()
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

void FileView::slot_SetWorkcopy()
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

void FileView::slot_RefreshFiles()
{
	refreshFiles();

	return;
}

void FileView::filesViewSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
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

	m_getWorkcopyAction->setEnabled(canGetWorkcopy);
	m_setWorkcopyAction->setEnabled(canSetWorkcopy == 1);			// can set work copy just for one file

	m_deleteFileAction->setEnabled(hasDeletePossibility);

	return;

}

FilesModel& FileView::filesModel()
{
	return m_filesModel;
}

const DbFileInfo& FileView::parentFile() const
{
	return m_parentFile;
}

int FileView::parentFileId() const
{
	return m_parentFile.fileId();
}

DbController* FileView::dbController()
{
	return m_dbController;
}
