#include "Stable.h"
#include "SchemeTabPage.h"
#include "CreateSchemeDialog.h"
#include "ChangesetDialog.h"
#include "CheckInDialog.h"

//
//
//	SchemeFileView
//
//
SchemeFileView::SchemeFileView(DbController* dbcontroller, const QString& parentFileName) :
	FileListView(dbcontroller, parentFileName)
{
	filesModel().setFilter("vfr");
	return;
}

SchemeFileView::~SchemeFileView()
{
}

void SchemeFileView::openFile(std::vector<DbFileInfo> files)
{
	emit openFileSignal(files);
	return;
}

void SchemeFileView::viewFile(std::vector<DbFileInfo> files)
{
	emit viewFileSignal(files);
	return;
}

void SchemeFileView::addFile()
{
	emit addFileSignal();
}

void SchemeFileView::checkIn(std::vector<DbFileInfo> files)
{
	emit checkInSignal(files);
}

void SchemeFileView::undoChanges(std::vector<DbFileInfo> files)
{
	emit undoChangesSignal(files);
}

void SchemeFileView::deleteFile(std::vector<DbFileInfo> files)
{
	emit deleteFileSignal(files);
}

void SchemeFileView::fileDoubleClicked(DbFileInfo file)
{
	std::vector<DbFileInfo> v;
	v.push_back(file);

	if (file.state() == VcsState::CheckedOut)
	{
		openFile(v);
	}
	else
	{
		viewFile(v);
	}

	return;
}



//
//
// SchemesTabPage
//
//
SchemesTabPage::SchemesTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	m_tabWidget = new QTabWidget();
	m_tabWidget->setMovable(true);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(m_tabWidget);

	setLayout(layout);

	// --
	//
	connect(dbController(), &DbController::projectOpened, this, &SchemesTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &SchemesTabPage::projectClosed);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

SchemesTabPage::~SchemesTabPage()
{
}

void SchemesTabPage::projectOpened()
{
	this->setEnabled(true);
}

void SchemesTabPage::projectClosed()
{
	// Close all opened documents
	//
	assert(m_tabWidget);

	QWidget* controlTab = nullptr;
	std::list<QWidget*> tabsToDelete;

	for (int i = 0; i < m_tabWidget->count(); i++)
	{
		QWidget* tabPage = m_tabWidget->widget(i);

		if (dynamic_cast<SchemeControlTabPage*>(tabPage) != nullptr)
		{
			controlTab = tabPage;
		}
		else
		{
			tabsToDelete.push_back(tabPage);
		}
	}

	m_tabWidget->clear();

	m_tabWidget->addTab(controlTab, tr("Control"));

	for (auto widget : tabsToDelete)
	{
		delete widget;
	}

	this->setEnabled(false);
}


//
//
// SchemeControlTabPage
//
//



SchemeControlTabPage::~SchemeControlTabPage()
{
}

VFrame30::Scheme* SchemeControlTabPage::createScheme() const
{
	return m_createSchemeFunc();
}

void SchemeControlTabPage::CreateActions()
{
}

void SchemeControlTabPage::addFile()
{
	// Choose Configuration file name
	//
	bool ok = false;

	QString fileName = QInputDialog::getText(this, tr("Choose file name"), tr("file name:"), QLineEdit::Normal, "filename", &ok);
	if (ok == false)
	{
		return;
	}

	if (fileName.isEmpty() == true)
	{
		QMessageBox msg(this);
		msg.setText(tr("File name must not be empty."));
		msg.exec();
		return;
	}

	// Add extension if required
	//
	if (fileName.endsWith("." + m_filesView->filesModel().filter()) == false)	// ".vfr"
	{
		fileName += "." + m_filesView->filesModel().filter();
	}

	// Create new Scheme and add it to the vcs
	//
	std::shared_ptr<VFrame30::Scheme> scheme(m_createSchemeFunc());

	scheme->setGuid(QUuid::createUuid());

	scheme->setStrID("#STRID");
	scheme->setCaption("Caption");

	if (scheme->unit() == VFrame30::SchemeUnit::Display)
	{
		scheme->setDocWidth(1280);
		scheme->setDocHeight(1024);
	}
	else
	{
		// A3 Landscape
		//
		scheme->setDocWidth(420.0 / 25.4);
		scheme->setDocHeight(297.0 / 25.4);
	}

	CreateSchemeDialog propertiesDialog(scheme, this);
	if (propertiesDialog.exec() != QDialog::Accepted)
	{
		return;
	}

	QByteArray data;
	scheme->Save(data);

	std::shared_ptr<DbFile> vfFile = std::make_shared<DbFile>();
	vfFile->setFileName(fileName);
	vfFile->swapData(data);

	std::vector<std::shared_ptr<DbFile>> addFilesList;
	addFilesList.push_back(vfFile);

	db()->addFiles(&addFilesList, parentFile().fileId(), this);

	// Add file to the FileModel and select them
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
		}
	}

	m_filesView->filesViewSelectionChanged(QItemSelection(), QItemSelection());
	return;
}


void SchemeControlTabPage::deleteFile(std::vector<DbFileInfo> files)
{
	if (files.empty() == true)
	{
		assert(files.empty() == false);
		return;
	}

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
		EditSchemeTabPage* tb = dynamic_cast<EditSchemeTabPage*>(tabWidget->widget(i));
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

void SchemeControlTabPage::checkIn(std::vector<DbFileInfo> files)
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
		EditSchemeTabPage* tb = dynamic_cast<EditSchemeTabPage*>(tabWidget->widget(i));
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
		EditSchemeTabPage* tb = dynamic_cast<EditSchemeTabPage*>(tabWidget->widget(i));
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

void SchemeControlTabPage::undoChanges(std::vector<DbFileInfo> files)
{
	// 1 Ask user to confirm operation
	// 2 Undo changes to database
	// 3 Set frame to readonly mode
	//

	std::vector<DbFileInfo> undoFiles;

	for (const DbFileInfo& fi : files)
	{
		if (fi.state() == VcsState::CheckedOut &&
			fi.userId() == db()->currentUser().userId())
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
		EditSchemeTabPage* tb = dynamic_cast<EditSchemeTabPage*>(tabWidget->widget(i));
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

void SchemeControlTabPage::openFiles(std::vector<DbFileInfo> files)
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
		mb.setText(tr("File %1 already checked out by user %2.").arg(file.fileName()).arg(file.userId()));
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
		EditSchemeTabPage* tb = dynamic_cast<EditSchemeTabPage*>(tabWidget->widget(i));
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
	std::shared_ptr<VFrame30::Scheme> vf(VFrame30::Scheme::Create(out[0].get()->data()));

	if (vf == nullptr)
	{
		assert(vf != nullptr);
		return;
	}

	// Create TabPage and add it to the TabControl
	//
	DbFileInfo fi(*(out.front().get()));

	EditSchemeTabPage* editTabPage = new EditSchemeTabPage(vf, fi, db());

	connect(editTabPage, &EditSchemeTabPage::vcsFileStateChanged, this, &SchemeControlTabPage::refreshFiles);

	assert(tabWidget->parent());

	SchemesTabPage* schemesTabPage = dynamic_cast<SchemesTabPage*>(tabWidget->parent());
	if (schemesTabPage == nullptr)
	{
		assert(dynamic_cast<SchemesTabPage*>(tabWidget->parent()));
		return;
	}

	connect(schemesTabPage, &SchemesTabPage::buildStarted, editTabPage, &EditSchemeTabPage::saveWorkcopy);

	// --
	//
	editTabPage->setReadOnly(false);

	tabWidget->addTab(editTabPage, editTabPage->windowTitle());
	tabWidget->setCurrentWidget(editTabPage);

	return;
}

void SchemeControlTabPage::viewFiles(std::vector<DbFileInfo> files)
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

	// Get file history
	//
	std::vector<DbChangesetInfo> fileHistory;

	db()->getFileHistory(file, &fileHistory, this);

	// Show chageset dialog
	//
	int changesetId = ChangesetDialog::getChangeset(fileHistory, this);

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
	std::shared_ptr<VFrame30::Scheme> vf(VFrame30::Scheme::Create(out[0].get()->data()));

	QString tabPageTitle;
	tabPageTitle = QString("%1: %2 ReadOnly").arg(vf->strID()).arg(changesetId);

	// Find the opened file,
	//
	for (int i = 0; i < tabWidget->count(); i++)
	{
		EditSchemeTabPage* tb = dynamic_cast<EditSchemeTabPage*>(tabWidget->widget(i));
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

	EditSchemeTabPage* editTabPage = new EditSchemeTabPage(vf, fi, db());
	editTabPage->setReadOnly(true);

	tabWidget->addTab(editTabPage, tabPageTitle);
	tabWidget->setCurrentWidget(editTabPage);

	return;
}

void SchemeControlTabPage::refreshFiles()
{
	assert(m_filesView);
	m_filesView->refreshFiles();
	return;
}

const DbFileInfo& SchemeControlTabPage::parentFile() const
{
	return m_filesView->parentFile();
}


//
//
// EditSchemeTabPage
//
//
EditSchemeTabPage::EditSchemeTabPage(std::shared_ptr<VFrame30::Scheme> scheme, const DbFileInfo& fileInfo, DbController* dbcontroller) :
	HasDbController(dbcontroller),
	m_schemeWidget(nullptr)
{
	assert(scheme.get() != nullptr);

	setWindowTitle(scheme->strID());

	CreateActions();

	// Create controls
	//
	m_schemeWidget = new EditSchemeWidget(scheme, fileInfo, dbcontroller);

	connect(m_schemeWidget, &EditSchemeWidget::closeTab, this, &EditSchemeTabPage::closeTab);
	connect(m_schemeWidget, &EditSchemeWidget::modifiedChanged, this, &EditSchemeTabPage::modifiedChanged);
	connect(m_schemeWidget, &EditSchemeWidget::saveWorkcopy, this, &EditSchemeTabPage::saveWorkcopy);
	connect(m_schemeWidget, &EditSchemeWidget::checkInFile, this, &EditSchemeTabPage::checkInFile);
	connect(m_schemeWidget, &EditSchemeWidget::checkOutFile, this, &EditSchemeTabPage::checkOutFile);
	connect(m_schemeWidget, &EditSchemeWidget::undoChangesFile, this, &EditSchemeTabPage::undoChangesFile);
	connect(m_schemeWidget, &EditSchemeWidget::getCurrentWorkcopy, this, &EditSchemeTabPage::getCurrentWorkcopy);
	connect(m_schemeWidget, &EditSchemeWidget::setCurrentWorkcopy, this, &EditSchemeTabPage::setCurrentWorkcopy);

	QHBoxLayout* pMainLayout = new QHBoxLayout();
	pMainLayout->addWidget(m_schemeWidget);

	setLayout(pMainLayout);

	return;
}

EditSchemeTabPage::~EditSchemeTabPage()
{
}

void EditSchemeTabPage::setPageTitle()
{
	QTabWidget* tabWidget = dynamic_cast<QTabWidget*>(parentWidget()->parentWidget());
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
			newTitle = QString("%1: ReadOnly").arg(m_schemeWidget->scheme()->strID());
		}
		else
		{
			newTitle = QString("%1: %2 ReadOnly").arg(m_schemeWidget->scheme()->strID()).arg(fileInfo().changeset());
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
			newTitle = m_schemeWidget->scheme()->strID() + "*";
		}
		else
		{
			newTitle = m_schemeWidget->scheme()->strID();
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

void EditSchemeTabPage::CreateActions()
{
}

void EditSchemeTabPage::closeTab()
{
	if (m_schemeWidget->modified() == true)
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
			assert(false);			// to do
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

void EditSchemeTabPage::modifiedChanged(bool /*modified*/)
{
	setPageTitle();
}

void EditSchemeTabPage::checkInFile()
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

void EditSchemeTabPage::checkOutFile()
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

	m_schemeWidget->scheme()->Load(out[0].get()->data());

	setFileInfo(*(out.front().get()));

	setReadOnly(false);
	setPageTitle();

	m_schemeWidget->resetAction();
	m_schemeWidget->clearSelection();

	m_schemeWidget->update();

	emit vcsFileStateChanged();
	return;
}

void EditSchemeTabPage::undoChangesFile()
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

			m_schemeWidget->resetAction();
			m_schemeWidget->clearSelection();

			m_schemeWidget->update();
		}
	}

	emit vcsFileStateChanged();
	return;
}

bool EditSchemeTabPage::saveWorkcopy()
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
	m_schemeWidget->scheme()->Save(data);

	if (data.isEmpty() == true)
	{
		assert(data.isEmpty() == false);
		return false;
	}

	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
	static_cast<DbFileInfo*>(file.get())->operator=(fileInfo());
	file->swapData(data);

	bool result = db()->setWorkcopy(file, this);
	if (result == true)
	{
		resetModified();
		return true;
	}

	return false;
}

void EditSchemeTabPage::getCurrentWorkcopy()
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

	bool writeResult = m_schemeWidget->scheme()->Save(fileName);

	if (writeResult == false)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Write file error."));
		msgBox.setInformativeText(tr("Cannot write file %1.").arg(fileInfo().fileName()));
		msgBox.exec();
	}

	return;
}

void EditSchemeTabPage::setCurrentWorkcopy()
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
	bool readResult = m_schemeWidget->scheme()->Load(fileName);
	if (readResult == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Can't read file %1.").arg(fileName));
		mb.exec();
		return;
	}

	// --
	setPageTitle();

	m_schemeWidget->resetAction();
	m_schemeWidget->clearSelection();

	m_schemeWidget->resetEditEngine();
	m_schemeWidget->setModified();

	m_schemeWidget->update();

	return;
}

const DbFileInfo& EditSchemeTabPage::fileInfo() const
{
	assert(m_schemeWidget);
	return m_schemeWidget->fileInfo();
}

void EditSchemeTabPage::setFileInfo(const DbFileInfo& fi)
{
	assert(m_schemeWidget);
	m_schemeWidget->setFileInfo(fi);

	setPageTitle();
}

bool EditSchemeTabPage::readOnly() const
{
	assert(m_schemeWidget);
	return m_schemeWidget->readOnly();
}

void EditSchemeTabPage::setReadOnly(bool value)
{
	assert(m_schemeWidget);
	m_schemeWidget->setReadOnly(value);
}

bool EditSchemeTabPage::modified() const
{
	assert(m_schemeWidget);
	return m_schemeWidget->modified();
}

void EditSchemeTabPage::resetModified()
{
	assert(m_schemeWidget);
	return m_schemeWidget->resetModified();
}
