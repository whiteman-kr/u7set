#include "Stable.h"
#include <QJsonArray>
#include "SchemaTabPage.h"
#include "CreateSchemaDialog.h"
#include "Forms/SelectChangesetDialog.h"
#include "Forms/FileHistoryDialog.h"
#include "Forms/CompareDialog.h"
#include "Forms/ComparePropertyObjectDialog.h"
#include "CheckInDialog.h"
#include "GlobalMessanger.h"
#include "../lib/PropertyEditor.h"

//
//
//	SchemaFileView
//
//
SchemaFileView::SchemaFileView(DbController* dbcontroller, const QString& parentFileName) :
	HasDbController(dbcontroller),
	m_parentFileName(parentFileName)
{
	assert(dbcontroller != nullptr);
	assert(m_parentFileName.isEmpty() == false);

	// --
	//
	filesModel().setFilter("vfr");

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
	verticalHeader()->setResizeMode(QHeaderView::Fixed);

	horizontalHeader()->setHighlightSections(false);


	setColumnWidth(static_cast<int>(SchemaListModel::FileNameColumn), 180);
	setColumnWidth(static_cast<int>(SchemaListModel::FileCaptionColumn), 400);
	setColumnWidth(static_cast<int>(SchemaListModel::FileDetailsColumn), 250);

	// Set context menu
	//
	setContextMenuPolicy(Qt::ActionsContextMenu);

	addAction(m_openFileAction);
	addAction(m_viewFileAction);
	addAction(m_separatorAction0);

	addAction(m_checkOutAction);
	addAction(m_checkInAction);
	addAction(m_undoChangesAction);
	addAction(m_historyAction);
	addAction(m_compareAction);
	addAction(m_allSchemasHistoryAction);

	addAction(m_separatorAction1);
	addAction(m_addFileAction);
	addAction(m_cloneFileAction);
	addAction(m_deleteFileAction);

	addAction(m_separatorAction2);
	addAction(m_exportWorkingcopyAction);
	addAction(m_importWorkingcopyAction);

	addAction(m_separatorAction3);
	addAction(m_refreshFileAction);

	addAction(m_propertiesAction);

	// --
	//
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SchemaFileView::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SchemaFileView::projectClosed);

	connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &SchemaFileView::filesViewSelectionChanged);

	connect(this, &QTableView::doubleClicked, this, &SchemaFileView::slot_doubleClicked);

	setFont(qApp->font());

	return;
}

void SchemaFileView::CreateActions()
{
	m_openFileAction = new QAction(tr("Open..."), this);
	m_openFileAction->setStatusTip(tr("Open file for edit..."));
	m_openFileAction->setEnabled(false);
	connect(m_openFileAction, &QAction::triggered, this, &SchemaFileView::slot_OpenFile);

	m_viewFileAction = new QAction(tr("View..."), this);
	m_viewFileAction->setStatusTip(tr("View file..."));
	m_viewFileAction->setEnabled(false);
	connect(m_viewFileAction, &QAction::triggered, this, &SchemaFileView::slot_ViewFile);

	m_separatorAction0 = new QAction(this);
	m_separatorAction0->setSeparator(true);

	m_checkOutAction = new QAction(tr("Check Out"), this);
	m_checkOutAction->setStatusTip(tr("Check Out for edit..."));
	m_checkOutAction->setEnabled(false);
	connect(m_checkOutAction, &QAction::triggered, this, &SchemaFileView::slot_CheckOut);

	m_checkInAction = new QAction(tr("Check In"), this);
	m_checkInAction->setStatusTip(tr("Check In changes..."));
	m_checkInAction->setEnabled(false);
	connect(m_checkInAction, &QAction::triggered, this, &SchemaFileView::slot_CheckIn);

	m_undoChangesAction = new QAction(tr("Undo Changes..."), this);
	m_undoChangesAction->setStatusTip(tr("Undo Pending Changes..."));
	m_undoChangesAction->setEnabled(false);
	connect(m_undoChangesAction, &QAction::triggered, this, &SchemaFileView::slot_UndoChanges);

	m_historyAction = new QAction(tr("History..."), this);
	m_historyAction->setStatusTip(tr("Show file history..."));
	m_historyAction->setEnabled(false);
	connect(m_historyAction, &QAction::triggered, this, &SchemaFileView::slot_showHistory);

	m_compareAction = new QAction(tr("Compare..."), this);
	m_compareAction->setStatusTip(tr("Compare file..."));
	m_compareAction->setEnabled(false);
	connect(m_compareAction, &QAction::triggered, this, &SchemaFileView::slot_compare);

	m_allSchemasHistoryAction = new QAction(tr("All Schemas History..."), this);
	m_allSchemasHistoryAction->setStatusTip(tr("Show all schemas history..."));
	m_allSchemasHistoryAction->setEnabled(true);
	connect(m_allSchemasHistoryAction, &QAction::triggered, this, &SchemaFileView::slot_showHistoryForAllSchemas);

	m_separatorAction1 = new QAction(this);
	m_separatorAction1->setSeparator(true);

	m_addFileAction = new QAction(tr("Add File..."), this);
	m_addFileAction->setStatusTip(tr("Add file to version control..."));
	m_addFileAction->setEnabled(false);
	connect(m_addFileAction, &QAction::triggered, this, &SchemaFileView::slot_AddFile);

	m_cloneFileAction = new QAction(tr("Clone..."), this);
	m_cloneFileAction->setStatusTip(tr("Clone file..."));
	m_cloneFileAction->setEnabled(false);
	connect(m_cloneFileAction, &QAction::triggered, this, &SchemaFileView::slot_cloneFile);

	m_deleteFileAction = new QAction(tr("Delete File..."), this);
	m_deleteFileAction ->setStatusTip(tr("Mark file as deleted..."));
	m_deleteFileAction ->setEnabled(false);
	connect(m_deleteFileAction , &QAction::triggered, this, &SchemaFileView::slot_DeleteFile);

	m_separatorAction2 = new QAction(this);
	m_separatorAction2->setSeparator(true);

	m_exportWorkingcopyAction = new QAction(tr("Export Workingcopy..."), this);
	m_exportWorkingcopyAction->setStatusTip(tr("Export workingcopy file to disk..."));
	m_exportWorkingcopyAction->setEnabled(false);
	connect(m_exportWorkingcopyAction, &QAction::triggered, this, &SchemaFileView::slot_GetWorkcopy);

	m_importWorkingcopyAction = new QAction(tr("Import Workingcopy..."), this);
	m_importWorkingcopyAction->setStatusTip(tr("Import workingcopy from disk file to project file..."));
	m_importWorkingcopyAction->setEnabled(false);
	connect(m_importWorkingcopyAction, &QAction::triggered, this, &SchemaFileView::slot_SetWorkcopy);

	m_separatorAction3 = new QAction(this);
	m_separatorAction3->setSeparator(true);

	m_refreshFileAction = new QAction(tr("Refresh"), this);
	m_refreshFileAction->setStatusTip(tr("Refresh file list..."));
	m_refreshFileAction->setEnabled(false);
	connect(m_refreshFileAction, &QAction::triggered, this, &SchemaFileView::slot_RefreshFiles);

	m_propertiesAction = new QAction(tr("Properties..."), this);
	m_propertiesAction->setStatusTip(tr("Edit schema properties..."));
	m_propertiesAction->setEnabled(false);
	connect(m_propertiesAction, &QAction::triggered, this, &SchemaFileView::slot_properties);

	return;
}

void SchemaFileView::setFiles(const std::vector<DbFileInfo>& files)
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

	selModel->reset();

	// Get file list from the DB
	//
	std::vector<DbUser> users;
	db()->getUserList(&users, this);

	filesModel().setFiles(files, users);

	// Restore selection
	//
	selModel->blockSignals(true);
	for (unsigned int i = 0; i < filesIds.size(); i++)
	{
		int selectRow = filesModel().getFileRow(filesIds[i]);

		if (selectRow != -1)
		{
			QModelIndex md = filesModel().index(selectRow, 0);
			selModel->select(md, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		}
	}
	selModel->blockSignals(false);

	return;
}

void SchemaFileView::clear()
{
	m_filesModel.clear();
}

void SchemaFileView::getSelectedFiles(std::vector<DbFileInfo>* out)
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

void SchemaFileView::refreshFiles()
{
	// Get file list from the DB
	//
	std::vector<DbFileInfo> files;

	db()->getFileList(&files, parentFile().fileId(), filesModel().filter(), true, this);

	// Set files to the view
	//
	setFiles(files);

	setSortingEnabled(true);	// it triggers setSortingEnabled() with the current sort section and order.

	return;
}

void SchemaFileView::projectOpened()
{
	this->setEnabled(true);

	filesViewSelectionChanged(QItemSelection(), QItemSelection());

	m_addFileAction->setEnabled(true);
	m_refreshFileAction->setEnabled(true);

	m_parentFile = db()->systemFileInfo(m_parentFileName);
	assert(m_parentFile.fileId() != -1);

	// Refresh file list in FileModel
	//
	refreshFiles();

	return;
}

void SchemaFileView::projectClosed()
{
	this->setEnabled(false);

	m_addFileAction->setEnabled(false);
	m_refreshFileAction->setEnabled(false);

	m_parentFile = DbFileInfo();

	clear();
}

void SchemaFileView::slot_OpenFile()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file.state() == VcsState::CheckedOut &&
			(file.userId() == db()->currentUser().userId() || db()->currentUser().isAdminstrator()))
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	emit openFileSignal(files);

	return;
}

void SchemaFileView::slot_ViewFile()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	if (selectedFiles.empty() == true)
	{
		return;
	}

	emit viewFileSignal(selectedFiles);

	return;
}

void SchemaFileView::slot_CheckOut()
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

	db()->checkOut(files, this);
	refreshFiles();

	return;
}

void SchemaFileView::slot_CheckIn()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file.userId() == db()->currentUser().userId() ||
			db()->currentUser().isAdminstrator() == true)
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	emit checkInSignal(files);

	return;
}

void SchemaFileView::slot_UndoChanges()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (size_t i = 0; i < selectedFiles.size(); i++)
	{
		DbFileInfo& file = selectedFiles[i];

		if (file.state() != VcsState::CheckedOut)
		{
			continue;
		}

		if (file.userId() == db()->currentUser().userId() ||
			db()->currentUser().isAdminstrator() == true)
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	emit undoChangesSignal(files);

	return;
}

void SchemaFileView::slot_showHistory()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	if (selectedFiles.size() != 1)
	{
		return;
	}

	// Get file history
	//
	DbFileInfo file = selectedFiles.front();
	std::vector<DbChangeset> fileHistory;

	bool ok = db()->getFileHistoryRecursive(file, &fileHistory, this);
	if (ok == false)
	{
		return;
	}

	// Show history dialog
	//
	FileHistoryDialog::showHistory(db(), file.fileName(), fileHistory, this);

	return;
}

void SchemaFileView::slot_compare()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	if (selectedFiles.size() != 1)
	{
		return;
	}

	// --
	//
	DbFileInfo file = selectedFiles.front();

	CompareDialog::showCompare(db(), DbChangesetObject(file), -1, this);

	return;
}

void SchemaFileView::slot_showHistoryForAllSchemas()
{
	// Get file history
	//
	std::vector<DbChangeset> fileHistory;

	bool ok = db()->getFileHistoryRecursive(m_parentFile, &fileHistory, this);
	if (ok == false)
	{
		return;
	}

	// Show history dialog
	//
	FileHistoryDialog::showHistory(db(), m_parentFile.fileName(), fileHistory, this);

	return;
}

void SchemaFileView::slot_AddFile()
{
	emit addFileSignal();

	//  setSortingEnabled() triggers a call to sortByColumn() with the current sort section and order.
	//
	setSortingEnabled(true);

	return;
}

void SchemaFileView::slot_cloneFile()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	if (selectedFiles.size() != 1)
	{
		return;
	}

	emit cloneFileSignal(selectedFiles.front());

	return;
}

void SchemaFileView::slot_DeleteFile()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file.state() == VcsState::CheckedIn ||
			(file.state() == VcsState::CheckedOut && file.userId() == db()->currentUser().userId()))
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	emit deleteFileSignal(files);

	return;
}

void SchemaFileView::slot_GetWorkcopy()
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

		if (file.state() == VcsState::CheckedOut && file.userId() == db()->currentUser().userId())
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	// --
	//
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
	db()->getWorkcopy(files, &out, this);

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

void SchemaFileView::slot_SetWorkcopy()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	std::vector<DbFileInfo> files;
	files.reserve(selectedFiles.size());

	for (unsigned int i = 0; i < selectedFiles.size(); i++)
	{
		auto file = selectedFiles[i];

		if (file.state() == VcsState::CheckedOut && file.userId() == db()->currentUser().userId())
		{
			files.push_back(file);
		}
	}

	if (files.empty() == true)
	{
		return;
	}

	// --
	//
	if (files.size() != 1)
	{
		return;
	}

	auto fileInfo = files[0];

	if (fileInfo.state() != VcsState::CheckedOut || fileInfo.userId() != db()->currentUser().userId())
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

	db()->setWorkcopy(workcopyFiles, this);

	refreshFiles();

	return;
}

void SchemaFileView::slot_RefreshFiles()
{
	refreshFiles();
	return;
}

void SchemaFileView::slot_doubleClicked(const QModelIndex& index)
{
	if (index.isValid() == true)
	{
		std::shared_ptr<DbFileInfo> file = m_filesModel.fileByRow(index.row());

		if (file.get() != nullptr)
		{
			std::vector<DbFileInfo> v;
			v.push_back(*file.get());

			if (file->state() == VcsState::CheckedOut)
			{
				emit openFileSignal(v);
			}
			else
			{
				emit viewFileSignal(v);
			}
		}
		else
		{
			assert(file.get() != nullptr);
		}
	}

	return;
}

void SchemaFileView::slot_properties()
{
	std::vector<DbFileInfo> selectedFiles;
	getSelectedFiles(&selectedFiles);

	if (selectedFiles.empty() == true)
	{
		return;
	}

	emit editSchemasProperties(selectedFiles);

	return;
}

void SchemaFileView::filesViewSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
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
	bool schemaPoperties = (s.empty() == false);

	int currentUserId = db()->currentUser().userId();
	bool currentUserIsAdmin = db()->currentUser().isAdminstrator();

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
		if (fileInfo->state() == VcsState::CheckedOut &&
			(fileInfo->userId() == currentUserId  || currentUserIsAdmin == true))
		{
			hasCheckInPossibility = true;
		}

		// hasUndoPossibility
		//
		if (fileInfo->state() == VcsState::CheckedOut &&
			(fileInfo->userId() == currentUserId || currentUserIsAdmin == true))
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
	m_cloneFileAction->setEnabled(hasViewPossibility == 1);

	m_checkOutAction->setEnabled(hasCheckOutPossibility);
	m_checkInAction->setEnabled(hasCheckInPossibility);
	m_undoChangesAction->setEnabled(hasUndoPossibility);
	m_historyAction->setEnabled(s.size() == 1);
	m_compareAction->setEnabled(s.size() == 1);

	m_exportWorkingcopyAction->setEnabled(canGetWorkcopy);
	m_importWorkingcopyAction->setEnabled(canSetWorkcopy == 1);			// can set work copy just for one file

	m_deleteFileAction->setEnabled(hasDeletePossibility);

	m_propertiesAction->setEnabled(schemaPoperties);

	return;
}


SchemaListModel& SchemaFileView::filesModel()
{
	return m_filesModel;
}

const std::vector<std::shared_ptr<DbFileInfo>>& SchemaFileView::files() const
{
	return m_filesModel.files();
}

const DbFileInfo& SchemaFileView::parentFile() const
{
	return m_parentFile;
}

int SchemaFileView::parentFileId() const
{
	return m_parentFile.fileId();
}



//
//
// SchemasTabPage
//
//
SchemasTabPage::SchemasTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	m_tabWidget = new QTabWidget();
	m_tabWidget->setMovable(true);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 6, 0, 0);

	layout->addWidget(m_tabWidget);

	setLayout(layout);

	// --
	//
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SchemasTabPage::projectOpened);
	connect(GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SchemasTabPage::projectClosed);

	connect(GlobalMessanger::instance(), &GlobalMessanger::compareObject, this, &SchemasTabPage::compareObject);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

SchemasTabPage::~SchemasTabPage()
{
}

bool SchemasTabPage::hasUnsavedSchemas() const
{
	for (int i = 0; i < m_tabWidget->count(); i++)
	{
		QWidget* tab = m_tabWidget->widget(i);

		if (tab == nullptr)
		{
			assert(tab);
			continue;
		}

		EditSchemaTabPage* schemaTabPage = dynamic_cast<EditSchemaTabPage*>(tab);
		if (schemaTabPage != nullptr)
		{
			if (schemaTabPage->modified() == true)
			{
				return true;
			}
		}
	}

	return false;
}

bool SchemasTabPage::saveUnsavedSchemas()
{
	bool ok = true;

	for (int i = 0; i < m_tabWidget->count(); i++)
	{
		QWidget* tab = m_tabWidget->widget(i);

		if (tab == nullptr)
		{
			assert(tab);
			continue;
		}

		EditSchemaTabPage* schemaTabPage = dynamic_cast<EditSchemaTabPage*>(tab);
		if (schemaTabPage != nullptr)
		{
			if (schemaTabPage->modified() == true)
			{
				ok &= schemaTabPage->saveWorkcopy();
			}
		}
	}

	return ok;
}

void SchemasTabPage::refreshControlTabPage()
{
	assert(m_tabWidget);

	for (int i = 0; i < m_tabWidget->count(); i++)
	{
		QWidget* tabPage = m_tabWidget->widget(i);

		if (qobject_cast<SchemaControlTabPage*>(tabPage) != nullptr)
		{
			SchemaControlTabPage* controlTabPage = dynamic_cast<SchemaControlTabPage*>(tabPage);
			controlTabPage->refreshFiles();
			break;
		}
	}

	return;
}

std::vector<EditSchemaTabPage*> SchemasTabPage::getOpenSchemas()
{
	std::vector<EditSchemaTabPage*> result;
	result.reserve(32);

	for (int i = 0; i < m_tabWidget->count(); i++)
	{
		QWidget* tab = m_tabWidget->widget(i);

		if (tab == nullptr)
		{
			assert(tab);
			continue;
		}

		EditSchemaTabPage* schemaTabPage = dynamic_cast<EditSchemaTabPage*>(tab);
		if (schemaTabPage != nullptr)
		{
			result.push_back(schemaTabPage);
		}
	}

	return result;
}

void SchemasTabPage::projectOpened()
{
	this->setEnabled(true);
}

void SchemasTabPage::projectClosed()
{
	GlobalMessanger::instance()->clearBuildSchemaIssues();
	GlobalMessanger::instance()->clearSchemaItemRunOrder();

	// Close all opened documents
	//
	assert(m_tabWidget);

	for (int i = m_tabWidget->count() - 1; i >= 0; i--)
	{
		QWidget* tabPage = m_tabWidget->widget(i);

		if (dynamic_cast<SchemaControlTabPage*>(tabPage) == nullptr)
		{
			int tabIndex = m_tabWidget->indexOf(tabPage);
			assert(tabIndex != -1);

			if (tabIndex != -1)
			{
				m_tabWidget->removeTab(i);
				delete tabPage;
			}
		}
	}

	this->setEnabled(false);
	return;
}

void SchemasTabPage::compareObject(DbChangesetObject object, CompareData compareData)
{
	// Can compare only files which are EquipmentObjects
	//
	if (object.isFile() == false)
	{
		return;
	}

	// Check file extension,
	// can compare	next files
	//
	if (object.name().endsWith("." + m_fileExtension) == false &&
		object.name().endsWith("." + m_templFileExtension) == false)
	{
		return;
	}

	// Get versions from the project database
	//
	std::shared_ptr<VFrame30::Schema> source = nullptr;

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
				source = VFrame30::Schema::Create(outFile->data());
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
				source = VFrame30::Schema::Create(outFile->data());
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
				source = VFrame30::Schema::Create(outFile->data());
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
	std::shared_ptr<VFrame30::Schema> target = nullptr;

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
				target = VFrame30::Schema::Create(outFile->data());
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
				target = VFrame30::Schema::Create(outFile->data());
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
				target = VFrame30::Schema::Create(outFile->data());
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

	// Make single schema
	//
	std::map<QUuid, CompareAction> itemsActions;

	for (std::shared_ptr<VFrame30::SchemaLayer> targetLayer : target->Layers)
	{
		for (std::shared_ptr<VFrame30::SchemaItem> targetItem : targetLayer->Items)
		{
			// Look for this item in source
			//
			std::shared_ptr<VFrame30::SchemaItem> sourceItem = source->getItemById(targetItem->guid());

			if (sourceItem != nullptr)
			{
				// Item is found, so it was modified
				//

				// Check if properties where modified
				//
				QString sourceStr = ComparePropertyObjectDialog::objedctToCompareString(sourceItem.get());
				QString targetStr = ComparePropertyObjectDialog::objedctToCompareString(targetItem.get());

				if (sourceStr == targetStr)
				{
					// Check if position was changed
					//
					std::vector<VFrame30::SchemaPoint> sourcePoints = sourceItem->getPointList();
					std::vector<VFrame30::SchemaPoint> targetPoints = targetItem->getPointList();

					if (sourcePoints == targetPoints)
					{
						itemsActions[targetItem->guid()] = CompareAction::Unmodified;
					}
					else
					{
						itemsActions[targetItem->guid()] = CompareAction::Modified;
					}
				}
				else
				{
					itemsActions[targetItem->guid()] = CompareAction::Modified;
				}

				continue;
			}

			if (sourceItem == nullptr)
			{
				// Item was added to targer
				//
				itemsActions[targetItem->guid()] = CompareAction::Added;
				continue;
			}
		}
	}

	// Look for deteled items (in target)
	//
	for (std::shared_ptr<VFrame30::SchemaLayer> sourceLayer : source->Layers)
	{
		for (std::shared_ptr<VFrame30::SchemaItem> sourceItem : sourceLayer->Items)
		{
			// Look for this item in source
			//
			std::shared_ptr<VFrame30::SchemaItem> targetItem = target->getItemById(sourceItem->guid());

			if (targetItem == nullptr)
			{
				// Item is found, so it was deleted in target
				//
				itemsActions[sourceItem->guid()] = CompareAction::Deleted;

				// Add item to target
				//
				bool layerFound = false;
				for (std::shared_ptr<VFrame30::SchemaLayer> targetLayer : target->Layers)
				{
					if (targetLayer->guid() == sourceLayer->guid())
					{
						targetLayer->Items.push_back(sourceItem);
						layerFound = true;
						break;
					}
				}

				assert(layerFound);
			}
		}
	}

	// Create tab page and add it to TabWidget
	//
	EditSchemaTabPage* compareTabPage = new EditSchemaTabPage(m_tabWidget, target, DbFileInfo(), db());

	compareTabPage->setReadOnly(true);
	compareTabPage->setCompareWidget(true, source, target);
	compareTabPage->setCompareItemActions(itemsActions);

	m_tabWidget->addTab(compareTabPage, "Compare " + target->schemaId());
	m_tabWidget->setCurrentWidget(compareTabPage);

	return;
}



//
//
// SchemaControlTabPage
//
//
SchemaControlTabPage::SchemaControlTabPage(QString fileExt,
										   DbController* db,
										   QString parentFileName,
										   QString templateFileExtension,
										   std::function<VFrame30::Schema*()> createSchemaFunc) :
		HasDbController(db),
		m_createSchemaFunc(createSchemaFunc),
		m_templateFileExtension(templateFileExtension)
{
	// Create controls
	//
	m_filesView = new SchemaFileView(db, parentFileName);
	m_filesView->filesModel().setFilter("." + fileExt);

	m_searchEdit = new QLineEdit(this);
	m_searchEdit->setPlaceholderText(tr("Search Text"));
	m_searchEdit->setMinimumWidth(400);

	m_searchButton = new QPushButton(tr("Search"));

	QGridLayout* layout = new QGridLayout(this);
	layout->addWidget(m_filesView, 0, 0, 1, 6);
	layout->addWidget(m_searchEdit, 1, 0, 1, 2);
	layout->addWidget(m_searchButton, 1, 2, 1, 1);
	layout->setColumnStretch(4, 100);

	setLayout(layout);

	m_searchAction = new QAction(tr("Edit search"), this);
	m_searchAction->setShortcut(QKeySequence::Find);

	this->addAction(m_searchAction);

	// Actions
	//
	m_refreshAction = new QAction(tr("Refresh"), this);
	m_refreshAction->setShortcut(QKeySequence::StandardKey::Refresh);
	connect(m_refreshAction, &QAction::triggered, this, &SchemaControlTabPage::refreshFiles);
	addAction(m_refreshAction);

	// --
	//
	connect(m_filesView, &SchemaFileView::openFileSignal, this, &SchemaControlTabPage::openFiles);
	connect(m_filesView, &SchemaFileView::viewFileSignal, this, &SchemaControlTabPage::viewFiles);
	connect(m_filesView, &SchemaFileView::cloneFileSignal, this, &SchemaControlTabPage::cloneFile);
	connect(m_filesView, &SchemaFileView::addFileSignal, this, &SchemaControlTabPage::addFile);
	connect(m_filesView, &SchemaFileView::deleteFileSignal, this, &SchemaControlTabPage::deleteFile);
	connect(m_filesView, &SchemaFileView::checkInSignal, this, &SchemaControlTabPage::checkIn);
	connect(m_filesView, &SchemaFileView::undoChangesSignal, this, &SchemaControlTabPage::undoChanges);
	connect(m_filesView, &SchemaFileView::editSchemasProperties, this, &SchemaControlTabPage::editSchemasProperties);

	connect(m_searchAction, &QAction::triggered, this, &SchemaControlTabPage::ctrlF);
	connect(m_searchEdit, &QLineEdit::returnPressed, this, &SchemaControlTabPage::search);
	connect(m_searchButton, &QPushButton::clicked, this, &SchemaControlTabPage::search);

	auto schema = createSchemaFunc();

	if (schema->isLogicSchema() == true)
	{
		connect(GlobalMessanger::instance(), &GlobalMessanger::addLogicSchema, this, &SchemaControlTabPage::addLogicSchema);
		connect(GlobalMessanger::instance(), &GlobalMessanger::searchSchemaForLm, this, &SchemaControlTabPage::searchSchemaForLm);
	}

	return;
}


SchemaControlTabPage::~SchemaControlTabPage()
{
}

VFrame30::Schema* SchemaControlTabPage::createSchema() const
{
	return m_createSchemaFunc();
}

void SchemaControlTabPage::refreshFiles()
{
	assert(m_filesView);
	m_filesView->refreshFiles();
	return;
}

void SchemaControlTabPage::addLogicSchema(QStringList deviceStrIds, QString lmDescriptionFile)
{
	// Create new Schema and add it to the vcs
	//
	std::shared_ptr<VFrame30::Schema> schema(m_createSchemaFunc());

	if (schema->isLogicSchema() == false)
	{
		assert(schema->isLogicSchema());
		return;
	}

	// Set New Guid
	//
	schema->setGuid(QUuid::createUuid());

	int sequenceNo = db()->nextCounterValue();

	// Set default properties
	//
	schema->setSchemaId("APPSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0'));
	schema->setCaption("Caption "  + QString::number(sequenceNo).rightJustified(6, '0'));

	schema->setDocWidth(420.0 / 25.4);
	schema->setDocHeight(297.0 / 25.4);

	VFrame30::LogicSchema* logicSchema = dynamic_cast<VFrame30::LogicSchema*>(schema.get());
	logicSchema->setEquipmentIdList(deviceStrIds);

	logicSchema->setPropertyValue(Hardware::PropertyNames::lmDescriptionFile, QVariant(lmDescriptionFile));

	// --
	//
	addSchemaFile(schema, false);

	QTabWidget* parentTabWidget = dynamic_cast<QTabWidget*>(this->parentWidget()->parentWidget());
	if (parentTabWidget == nullptr)
	{
		assert(parentTabWidget);
	}
	else
	{
		parentTabWidget->setCurrentWidget(this);
	}

	GlobalMessanger::instance()->fireChangeCurrentTab(this->parentWidget()->parentWidget()->parentWidget());

	m_filesView->setFocus();

	return;
}

void SchemaControlTabPage::addFile()
{
	// Create new Schema and add it to the vcs
	//
	std::shared_ptr<VFrame30::Schema> schema(m_createSchemaFunc());

	// Set New Guid
	//
	schema->setGuid(QUuid::createUuid());

	int sequenceNo = db()->nextCounterValue();

	// Set default ID
	//
	QString defaultId = "SCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');

	if (schema->isLogicSchema() == true)
	{
		defaultId = "APPSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
	}

	if (schema->isUfbSchema() == true)
	{
		defaultId = "UFBID" + QString::number(sequenceNo).rightJustified(6, '0');
	}

	if (schema->isMonitorSchema() == true)
	{
		defaultId = "MONITORSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
	}

	if (schema->isDiagSchema() == true)
	{
		defaultId = "DIAGSCHEMAID" + QString::number(sequenceNo).rightJustified(6, '0');
	}

	schema->setSchemaId(defaultId);

	// Set Caption
	//
	schema->setCaption("Caption "  + QString::number(sequenceNo).rightJustified(6, '0'));

	// Set default EqupmnetIDs for LogicSchema
	//
	if (dynamic_cast<VFrame30::LogicSchema*>(schema.get()) != nullptr)
	{
		VFrame30::LogicSchema* logicSchema = dynamic_cast<VFrame30::LogicSchema*>(schema.get());
		logicSchema->setEquipmentIds("SYSTEMID_RACKID_CH01_MD00");
	}

	// Set Width and Height
	//
	if (schema->unit() == VFrame30::SchemaUnit::Display)
	{
		schema->setDocWidth(1280);
		schema->setDocHeight(1024);
	}
	else
	{
		// A3 Landscape
		//
		if (schema->isUfbSchema() == true)
		{
			schema->setDocWidth(297.0 / 25.4);
			schema->setDocHeight(210.0 / 25.4);
		}
		else
		{
			schema->setDocWidth(420.0 / 25.4);
			schema->setDocHeight(297.0 / 25.4);
		}
	}

	addSchemaFile(schema, false);

	return;
}

void SchemaControlTabPage::addSchemaFile(std::shared_ptr<VFrame30::Schema> schema, bool dontShowPropDialog)
{
	// Show dialog to edit schema properties
	//
	if (dontShowPropDialog == false)
	{
		CreateSchemaDialog propertiesDialog(schema, db(), parentFile().fileId(), m_templateFileExtension, this);

		if (propertiesDialog.exec() != QDialog::Accepted)
		{
			return;
		}
	}

	//  Save file in DB
	//
	QByteArray data;
	schema->Save(data);

	std::shared_ptr<DbFile> vfFile = std::make_shared<DbFile>();

	vfFile->setFileName(schema->schemaId() + m_filesView->filesModel().filter());
	vfFile->setDetails(schema->details());
	vfFile->swapData(data);

	std::vector<std::shared_ptr<DbFile>> addFilesList;
	addFilesList.push_back(vfFile);

	db()->addFiles(&addFilesList, parentFile().fileId(), this);

	// Add file to the FileModel and select it
	//
	std::shared_ptr<DbFileInfo> file = std::make_shared<DbFileInfo>(*vfFile.get());

	if (file->fileId() != -1)
	{
		m_filesView->selectionModel()->clear();

		m_filesView->filesModel().addFile(file);

		int fileRow = m_filesView->filesModel().getFileRow(file->fileId());

		if (fileRow != -1)
		{
			QModelIndex md = m_filesView->filesModel().index(fileRow, 0);		// m_filesModel.columnCount()
			m_filesView->selectionModel()->select(md, QItemSelectionModel::Select | QItemSelectionModel::Rows);

			m_filesView->scrollTo(md);
		}
	}

	m_filesView->filesViewSelectionChanged(QItemSelection(), QItemSelection());
	return;
}

void SchemaControlTabPage::deleteFile(std::vector<DbFileInfo> files)
{
	if (files.empty() == true)
	{
		assert(files.empty() == false);
		return;
	}

	// Ask user to confirm operation
	//
	QMessageBox mb(this);
	mb.setWindowTitle(qApp->applicationName());
	mb.setText(tr("Are you sure you want to delete selected file(s)"));
	mb.setInformativeText(tr("If files have not been checked in before they will be deleted permanently.\nIf files were checked in at least one time they will be marked as deleted, to confirm operation perform Check In."));
	mb.setIcon(QMessageBox::Question);
	mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

	int mbResult = mb.exec();

	if (mbResult == QMessageBox::Cancel)
	{
		return;
	}

	// --
	//
	std::vector<std::shared_ptr<DbFileInfo>> deleteFiles;

	for(const DbFileInfo& f : files)
	{
		deleteFiles.push_back(std::make_shared<DbFileInfo>(f));
	}

	db()->deleteFiles(&deleteFiles, this);

	refreshFiles();

	// Update open tab pages
	//
	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
	if (tabWidget == nullptr)
	{
		assert(tabWidget != nullptr);
		return;
	}

	for (int i = 0; i < tabWidget->count(); i++)
	{
		EditSchemaTabPage* tb = dynamic_cast<EditSchemaTabPage*>(tabWidget->widget(i));
		if (tb == nullptr)
		{
			// It can be control tab page
			//
			continue;
		}

		for (std::shared_ptr<DbFileInfo> fi: deleteFiles)
		{
			if (tb->fileInfo().fileId() == fi->fileId() && tb->readOnly() == false)
			{
				tb->setReadOnly(true);
				tb->setFileInfo(*(fi.get()));
				tb->setPageTitle();
				break;
			}
		}
	}

	return;
}

void SchemaControlTabPage::checkIn(std::vector<DbFileInfo> files)
{
	if (files.empty() == true)
	{
		return;
	}

	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
	if (tabWidget == nullptr)
	{
		assert(tabWidget != nullptr);
		return;
	}

	// Save file if it is open
	//
	for (int i = 0; i < tabWidget->count(); i++)
	{
		EditSchemaTabPage* tb = dynamic_cast<EditSchemaTabPage*>(tabWidget->widget(i));
		if (tb == nullptr)
		{
			// It can be control tab page
			//
			continue;
		}

		for (const DbFileInfo& fi : files)
		{
			if (tb->fileInfo().fileId() == fi.fileId() && tb->readOnly() == false && tb->modified() == true)
			{
				tb->saveWorkcopy();
				break;
			}
		}
	}

	// Check in file
	//
	std::vector<DbFileInfo> updatedFiles;

	bool ok = CheckInDialog::checkIn(files, false, &updatedFiles, db(), this);
	if (ok == false)
	{
		return;
	}

	refreshFiles();

	// Refresh fileInfo from the Db
	//
	std::vector<int> fileIds;
	fileIds.reserve(files.size());

	for (const DbFileInfo& fi : files)
	{
		fileIds.push_back(fi.fileId());
	}

	db()->getFileInfo(&fileIds, &files, this);

	// Remove deleted files
	//
	std::remove_if(std::begin(files), std::end(files),
		[](const DbFileInfo& file)
		{
			return file.deleted();
		});


	// Set readonly to file if it is open
	//
	for (int i = 0; i < tabWidget->count(); i++)
	{
		EditSchemaTabPage* tb = dynamic_cast<EditSchemaTabPage*>(tabWidget->widget(i));
		if (tb == nullptr)
		{
			// It can be control tab page
			//
			continue;
		}

		for (const DbFileInfo& fi : files)
		{
			if (tb->fileInfo().fileId() == fi.fileId() && tb->readOnly() == false)
			{
				tb->setReadOnly(true);
				tb->setFileInfo(fi);
				break;
			}
		}
	}

	return;
}

void SchemaControlTabPage::undoChanges(std::vector<DbFileInfo> files)
{
	// 1 Ask user to confirm operation
	// 2 Undo changes to database
	// 3 Set frame to readonly mode
	//

	std::vector<DbFileInfo> undoFiles;

	for (const DbFileInfo& fi : files)
	{
		if (fi.state() == VcsState::CheckedOut &&
			(fi.userId() == db()->currentUser().userId() || db()->currentUser().isAdminstrator() == true))
		{
			undoFiles.push_back(fi);
		}
	}

	if (undoFiles.empty() == true)
	{
		// Nothing to undo
		//
		return;
	}

	QMessageBox mb(this);
	mb.setText(tr("This operation will undo all pending changes for the document and will revert it to the prior state!"));
	mb.setInformativeText(tr("Do you want to undo pending changes?"));
	mb.setIcon(QMessageBox::Question);
	mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

	if (mb.exec() != QMessageBox::Ok)
	{
		return;
	}

	// Undo changes in DB
	//
	db()->undoChanges(undoFiles, this);

	// Update open tab pages
	//
	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
	if (tabWidget == nullptr)
	{
		assert(tabWidget != nullptr);
		return;
	}

	for (int i = 0; i < tabWidget->count(); i++)
	{
		EditSchemaTabPage* tb = dynamic_cast<EditSchemaTabPage*>(tabWidget->widget(i));
		if (tb == nullptr)
		{
			// It can be control tab page
			//
			continue;
		}

		for (const DbFileInfo& fi : undoFiles)
		{
			if (tb->fileInfo().fileId() == fi.fileId() && tb->readOnly() == false)
			{
				tb->setReadOnly(true);
				tb->setFileInfo(fi);
				tb->setPageTitle();
				break;
			}
		}
	}

	refreshFiles();
}

void SchemaControlTabPage::openFiles(std::vector<DbFileInfo> files)
{
	if (files.empty() == true || files.size() != 1)
	{
		assert(files.empty() == false);
		return;
	}

	const DbFileInfo file = files[0];

	if (file.state() != VcsState::CheckedOut)
	{
		QMessageBox mb(this);
		mb.setText(tr("Check out file for edit first."));
		mb.exec();
		return;
	}

	if (file.state() == VcsState::CheckedOut &&
		file.userId() != db()->currentUser().userId())
	{
		QMessageBox mb(this);
		QString username = db()->username(file.userId());
		mb.setText(tr("File %1 is already checked out by user <b>%2</b>.").arg(file.fileName()).arg(username));
		mb.exec();
		return;
	}

	assert(file.state() == VcsState::CheckedOut && file.userId() == db()->currentUser().userId());

	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
	if (tabWidget == nullptr)
	{
		assert(tabWidget != nullptr);
		return;
	}

	// Check if file already open, and activate file tab if it is
	//

	// Find the opened file, bu filId
	//
	for (int i = 0; i < tabWidget->count(); i++)
	{
		EditSchemaTabPage* tb = dynamic_cast<EditSchemaTabPage*>(tabWidget->widget(i));
		if (tb == nullptr)
		{
			// It can be control tab page
			//
			continue;
		}

		if (tb->fileInfo().fileId() == file.fileId() &&
			tb->fileInfo().changeset() == file.changeset() &&
			tb->readOnly() == false)
		{
			tabWidget->setCurrentIndex(i);
			return;
		}
	}

	// Get file from the DB
	//
	std::vector<std::shared_ptr<DbFile>> out;

	bool result = db()->getWorkcopy(files, &out, this);
	if (result == false || out.size() != files.size())
	{
		QMessageBox::critical(this, tr("Error"), "Can't get file from the database.");
		return;
	}

	// Load file
	//
	std::shared_ptr<VFrame30::Schema> vf(VFrame30::Schema::Create(out[0].get()->data()));

	if (vf == nullptr)
	{
		assert(vf != nullptr);
		return;
	}

	// Create TabPage and add it to the TabControl
	//
	DbFileInfo fi(*(out.front().get()));

	EditSchemaTabPage* editTabPage = new EditSchemaTabPage(tabWidget, vf, fi, db());

	connect(editTabPage, &EditSchemaTabPage::vcsFileStateChanged, this, &SchemaControlTabPage::refreshFiles);

	assert(tabWidget->parent());

	SchemasTabPage* schemasTabPage = dynamic_cast<SchemasTabPage*>(tabWidget->parent());
	if (schemasTabPage == nullptr)
	{
		assert(dynamic_cast<SchemasTabPage*>(tabWidget->parent()));
		return;
	}

	connect(GlobalMessanger::instance(), &GlobalMessanger::buildStarted, editTabPage, &EditSchemaTabPage::saveWorkcopy);

	// --
	//
	editTabPage->setReadOnly(false);

	tabWidget->addTab(editTabPage, editTabPage->windowTitle());
	tabWidget->setCurrentWidget(editTabPage);

	// Update AFBs/UFBs after creating tab page, so it will be possible to set new (modified) caption
	// to the tab page title
	//
	editTabPage->updateAfbSchemaItems();
	editTabPage->updateUfbSchemaItems();
	editTabPage->updateBussesSchemaItems();

	return;
}

void SchemaControlTabPage::viewFiles(std::vector<DbFileInfo> files)
{
	if (files.empty() == true || files.size() != 1)
	{
		assert(files.empty() == false);
		return;
	}

	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
	if (tabWidget == nullptr)
	{
		assert(tabWidget != nullptr);
		return;
	}

	const DbFileInfo file = files[0];

	// Show chageset dialog
	//
	int changesetId = SelectChangesetDialog::getFileChangeset(db(), file, this);

	if (changesetId == -1)
	{
		return;
	}

	// Get file with choosen changeset
	//
	std::vector<std::shared_ptr<DbFile>> out;

	bool result = db()->getSpecificCopy(files, changesetId, &out, this);
	if (result == false || out.size() != files.size())
	{
		return;
	}

	DbFileInfo fi(*(out.front().get()));

	// Load file
	//
	std::shared_ptr<VFrame30::Schema> vf(VFrame30::Schema::Create(out[0].get()->data()));

	QString tabPageTitle;
	tabPageTitle = QString("%1: %2 ReadOnly").arg(vf->schemaId()).arg(changesetId);

	// Find the opened file,
	//
	for (int i = 0; i < tabWidget->count(); i++)
	{
		EditSchemaTabPage* tb = dynamic_cast<EditSchemaTabPage*>(tabWidget->widget(i));
		if (tb == nullptr)
		{
			// It can be control tab page
			//
			continue;
		}

		if (tb->fileInfo().fileId() == fi.fileId() &&
			tb->fileInfo().changeset() == fi.changeset() &&
			tb->readOnly() == true)
		{
			tabWidget->setCurrentIndex(i);
			return;
		}
	}

	// Create TabPage and add it to the TabControl
	//

	EditSchemaTabPage* editTabPage = new EditSchemaTabPage(tabWidget, vf, fi, db());
	editTabPage->setReadOnly(true);

	tabWidget->addTab(editTabPage, tabPageTitle);
	tabWidget->setCurrentWidget(editTabPage);

	return;
}

void SchemaControlTabPage::cloneFile(DbFileInfo file)
{
	// Get file from the DB
	//
	std::shared_ptr<DbFile> out;

	bool result = db()->getLatestVersion(file, &out, this);
	if (result == false || out == nullptr)
	{
		return;
	}

	// Load file
	//
	std::shared_ptr<VFrame30::Schema> schema(VFrame30::Schema::Create(out->data()));
	if (schema == nullptr)
	{
		assert(schema != nullptr);
		return;
	}

	// Get new SchemaID
	//
	bool ok = false;
	int globalCounter = db()->nextCounterValue();
	QString newSchemaId = QInputDialog::getText(this, qAppName(), tr("New SchemaID <b>(cannot be changed later)</b>:"),
												QLineEdit::Normal,
												schema->schemaId() + QString::number(globalCounter), &ok,
												Qt::WindowFlags() & (~Qt::WindowContextHelpButtonHint));

	if (ok == false || newSchemaId.isEmpty() == true)
	{
		return;
	}

	// Set new lables and guids
	//
	schema->setSchemaId(newSchemaId);
	schema->setGuid(QUuid::createUuid());

#ifdef _DEBUG
	std::vector<QUuid> oldGuids = schema->getGuids();
	std::set<QUuid> oldGuidsMap = {oldGuids.begin(), oldGuids.end()};
#endif
	for (std::shared_ptr<VFrame30::SchemaLayer> layer : schema->Layers)
	{
		layer->setGuid(QUuid::createUuid());

		for (std::shared_ptr<VFrame30::SchemaItem> item : layer->Items)
		{
			item->setNewGuid();

			if (item->isFblItemRect() == true)
			{
				globalCounter = db()->nextCounterValue();
				item->toFblItemRect()->setLabel(schema->schemaId() + "_" + QString::number(globalCounter));
			}
		}
	}

#ifdef _DEBUG
	// Check if all guids were updated
	//
	std::vector<QUuid> newGuids = schema->getGuids();
	for (const QUuid& guid : newGuids)
	{
		size_t c = oldGuidsMap.count(guid);
		assert(c == 0);
	}
#endif

	addSchemaFile(schema, true);
	return;
}

void SchemaControlTabPage::editSchemasProperties(std::vector<DbFileInfo> selectedFiles)
{
	bool readOnly = true;

	for (const DbFileInfo& file : selectedFiles)
	{
		if (file.state() == VcsState::CheckedOut &&
			(file.userId() == db()->currentUser().userId() || db()->currentUser().isAdminstrator() == true))
		{
			readOnly = false;
		}
	}

	// If schema is opened, can't edit its' properties
	//
	SchemasTabPage* parent = dynamic_cast<SchemasTabPage*>(this->parentWidget()->parentWidget()->parentWidget());
	if (parent == nullptr)
	{
		assert(parent);
		return;
	}

	std::vector<EditSchemaTabPage*> openedSchames = parent->getOpenSchemas();

	for (const DbFileInfo& file : selectedFiles)
	{
		auto foundTab = std::find_if(openedSchames.begin(), openedSchames.end(),
					[&file](const EditSchemaTabPage* tabPage)
					{
						assert(tabPage);
						return	tabPage->fileInfo().fileId() == file.fileId() &&
								tabPage->readOnly() == false;
					});

		if (foundTab != openedSchames.end())
		{
			EditSchemaTabPage* tab = *foundTab;
			QMessageBox::critical(this, qAppName(), tr("Can't edit %1 schema properties, as it is opened for edit. Close schema to edit it's properties.").arg(tab->schema()->schemaId()));
			return;
		}
	}

	// Load schemas
	//
	std::vector<std::shared_ptr<DbFile>> out;

	bool ok = db()->getLatestVersion(selectedFiles, & out, this);

	if (ok == false)
	{
		return;
	}

	// Read schemas
	//
	std::vector<std::pair<std::shared_ptr<DbFile>, std::shared_ptr<VFrame30::Schema>>> schemas;

	schemas.reserve(out.size());

	for (std::shared_ptr<DbFile> file : out)
	{
		std::shared_ptr<VFrame30::Schema> schema = VFrame30::Schema::Create(file->data());
		if (schema == nullptr)
		{
			assert(schema != nullptr);
			return;
		}

		schemas.push_back({file, schema});
	}

	// Show schema properties dialog
	//
	QDialog d(this);

	d.setWindowTitle(tr("Schema(s) Properties"));
	d.setWindowFlags((d.windowFlags() &
					~Qt::WindowMinimizeButtonHint &
					~Qt::WindowMaximizeButtonHint &
					~Qt::WindowContextHelpButtonHint) | Qt::CustomizeWindowHint);

	ExtWidgets::PropertyEditor* propertyEditor = new ExtWidgets::PropertyEditor(this);
	propertyEditor->setReadOnly(readOnly);

	std::vector<std::shared_ptr<PropertyObject>> propertyObjects;
	propertyObjects.reserve(schemas.size());

	for (std::pair<std::shared_ptr<DbFile>, std::shared_ptr<VFrame30::Schema>> s : schemas)
	{
		propertyObjects.push_back(s.second);
		assert(propertyObjects.back() != nullptr);
	}

	propertyEditor->setObjects(propertyObjects);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(propertyEditor);
	layout->addWidget(buttonBox);

	d.setLayout(layout);

	connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	d.resize(d.sizeHint() * 1.5);

	int result = d.exec();

	if (result == QDialog::Accepted)
	{
		std::vector<std::shared_ptr<DbFile>> filesToSave;
		filesToSave.reserve(schemas.size());

		for (std::pair<std::shared_ptr<DbFile>, std::shared_ptr<VFrame30::Schema>> s : schemas)
		{
			std::shared_ptr<DbFile> file = s.first;
			std::shared_ptr<VFrame30::Schema> schema = s.second;

			if (file->state() != VcsState::CheckedOut ||
				(file->userId() != db()->currentUser().userId() && db()->currentUser().isAdminstrator() == false))
			{
				continue;
			}

			QByteArray data;
			schema->Save(data);

			if (data.isEmpty() == true)
			{
				assert(data.isEmpty() == false);
				return;
			}

			file->swapData(data);

			QString detailsString = schema->details();
			file->setDetails(detailsString);

			filesToSave.push_back(file);
		}

		if (filesToSave.empty() == false)
		{
			db()->setWorkcopy(filesToSave, this);

			refreshFiles();
		}
	}

	return;
}

void SchemaControlTabPage::ctrlF()
{
	assert(m_searchEdit);
	m_searchEdit->setFocus();
	m_searchEdit->selectAll();

	return;
}

void SchemaControlTabPage::search()
{

	// Search for text in schemas
	//
	assert(m_filesView);
	assert(m_searchEdit);

	QString searchText = m_searchEdit->text().trimmed();

	if (searchText.isEmpty() == true)
	{
		m_filesView->clearSelection();
		return;
	}

	// --
	//
	const std::vector<std::shared_ptr<DbFileInfo>>& files = m_filesView->files();

	std::vector<std::shared_ptr<DbFileInfo>> foundFiles;
	foundFiles.reserve(files.size());

	for (std::shared_ptr<DbFileInfo> f : files)
	{
		if (f->fileName().contains(searchText, Qt::CaseInsensitive) == true)
		{
			foundFiles.push_back(f);
			continue;
		}

		// Parse details
		//
		VFrame30::SchemaDetails details;

		bool ok = details.parseDetails(f->details());
		if (ok == false)
		{
			continue;
		}

		bool searchResult = details.searchForString(searchText);
		if (searchResult == true)
		{
			foundFiles.push_back(f);
			continue;
		}
	}

	// Select found schemas
	//
	m_filesView->selectionModel()->clearSelection();

	QModelIndex firstModeleIndexToScroll;						// The first found ModelIndex will be kept here, to scroll to it (EnsureVisible)

	for (std::shared_ptr<DbFileInfo> f : foundFiles)
	{
		int fileRow = m_filesView->filesModel().getFileRow(f->fileId());
		assert(fileRow != -1);

		if (fileRow == -1)
		{
			continue;
		}

		QModelIndex md = m_filesView->filesModel().index(fileRow, 0);
		assert(md.isValid() == true);

		if (md.isValid() == true)
		{
			m_filesView->selectionModel()->select(md, QItemSelectionModel::Select | QItemSelectionModel::Rows);

			if (firstModeleIndexToScroll.isValid() == false)	// Save only first time
			{
				firstModeleIndexToScroll = md;
			}
		}
	}

	m_filesView->filesViewSelectionChanged(QItemSelection(), QItemSelection());

	if (firstModeleIndexToScroll.isValid() == true)
	{
		m_filesView->scrollTo(firstModeleIndexToScroll);
	}

	m_filesView->setFocus();

	return;
}

void SchemaControlTabPage::searchSchemaForLm(QString equipmentId)
{
	// Set focus to LogicSchemaTabPage and to ControlTabPage
	//
	QTabWidget* parentTabWidget = dynamic_cast<QTabWidget*>(this->parentWidget()->parentWidget());
	if (parentTabWidget == nullptr)
	{
		assert(parentTabWidget);
	}
	else
	{
		parentTabWidget->setCurrentWidget(this);
	}

	GlobalMessanger::instance()->fireChangeCurrentTab(this->parentWidget()->parentWidget()->parentWidget());

	m_filesView->setFocus();

	// Set Search string and perform search
	//
	m_searchEdit->setText(equipmentId.trimmed());
	search();

	return;
}

const DbFileInfo& SchemaControlTabPage::parentFile() const
{
	return m_filesView->parentFile();
}


//
//
// EditSchemaTabPage
//
//
EditSchemaTabPage::EditSchemaTabPage(QTabWidget* tabWidget, std::shared_ptr<VFrame30::Schema> schema, const DbFileInfo& fileInfo, DbController* dbcontroller) :
	HasDbController(dbcontroller),
	m_schemaWidget(nullptr),
	m_tabWidget(tabWidget)
{
	assert(m_tabWidget);
	assert(schema.get() != nullptr);

	setWindowTitle(schema->schemaId());

	// Create controls
	//
	schema->setChangeset(fileInfo.changeset());

	m_schemaWidget = new EditSchemaWidget(schema, fileInfo, dbcontroller);

	connect(m_schemaWidget, &EditSchemaWidget::closeTab, this, &EditSchemaTabPage::closeTab);
	connect(m_schemaWidget, &EditSchemaWidget::modifiedChanged, this, &EditSchemaTabPage::modifiedChanged);
	connect(m_schemaWidget, &EditSchemaWidget::saveWorkcopy, this, &EditSchemaTabPage::saveWorkcopy);
	connect(m_schemaWidget, &EditSchemaWidget::checkInFile, this, &EditSchemaTabPage::checkInFile);
	connect(m_schemaWidget, &EditSchemaWidget::checkOutFile, this, &EditSchemaTabPage::checkOutFile);
	connect(m_schemaWidget, &EditSchemaWidget::undoChangesFile, this, &EditSchemaTabPage::undoChangesFile);
	connect(m_schemaWidget, &EditSchemaWidget::getCurrentWorkcopy, this, &EditSchemaTabPage::getCurrentWorkcopy);
	connect(m_schemaWidget, &EditSchemaWidget::setCurrentWorkcopy, this, &EditSchemaTabPage::setCurrentWorkcopy);


	// ToolBar
	//
	m_toolBar = new QToolBar(this);
	m_toolBar->setOrientation(Qt::Vertical);

	m_toolBar->addAction(m_schemaWidget->m_fileAction);

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_schemaWidget->m_addLineAction);
	m_toolBar->addAction(m_schemaWidget->m_addRectAction);
	m_toolBar->addAction(m_schemaWidget->m_addPathAction);
	m_toolBar->addAction(m_schemaWidget->m_addTextAction);

	if (schema->isLogicSchema() == true)
	{
		m_toolBar->addSeparator();
		m_toolBar->addAction(m_schemaWidget->m_addLinkAction);
		m_toolBar->addAction(m_schemaWidget->m_addInputSignalAction);
		m_toolBar->addAction(m_schemaWidget->m_addInOutSignalAction);
		m_toolBar->addAction(m_schemaWidget->m_addOutputSignalAction);
		m_toolBar->addAction(m_schemaWidget->m_addConstantAction);
		m_toolBar->addAction(m_schemaWidget->m_addTerminatorAction);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorAfb);
		m_toolBar->addAction(m_schemaWidget->m_addAfbAction);
		m_toolBar->addAction(m_schemaWidget->m_addUfbAction);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorConn);
		m_toolBar->addAction(m_schemaWidget->m_addTransmitter);
		m_toolBar->addAction(m_schemaWidget->m_addReceiver);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorLoop);
		m_toolBar->addAction(m_schemaWidget->m_addLoopbackSource);
		m_toolBar->addAction(m_schemaWidget->m_addLoopbackTarget);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorBus);
		m_toolBar->addAction(m_schemaWidget->m_addBusComposer);
		m_toolBar->addAction(m_schemaWidget->m_addBusExtractor);
	}

	if (schema->isUfbSchema())
	{
		m_toolBar->addSeparator();
		m_toolBar->addAction(m_schemaWidget->m_addLinkAction);
		m_toolBar->addAction(m_schemaWidget->m_addInputSignalAction);
		m_toolBar->addAction(m_schemaWidget->m_addOutputSignalAction);
		m_toolBar->addAction(m_schemaWidget->m_addConstantAction);
		m_toolBar->addAction(m_schemaWidget->m_addTerminatorAction);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorAfb);
		m_toolBar->addAction(m_schemaWidget->m_addAfbAction);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorLoop);
		m_toolBar->addAction(m_schemaWidget->m_addLoopbackSource);
		m_toolBar->addAction(m_schemaWidget->m_addLoopbackTarget);

		m_toolBar->addAction(m_schemaWidget->m_addSeparatorBus);
		m_toolBar->addAction(m_schemaWidget->m_addBusComposer);
		m_toolBar->addAction(m_schemaWidget->m_addBusExtractor);
	}

	if (schema->isMonitorSchema())
	{
		m_toolBar->addSeparator();
		m_toolBar->addAction(m_schemaWidget->m_addValueAction);
		m_toolBar->addAction(m_schemaWidget->m_addPushButtonAction);
		m_toolBar->addAction(m_schemaWidget->m_addLineEditAction);
	}

	m_toolBar->addSeparator();
	m_toolBar->addAction(m_schemaWidget->m_orderAction);
	m_toolBar->addAction(m_schemaWidget->m_sizeAndPosAction);

	m_toolBar->addAction(m_schemaWidget->m_infoModeAction);

	// --
	//
	CreateActions();

	// --
	//
	QHBoxLayout* pMainLayout = new QHBoxLayout();

	pMainLayout->setContentsMargins(0, 5, 0, 5);
	pMainLayout->setSpacing(0);

	pMainLayout->addWidget(m_toolBar);
	pMainLayout->addWidget(m_schemaWidget);

	setLayout(pMainLayout);

	// --
	//
	connect(m_schemaWidget->m_fileAction, &QAction::triggered, this, &EditSchemaTabPage::fileMenuTriggered);
	connect(m_schemaWidget->m_orderAction, &QAction::triggered, this, &EditSchemaTabPage::itemsOrderTriggered);
	connect(m_schemaWidget->m_sizeAndPosAction, &QAction::triggered, this, &EditSchemaTabPage::sizeAndPosMenuTriggered);

	connect(m_tabWidget, &QTabWidget::currentChanged, m_schemaWidget, &EditSchemaWidget::hideWorkDialogs);

	return;
}

EditSchemaTabPage::~EditSchemaTabPage()
{
}

void EditSchemaTabPage::setPageTitle()
{
	QWidget* thisParent = parentWidget();
	if (thisParent == nullptr)
	{
		// This widget has not been created yet?
		//
		return;
	}

	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(thisParent->parentWidget());
	if (tabWidget == nullptr)
	{
		assert(tabWidget != nullptr);
		return;
	}

	QString newTitle;

	if (readOnly() == true || fileInfo().userId() != db()->currentUser().userId())
	{
		if (fileInfo().changeset() == -1 || fileInfo().changeset() == 0)
		{
			newTitle = QString("%1: ReadOnly").arg(m_schemaWidget->schema()->schemaId());
		}
		else
		{
			newTitle = QString("%1: %2 ReadOnly").arg(m_schemaWidget->schema()->schemaId()).arg(fileInfo().changeset());
		}

		if (fileInfo().deleted() == true)
		{
			newTitle += QString(", deleted");
		}
	}
	else
	{
		if (modified() == true)
		{
			newTitle = m_schemaWidget->schema()->schemaId() + "*";
		}
		else
		{
			newTitle = m_schemaWidget->schema()->schemaId();
		}
	}

	for (int i = 0; i < tabWidget->count(); i++)
	{
		if (tabWidget->widget(i) == this)
		{
			tabWidget->setTabText(i, newTitle);
			return;
		}
	}
}

void EditSchemaTabPage::updateAfbSchemaItems()
{
	if (m_schemaWidget == nullptr)
	{
		assert(m_schemaWidget);
		return;
	}

	m_schemaWidget->updateAfbsForSchema();

	return;
}

void EditSchemaTabPage::updateUfbSchemaItems()
{
	if (m_schemaWidget == nullptr)
	{
		assert(m_schemaWidget);
		return;
	}

	m_schemaWidget->updateUfbsForSchema();

	return;
}

void EditSchemaTabPage::updateBussesSchemaItems()
{
	if (m_schemaWidget == nullptr)
	{
		assert(m_schemaWidget);
		return;
	}

	m_schemaWidget->updateBussesForSchema();

	return;
}


void EditSchemaTabPage::CreateActions()
{
}

void EditSchemaTabPage::closeTab()
{
	if (m_schemaWidget->modified() == true)
	{
		QMessageBox mb(this);
		mb.setText(tr("The document has been modified."));
		mb.setInformativeText(tr("Do you want to save chages to %1?").arg(fileInfo().fileName()));
		mb.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		mb.setDefaultButton(QMessageBox::Save);

		int result = mb.exec();

		switch (result)
		{
		case QMessageBox::Save:
			saveWorkcopy();
			break;
		case QMessageBox::Discard:
			break;
		case QMessageBox::Cancel:
			return;
		}
	}

	// Find current tab and close it
	//
	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
	if (tabWidget == nullptr)
	{
		assert(tabWidget != nullptr);
		return;
	}

	for (int i = 0; i < tabWidget->count(); i++)
	{
		if (tabWidget->widget(i) == this)
		{
			tabWidget->removeTab(i);
		}
	}

	this->deleteLater();
	return;
}

void EditSchemaTabPage::modifiedChanged(bool /*modified*/)
{
	setPageTitle();
}

void EditSchemaTabPage::checkInFile()
{
	if (readOnly() == true ||
		fileInfo().state() != VcsState::CheckedOut ||
		(fileInfo().userId() != db()->currentUser().userId() && db()->currentUser().isAdminstrator() == false))
	{
		return;
	}

	// Save workcopy and checkin
	//
	if (modified() == true)
	{
		bool saveResult = saveWorkcopy();

		if (saveResult == false)
		{
			return;
		}
	}

	std::vector<DbFileInfo> files;
	files.push_back(fileInfo());

	std::vector<DbFileInfo> updatedFiles;

	bool checkInResult = CheckInDialog::checkIn(files, false, &updatedFiles, db(), this);
	if (checkInResult == false)
	{
		return;
	}

	emit vcsFileStateChanged();

	DbFileInfo fi;
	db()->getFileInfo(fileInfo().fileId(), &fi, this);

	setFileInfo(fi);

	setReadOnly(true);

	setPageTitle();

	return;
}

void EditSchemaTabPage::checkOutFile()
{
	if (readOnly() == false ||
		fileInfo().state() != VcsState::CheckedIn)
	{
		return;
	}

	std::vector<DbFileInfo> files;
	files.push_back(fileInfo());

	bool result = db()->checkOut(files, this);
	if (result == false)
	{
		return;
	}

	// Read the workcopy and load it to the current document
	//
	std::vector<std::shared_ptr<DbFile>> out;

	result = db()->getWorkcopy(files, &out, this);
	if (result == false || out.size() != files.size())
	{
		return;
	}

	m_schemaWidget->schema()->Load(out[0].get()->data());

	setFileInfo(*(out.front().get()));

	setReadOnly(false);
	setPageTitle();

	m_schemaWidget->resetAction();
	m_schemaWidget->clearSelection();

	m_schemaWidget->update();

	emit vcsFileStateChanged();
	return;
}

void EditSchemaTabPage::undoChangesFile()
{
	// 1 Ask user to confirm operation
	// 2 Undo changes to database
	// 3 Set frame to readonly mode
	//
	if (readOnly() == true ||
		fileInfo().state() != VcsState::CheckedOut ||
		fileInfo().userId() != db()->currentUser().userId())
	{
		assert(fileInfo().userId() == db()->currentUser().userId());
		return;
	}

	QMessageBox mb(this);
	mb.setText(tr("This operation will undo all pending changes for the document and will revert it to the prior state!"));
	mb.setInformativeText(tr("Do you want to undo pending changes?"));
	mb.setIcon(QMessageBox::Question);
	mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

	if (mb.exec() == QMessageBox::Ok)
	{
		DbFileInfo fi = fileInfo();

		bool result = db()->undoChanges(fi, this);

		if (result == true)
		{
			setFileInfo(fi);

			setReadOnly(true);
			setPageTitle();

			m_schemaWidget->resetAction();
			m_schemaWidget->clearSelection();

			m_schemaWidget->update();
		}
	}

	emit vcsFileStateChanged();
	return;
}

void EditSchemaTabPage::fileMenuTriggered()
{
	if (m_toolBar == nullptr)
	{
		assert(m_toolBar);
		return;
	}

	QWidget* w = m_toolBar->widgetForAction(m_schemaWidget->m_fileAction);

	if (w == nullptr)
	{
		assert(w);
		return;
	}

	QPoint pt = w->pos();
	pt.rx() += w->width();

	m_schemaWidget->m_fileMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

void EditSchemaTabPage::sizeAndPosMenuTriggered()
{
	if (m_toolBar == nullptr)
	{
		assert(m_toolBar);
		return;
	}

	QWidget* w = m_toolBar->widgetForAction(m_schemaWidget->m_sizeAndPosAction);

	if (w == nullptr)
	{
		assert(w);
		return;
	}

	QPoint pt = w->pos();
	pt.rx() += w->width();

	m_schemaWidget->m_sizeAndPosMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

void EditSchemaTabPage::itemsOrderTriggered()
{
	if (m_toolBar == nullptr)
	{
		assert(m_toolBar);
		return;
	}

	QWidget* w = m_toolBar->widgetForAction(m_schemaWidget->m_orderAction);

	if (w == nullptr)
	{
		assert(w);
		return;
	}

	QPoint pt = w->pos();
	pt.rx() += w->width();

	m_schemaWidget->m_orderMenu->popup(m_toolBar->mapToGlobal(pt));

	return;
}

bool EditSchemaTabPage::saveWorkcopy()
{
	if (readOnly() == true ||
		modified() == false ||
		fileInfo().state() != VcsState::CheckedOut ||
		fileInfo().userId() != db()->currentUser().userId())
	{
		assert(fileInfo().userId() == db()->currentUser().userId());
		return false;
	}

	QByteArray data;
	m_schemaWidget->schema()->Save(data);

	if (data.isEmpty() == true)
	{
		assert(data.isEmpty() == false);
		return false;
	}

	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
	static_cast<DbFileInfo*>(file.get())->operator=(fileInfo());
	file->swapData(data);

	QString detailsString = m_schemaWidget->schema()->details();
	file->setDetails(detailsString);

	bool result = db()->setWorkcopy(file, this);
	if (result == true)
	{
		resetModified();
		return true;
	}

	return false;
}

void EditSchemaTabPage::getCurrentWorkcopy()
{
	// Select destination folder
	//
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty() == true)
	{
		return;
	}

	if (dir[dir.length() - 1] != '/')
	{
		dir.append("/");
	}

	// Save files to disk
	//
	QString fileName = dir + fileInfo().fileName();

	bool writeResult = m_schemaWidget->schema()->Save(fileName);

	if (writeResult == false)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Write file error."));
		msgBox.setInformativeText(tr("Cannot write file %1.").arg(fileInfo().fileName()));
		msgBox.exec();
	}

	return;
}

void EditSchemaTabPage::setCurrentWorkcopy()
{
	if (readOnly() == true ||
		fileInfo().state() != VcsState::CheckedOut ||
		(fileInfo().userId() != db()->currentUser().userId() && db()->currentUser().isAdminstrator() == false))
	{
		assert(fileInfo().userId() == db()->currentUser().userId());
		return;
	}

	// Select file
	//
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"));
	if (fileName.isEmpty() == true)
	{
		return;
	}

	// Load file
	//
	bool readResult = m_schemaWidget->schema()->Load(fileName);
	if (readResult == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Can't read file %1.").arg(fileName));
		mb.exec();
		return;
	}

	// --
	setPageTitle();

	m_schemaWidget->resetAction();
	m_schemaWidget->clearSelection();

	m_schemaWidget->resetEditEngine();
	m_schemaWidget->setModified();

	m_schemaWidget->update();

	return;
}

std::shared_ptr<VFrame30::Schema> EditSchemaTabPage::schema()
{
	assert(m_schemaWidget);
	std::shared_ptr<VFrame30::Schema> s = m_schemaWidget->schema();
	return s;
}

const DbFileInfo& EditSchemaTabPage::fileInfo() const
{
	assert(m_schemaWidget);
	return m_schemaWidget->fileInfo();
}

void EditSchemaTabPage::setFileInfo(const DbFileInfo& fi)
{
	assert(m_schemaWidget);
	m_schemaWidget->setFileInfo(fi);

	m_schemaWidget->schema()->setChangeset(fi.changeset());

	setPageTitle();
}

bool EditSchemaTabPage::readOnly() const
{
	assert(m_schemaWidget);
	return m_schemaWidget->readOnly();
}

void EditSchemaTabPage::setReadOnly(bool value)
{
	assert(m_schemaWidget);
	m_schemaWidget->setReadOnly(value);
}

bool EditSchemaTabPage::modified() const
{
	assert(m_schemaWidget);
	return m_schemaWidget->modified();
}

void EditSchemaTabPage::resetModified()
{
	assert(m_schemaWidget);
	return m_schemaWidget->resetModified();
}

bool EditSchemaTabPage::compareWidget() const
{
	return m_schemaWidget->compareWidget();
}

bool EditSchemaTabPage::isCompareWidget() const
{
	return m_schemaWidget->compareWidget();
}

void EditSchemaTabPage::setCompareWidget(bool value, std::shared_ptr<VFrame30::Schema> source, std::shared_ptr<VFrame30::Schema> target)
{
	return m_schemaWidget->setCompareWidget(value, source, target);
}

void EditSchemaTabPage::setCompareItemActions(const std::map<QUuid, CompareAction>& itemsActions)
{
	m_schemaWidget->setCompareItemActions(itemsActions);
}
