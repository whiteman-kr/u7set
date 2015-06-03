#include "Stable.h"
#include "SchemeTabPage.h"
#include "VideoFramePropertiesDialog.h"
#include "ChangesetDialog.h"
#include "CheckInDialog.h"

//
//
//	VideoFrameFileView
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

void SchemeFileView::deleteFile(std::vector<DbFileInfo> files)
{
	emit deleteFileSignal(files);
}



//
//
// EditVideoFrameTabPage
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
// VideoFrameControlTabPage
//
//

SchemeControlTabPage::SchemeControlTabPage(const QString& fileExt,
		DbController* dbcontroller, const QString& parentFileName,
		std::function<VFrame30::Scheme*()> createVideoFrameFunc) :

	HasDbController(dbcontroller),
	m_createVideoFrameFunc(createVideoFrameFunc)
{
	// Create actions
	//
	CreateActions();

	// Create controls
	//
	m_filesView = new SchemeFileView(dbcontroller, parentFileName);
	m_filesView->filesModel().setFilter(fileExt);

	QHBoxLayout* pMainLayout = new QHBoxLayout();
	pMainLayout->addWidget(m_filesView);

	setLayout(pMainLayout);

	// --
	//
	//connect(dbcontroller, &DbController::projectOpened, this, &VideoFrameControlTabPage::projectOpened);
	//connect(dbcontroller, &DbController::projectClosed, this, &VideoFrameControlTabPage::projectClosed);

	connect(m_filesView, &SchemeFileView::openFileSignal, this, &SchemeControlTabPage::openFiles);
	connect(m_filesView, &SchemeFileView::viewFileSignal, this, &SchemeControlTabPage::viewFiles);
	connect(m_filesView, &SchemeFileView::addFileSignal, this, &SchemeControlTabPage::addFile);
	connect(m_filesView, &SchemeFileView::deleteFileSignal, this, &SchemeControlTabPage::deleteFile);
	connect(m_filesView, &SchemeFileView::checkInSignal, this, &SchemeControlTabPage::checkIn);

	return;
}

SchemeControlTabPage::~SchemeControlTabPage()
{
}

VFrame30::Scheme* SchemeControlTabPage::createVideoFrame() const
{
	return m_createVideoFrameFunc();
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

	// Create new videoframe and add it to the vcs
	//
	std::shared_ptr<VFrame30::Scheme> vf(m_createVideoFrameFunc());

	vf->setGuid(QUuid::createUuid());

	vf->setStrID("STRID");
	vf->setCaption("Caption");

	vf->setDocWidth(vf->unit() == VFrame30::SchemeUnit::Display ? 1280 : (420 / 25.4));
	vf->setDocHeight(vf->unit() == VFrame30::SchemeUnit::Display ? 1024 : (297 / 25.4));

	VideoFramePropertiesDialog propertiesDialog(vf, this);
	if (propertiesDialog.exec() != QDialog::Accepted)
	{
		return;
	}

	QByteArray data;
	vf->Save(data);

	std::shared_ptr<DbFile> vfFile = std::make_shared<DbFile>();
	vfFile->setFileName(fileName);
	vfFile->swapData(data);

	std::vector<std::shared_ptr<DbFile>> addFilesList;
	addFilesList.push_back(vfFile);

	dbcontroller()->addFiles(&addFilesList, parentFile().fileId(), this);

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

	std::vector<std::shared_ptr<DbFileInfo>> v;

	for(const auto& f : files)
	{
		v.push_back(std::make_shared<DbFileInfo>(f));
	}

	dbcontroller()->deleteFiles(&v, this);

	refreshFiles();

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
	bool ok = CheckInDialog::checkIn(files, dbcontroller(), this);
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

	dbcontroller()->getFileInfo(&fileIds, &files, this);

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
		file.userId() != dbcontroller()->currentUser().userId())
	{
		QMessageBox mb(this);
		mb.setText(tr("File %1 already checked out by user %2.").arg(file.fileName()).arg(file.userId()));
		mb.exec();
		return;
	}

	assert(file.state() == VcsState::CheckedOut && file.userId() == dbcontroller()->currentUser().userId());

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

	bool result = dbcontroller()->getWorkcopy(files, &out, this);
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

	EditSchemeTabPage* editTabPage = new EditSchemeTabPage(vf, fi, dbcontroller());

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

	dbcontroller()->getFileHistory(file, &fileHistory, this);

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

	bool result = dbcontroller()->getSpecificCopy(files, changesetId, &out, this);
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

	EditSchemeTabPage* editTabPage = new EditSchemeTabPage(vf, fi, dbcontroller());
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
// EditVideoFrameTabPage
//
//
EditSchemeTabPage::EditSchemeTabPage(std::shared_ptr<VFrame30::Scheme> videoFrame, const DbFileInfo& fileInfo, DbController* dbcontroller) :
	HasDbController(dbcontroller),
	m_videoFrameWidget(nullptr)
{
	assert(videoFrame.get() != nullptr);

	setWindowTitle(videoFrame->strID());

	CreateActions();

	// Create controls
	//
	m_videoFrameWidget = new EditSchemeWidget(videoFrame, fileInfo, dbcontroller);

	connect(m_videoFrameWidget, &EditSchemeWidget::closeTab, this, &EditSchemeTabPage::closeTab);
	connect(m_videoFrameWidget, &EditSchemeWidget::modifiedChanged, this, &EditSchemeTabPage::modifiedChanged);
	connect(m_videoFrameWidget, &EditSchemeWidget::saveWorkcopy, this, &EditSchemeTabPage::saveWorkcopy);
	connect(m_videoFrameWidget, &EditSchemeWidget::checkInFile, this, &EditSchemeTabPage::checkInFile);
	connect(m_videoFrameWidget, &EditSchemeWidget::checkOutFile, this, &EditSchemeTabPage::checkOutFile);
	connect(m_videoFrameWidget, &EditSchemeWidget::undoChangesFile, this, &EditSchemeTabPage::undoChangesFile);
	connect(m_videoFrameWidget, &EditSchemeWidget::getCurrentWorkcopy, this, &EditSchemeTabPage::getCurrentWorkcopy);
	connect(m_videoFrameWidget, &EditSchemeWidget::setCurrentWorkcopy, this, &EditSchemeTabPage::setCurrentWorkcopy);

	QHBoxLayout* pMainLayout = new QHBoxLayout();
	pMainLayout->addWidget(m_videoFrameWidget);

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

	if (readOnly() == true || fileInfo().userId() != dbcontroller()->currentUser().userId())
	{
		if (fileInfo().changeset() != -1)
		{
			newTitle = QString("%1: %2 ReadOnly").arg(m_videoFrameWidget->scheme()->strID()).arg(fileInfo().changeset());
		}
		else
		{
			newTitle = QString("%1: ReadOnly").arg(m_videoFrameWidget->scheme()->strID());
		}
	}
	else
	{
		if (modified() == true)
		{
			newTitle = m_videoFrameWidget->scheme()->strID() + "*";
		}
		else
		{
			newTitle = m_videoFrameWidget->scheme()->strID();
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
	if (m_videoFrameWidget->modified() == true)
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
		(fileInfo().userId() != dbcontroller()->currentUser().userId() && dbcontroller()->currentUser().isAdminstrator() == false))
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

	bool checkInResult = CheckInDialog::checkIn(files, dbcontroller(), this);
	if (checkInResult == false)
	{
		return;
	}

	emit vcsFileStateChanged();

	DbFileInfo fi;
	dbcontroller()->getFileInfo(fileInfo().fileId(), &fi, this);

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

	bool result = dbcontroller()->checkOut(files, this);
	if (result == false)
	{
		return;
	}

	// Read the workcopy and load it to the current document
	//
	std::vector<std::shared_ptr<DbFile>> out;

	result = dbcontroller()->getWorkcopy(files, &out, this);
	if (result == false || out.size() != files.size())
	{
		return;
	}

	m_videoFrameWidget->scheme()->Load(out[0].get()->data());

	setFileInfo(*(out.front().get()));

	setReadOnly(false);
	setPageTitle();

	m_videoFrameWidget->resetAction();
	m_videoFrameWidget->clearSelection();

	m_videoFrameWidget->update();

	emit vcsFileStateChanged();
	return;
}

void EditSchemeTabPage::undoChangesFile()
{
	// 1 Ask user to confirm operation
	// 2 Undo changes to database
	// 3 Set frame to readonly mode
	//

	assert(false);

	/*
	if (readOnly() == true ||
		fileInfo().state() != VcsState::CheckedOut ||
		fileInfo().user() != dbcontroller()->currentUser())
	{
		assert(fileInfo().user() == dbcontroller()->currentUser());
		return;
	}

	QMessageBox mb(this);
	mb.setText(tr("This operation will undo all pending changes for the document and will revert it to the prior state!"));
	mb.setInformativeText(tr("Do you want to undo pending changes?"));
	mb.setIcon(QMessageBox::Question);
	mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

	if (mb.exec() == QMessageBox::Ok)
	{
		std::vector<DbFileInfo> files;
		files.push_back(fileInfo());

		bool result = dbcontroller()->undoChanges(files, this);

		if (result == true)
		{
			DbFileInfo fi;
			dbcontroller()->getFileInfo(fileInfo().fileId(), &fi);

			setFileInfo(fi);

			setReadOnly(true);
			setPageTitle();

			m_videoFrameWidget->resetAction();
			m_videoFrameWidget->clearSelection();

			m_videoFrameWidget->update();
		}
	}

	emit vcsFileStateChanged();
	return;*/
}

bool EditSchemeTabPage::saveWorkcopy()
{
	if (readOnly() == true ||
		modified() == false ||
		fileInfo().state() != VcsState::CheckedOut ||
		fileInfo().userId() != dbcontroller()->currentUser().userId())
	{
		assert(fileInfo().userId() == dbcontroller()->currentUser().userId());
		return false;
	}

	QByteArray data;
	m_videoFrameWidget->scheme()->Save(data);

	if (data.isEmpty() == true)
	{
		assert(data.isEmpty() == false);
		return false;
	}

	std::shared_ptr<DbFile> file = std::make_shared<DbFile>();
	static_cast<DbFileInfo*>(file.get())->operator=(fileInfo());
	file->swapData(data);

	bool result = dbcontroller()->setWorkcopy(file, this);
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

	bool writeResult = m_videoFrameWidget->scheme()->Save(fileName);

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
		(fileInfo().userId() != dbcontroller()->currentUser().userId() && dbcontroller()->currentUser().isAdminstrator() == false))
	{
		assert(fileInfo().userId() == dbcontroller()->currentUser().userId());
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
	bool readResult = m_videoFrameWidget->scheme()->Load(fileName);
	if (readResult == false)
	{
		QMessageBox mb(this);
		mb.setText(tr("Can't read file %1.").arg(fileName));
		mb.exec();
		return;
	}

	// --
	setPageTitle();

	m_videoFrameWidget->resetAction();
	m_videoFrameWidget->clearSelection();

	m_videoFrameWidget->resetEditEngine();
	m_videoFrameWidget->setModified();

	m_videoFrameWidget->update();

	return;
}

const DbFileInfo& EditSchemeTabPage::fileInfo() const
{
	assert(m_videoFrameWidget);
	return m_videoFrameWidget->fileInfo();
}

void EditSchemeTabPage::setFileInfo(const DbFileInfo& fi)
{
	assert(m_videoFrameWidget);
	m_videoFrameWidget->setFileInfo(fi);

	setPageTitle();
}

bool EditSchemeTabPage::readOnly() const
{
	assert(m_videoFrameWidget);
	return m_videoFrameWidget->readOnly();
}

void EditSchemeTabPage::setReadOnly(bool value)
{
	assert(m_videoFrameWidget);
	m_videoFrameWidget->setReadOnly(value);
}

bool EditSchemeTabPage::modified() const
{
	assert(m_videoFrameWidget);
	return m_videoFrameWidget->modified();
}

void EditSchemeTabPage::resetModified()
{
	assert(m_videoFrameWidget);
	return m_videoFrameWidget->resetModified();
}
