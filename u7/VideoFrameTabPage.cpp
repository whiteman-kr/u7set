#include "Stable.h"
#include "VideoFrameTabPage.h"
#include "VideoFramePropertiesDialog.h"
#include "ChangesetDialog.h"
#include "CheckInDialog.h"

//
//
//	VideoFrameFileView
//
//
VideoFrameFileView::VideoFrameFileView(DbController* dbcontroller, const QString& parentFileName) :
	FileView(dbcontroller, parentFileName)
{
	filesModel().setFilter("vfr");
	return;
}

VideoFrameFileView::~VideoFrameFileView()
{
}

void VideoFrameFileView::openFile(std::vector<DbFileInfo> files)
{
	emit openFileSignal(files);
	return;
}

void VideoFrameFileView::viewFile(std::vector<DbFileInfo> files)
{
	emit viewFileSignal(files);
	return;
}

void VideoFrameFileView::addFile()
{
	emit addFileSignal();
}


//
//
// EditVideoFrameTabPage
//
//
VideoFrameTabPage::VideoFrameTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
	m_tabWidget = new QTabWidget();
	m_tabWidget->setMovable(true);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(m_tabWidget);

	setLayout(layout);

	// --
	//
	connect(dbController(), &DbController::projectOpened, this, &VideoFrameTabPage::projectOpened);
	connect(dbController(), &DbController::projectClosed, this, &VideoFrameTabPage::projectClosed);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);
}

VideoFrameTabPage::~VideoFrameTabPage()
{
}

void VideoFrameTabPage::projectOpened()
{
	this->setEnabled(true);
}

void VideoFrameTabPage::projectClosed()
{
	this->setEnabled(false);
}


//
//
// VideoFrameControlTabPage
//
//

VideoFrameControlTabPage::VideoFrameControlTabPage(const QString& fileExt,
		DbController* dbcontroller, const QString& parentFileName,
		std::function<VFrame30::CVideoFrame*()> createVideoFrameFunc) :

	HasDbController(dbcontroller),
	m_createVideoFrameFunc(createVideoFrameFunc)
{
	// Create actions
	//
	CreateActions();

	// Create controls
	//
	m_filesView = new VideoFrameFileView(dbcontroller, parentFileName);
	m_filesView->filesModel().setFilter(fileExt);

	QHBoxLayout* pMainLayout = new QHBoxLayout();
	pMainLayout->addWidget(m_filesView);

	setLayout(pMainLayout);

	// --
	//
	connect(m_filesView, &VideoFrameFileView::openFileSignal, this, &VideoFrameControlTabPage::openFiles);
	connect(m_filesView, &VideoFrameFileView::viewFileSignal, this, &VideoFrameControlTabPage::viewFiles);
	connect(m_filesView, &VideoFrameFileView::addFileSignal, this, &VideoFrameControlTabPage::addFile);

	return;
}

VideoFrameControlTabPage::~VideoFrameControlTabPage()
{
}

VFrame30::CVideoFrame* VideoFrameControlTabPage::createVideoFrame() const
{
	return m_createVideoFrameFunc();
}

void VideoFrameControlTabPage::CreateActions()
{
}

void VideoFrameControlTabPage::addFile()
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
	std::shared_ptr<VFrame30::CVideoFrame> vf(m_createVideoFrameFunc());

	vf->setGuid(QUuid::createUuid());

	vf->setStrID("STRID");
	vf->setCaption("Caption");

	vf->setDocWidth(vf->unit() == VFrame30::Display ? 1280 : (420 / 25.4));
	vf->setDocHeight(vf->unit() == VFrame30::Display ? 1024 : (297 / 25.4));

	VideoFramePropertiesDialog propertiesDialog(vf, this);
	if (propertiesDialog.exec() != QDialog::Accepted)
	{
		return;
	}

	Proto::StreamedData sd;
	vf->Save(sd);

	std::shared_ptr<DbFile> vfFile = std::make_shared<DbFile>();
	vfFile->setFileName(fileName);
	vfFile->swapData(sd.mutable_data());

	std::vector<std::shared_ptr<DbFile>> addFilesList;
	addFilesList.push_back(vfFile);

	dbcontroller()->addFiles(&addFilesList, dbcontroller()->wvsFileId(), this);

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

void VideoFrameControlTabPage::openFiles(std::vector<DbFileInfo> files)
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

	if (file.state() == VcsState::CheckedOut && file.userId() != dbcontroller()->currentUser().userId())
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

	// Find the opened file,
	//
	for (int i = 0; i < tabWidget->count(); i++)
	{
		EditVideoFrameTabPage* tb = dynamic_cast<EditVideoFrameTabPage*>(tabWidget->widget(i));
		if (tb == nullptr)
		{
			// It can be control tab page
			//
			continue;
		}

		if (tb->fileInfo().fileName() == file.fileName() &&
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
		return;
	}

	// Load file
	//
	std::shared_ptr<VFrame30::CVideoFrame> vf(VFrame30::CVideoFrame::Create(out[0].get()->data()));

	// Create TabPage and add it to the TabControl
	//
	DbFileInfo fi(*(out.front().get()));

	EditVideoFrameTabPage* editTabPage = new EditVideoFrameTabPage(vf, fi, dbcontroller());

	connect(editTabPage, &EditVideoFrameTabPage::vcsFileStateChanged, this, &VideoFrameControlTabPage::refreshFiles);

	editTabPage->setReadOnly(false);

	tabWidget->addTab(editTabPage, editTabPage->windowTitle());
	tabWidget->setCurrentWidget(editTabPage);

	return;
}

void VideoFrameControlTabPage::viewFiles(std::vector<DbFileInfo> files)
{
	assert(false);
	/*
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

	bool result = dbstore()->getSpecificCopy(changesetId, files, &out, this);
	if (result == false || out.size() != files.size())
	{
		return;
	}

	DbFileInfo fi(*(out.front().get()));

	// Load file
	//
	std::shared_ptr<VFrame30::CVideoFrame> vf(VFrame30::CVideoFrame::Create(out[0].get()->data()));

	QString tabPageTitle;
	tabPageTitle = QString("%1: %2 ReadOnly").arg(vf->strID()).arg(changesetId);

	// Find the opened file,
	//
	for (int i = 0; i < tabWidget->count(); i++)
	{
		EditVideoFrameTabPage* tb = dynamic_cast<EditVideoFrameTabPage*>(tabWidget->widget(i));
		if (tb == nullptr)
		{
			// It can be control tab page
			//
			continue;
		}

		if (tb->fileInfo().fileName() == fi.fileName() &&
			tb->fileInfo().changeset() == fi.changeset() &&
			tb->readOnly() == true)
		{
			tabWidget->setCurrentIndex(i);
			return;
		}
	}

	// Create TabPage and add it to the TabControl
	//

	EditVideoFrameTabPage* editTabPage = new EditVideoFrameTabPage(vf, fi, dbstore());
	editTabPage->setReadOnly(true);

	tabWidget->addTab(editTabPage, tabPageTitle);
	tabWidget->setCurrentWidget(editTabPage);

	return;*/
}

void VideoFrameControlTabPage::refreshFiles()
{
	assert(m_filesView);
	m_filesView->refreshFiles();
	return;
}


//
//
// EditVideoFrameTabPage
//
//
EditVideoFrameTabPage::EditVideoFrameTabPage(std::shared_ptr<VFrame30::CVideoFrame> videoFrame, const DbFileInfo& fileInfo, DbController* dbcontroller) :
	HasDbController(dbcontroller),
	m_videoFrameWidget(nullptr)
{
	assert(videoFrame.get() != nullptr);

	setWindowTitle(videoFrame->strID());

	CreateActions();

	// Create controls
	//
	m_videoFrameWidget = new EditVideoFrameWidget(videoFrame, fileInfo);

	connect(m_videoFrameWidget, &EditVideoFrameWidget::closeTab, this, &EditVideoFrameTabPage::closeTab);
	connect(m_videoFrameWidget, &EditVideoFrameWidget::saveWorkcopy, this, &EditVideoFrameTabPage::saveWorkcopy);
	connect(m_videoFrameWidget, &EditVideoFrameWidget::checkInFile, this, &EditVideoFrameTabPage::checkInFile);
	connect(m_videoFrameWidget, &EditVideoFrameWidget::checkOutFile, this, &EditVideoFrameTabPage::checkOutFile);
	connect(m_videoFrameWidget, &EditVideoFrameWidget::undoChangesFile, this, &EditVideoFrameTabPage::undoChangesFile);
	connect(m_videoFrameWidget, &EditVideoFrameWidget::getCurrentWorkcopy, this, &EditVideoFrameTabPage::getCurrentWorkcopy);
	connect(m_videoFrameWidget, &EditVideoFrameWidget::setCurrentWorkcopy, this, &EditVideoFrameTabPage::setCurrentWorkcopy);

	QHBoxLayout* pMainLayout = new QHBoxLayout();
	pMainLayout->addWidget(m_videoFrameWidget);

	setLayout(pMainLayout);

	return;
}

EditVideoFrameTabPage::~EditVideoFrameTabPage()
{
}

void EditVideoFrameTabPage::CreateActions()
{
}

void EditVideoFrameTabPage::setPageTitle()
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
			newTitle = QString("%1: %2 ReadOnly").arg(m_videoFrameWidget->videoFrame()->strID()).arg(fileInfo().changeset());
		}
		else
		{
			newTitle = QString("%1: ReadOnly").arg(m_videoFrameWidget->videoFrame()->strID());
		}
	}
	else
	{
		newTitle = m_videoFrameWidget->videoFrame()->strID();
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

void EditVideoFrameTabPage::closeTab()
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

void EditVideoFrameTabPage::checkInFile()
{
	assert(false);
	/*
	if (readOnly() == true ||
		fileInfo().state() != VcsState::CheckedOut ||
		fileInfo().user() != dbcontroller()->currentUser())
	{
		assert(fileInfo().user() == dbcontroller()->currentUser());
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
	dbcontroller()->getFileInfo(fileInfo().fileId(), &fi);

	setFileInfo(fi);

	setReadOnly(true);

	setPageTitle();
*/
	return;
}

void EditVideoFrameTabPage::checkOutFile()
{
	assert(false);
	/*
	if (readOnly() == false ||
		fileInfo().state() != VcsState::CheckedIn)
	{
		return;
	}

	std::vector<DbFileInfo> files;
	files.push_back(fileInfo());

	bool result = dbcontroller()->checkOutFiles(files, this);
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

	m_videoFrameWidget->videoFrame()->Load(out[0].get()->data());

	setFileInfo(*(out.front().get()));

	setReadOnly(false);
	setPageTitle();

	m_videoFrameWidget->resetAction();
	m_videoFrameWidget->clearSelection();

	m_videoFrameWidget->update();

	emit vcsFileStateChanged();
	return;
	*/
}

void EditVideoFrameTabPage::undoChangesFile()
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

bool EditVideoFrameTabPage::saveWorkcopy()
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
	m_videoFrameWidget->videoFrame()->Save(data);

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

void EditVideoFrameTabPage::getCurrentWorkcopy()
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

	bool writeResult = m_videoFrameWidget->videoFrame()->Save(fileName);

	if (writeResult == false)
	{
		QMessageBox msgBox(this);
		msgBox.setText(tr("Write file error."));
		msgBox.setInformativeText(tr("Cannot write file %1.").arg(fileInfo().fileName()));
		msgBox.exec();
	}

	return;
}

void EditVideoFrameTabPage::setCurrentWorkcopy()
{
	if (readOnly() == true ||
		fileInfo().state() != VcsState::CheckedOut ||
		fileInfo().userId() != dbcontroller()->currentUser().userId())
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
	bool readResult = m_videoFrameWidget->videoFrame()->Load(fileName);
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

const DbFileInfo& EditVideoFrameTabPage::fileInfo() const
{
	assert(m_videoFrameWidget);
	return m_videoFrameWidget->fileInfo();
}

void EditVideoFrameTabPage::setFileInfo(const DbFileInfo& fi)
{
	assert(m_videoFrameWidget);
	m_videoFrameWidget->setFileInfo(fi);
}

bool EditVideoFrameTabPage::readOnly() const
{
	assert(m_videoFrameWidget);
	return m_videoFrameWidget->readOnly();
}

void EditVideoFrameTabPage::setReadOnly(bool value)
{
	assert(m_videoFrameWidget);
	m_videoFrameWidget->setReadOnly(value);
}

bool EditVideoFrameTabPage::modified() const
{
	assert(m_videoFrameWidget);
	return m_videoFrameWidget->modified();
}

void EditVideoFrameTabPage::resetModified()
{
	assert(m_videoFrameWidget);
	return m_videoFrameWidget->resetModified();
}
